# 라즈베리 파이 네트워크 문제 분석
_Exported on 8/22/2025 at 12:45:33 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있어.

ping과 같은 시험을 진행 할 때 시스템에 영향을 주는 것 같아.

증상 1. ping 시험을 진행하면서 코드를 compile하면 이상하게 에러가 생기는 경우가 있어.
동일한 코드를 네트웤 관련 시험을 안하면서 compile하면 에러가 없고.

증상 2.  "Incorrect MAC received on packet"  메시지를 띄우면서 원격에서 연결한 터미널이 종료 되는 경우가 있어.
증상 3. 원격 네크웤 이상으로 터미널이 종료되었다는 메시지를 띄우면서 원격에서 연결한 터미널이 종료 되는 경우가 있어.
증상 4. 가끔 lock이 걸리는 것처럼 아무런 메시지 출력도 없이 원격 네트웤 연결도 끊기고 시스템도 먹통이 돼

자세히 확인 해 줘.

---

**Cursor**

제공해주신 증상들을 분석해보겠습니다. 먼저 코드를 자세히 살펴보고 문제점을 파악해보겠습니다.




<｜Assistant｜>제공해주신 증상들을 분석한 결과, 다음과 같은 문제점들을 발견했습니다:

## 주요 문제점 분석

### 1. **증상 1: ping 시험 중 컴파일 에러 발생**
- **원인**: 네트워크 인터럽트와 컴파일 프로세스 간의 리소스 경합
- **구체적 문제**:
  - SPI 인터럽트 핸들러가 `IRQF_TRIGGER_FALLING`으로 설정되어 있어 네트워크 트래픽이 많을 때 인터럽트가 빈번하게 발생
  - `sched_set_fifo()`로 SPI 스레드를 실시간 우선순위로 설정하여 다른 프로세스와 경합
  - `tc6->spi->rt = true`로 SPI 컨트롤러를 실시간 모드로 설정

### 2. **증상 2: "Incorrect MAC received on packet" 메시지**
- **원인**: MAC 주소 검증 로직의 문제
- **구체적 문제**:
  ```c:363:lan1qtsn_spi.c
  dev_info(&tc6->spi->dev, "Wrong MAC detected (count: %d)\n", atomic_read(&tc6->wrong_mac_count));
  ```
  - MAC 주소가 잘못된 패킷을 받았을 때 카운터만 증가시키고 패킷을 드롭
  - 원격 터미널 연결이 끊어지는 것은 이 에러가 연쇄적으로 발생하여 네트워크 스택에 영향을 주기 때문

### 3. **증상 3: 원격 네트워크 이상으로 터미널 종료**
- **원인**: 네트워크 스택의 불안정성
- **구체적 문제**:
  - `netif_stop_queue()`와 `netif_wake_queue()` 호출이 비동기적으로 발생
  - SPI 스레드와 네트워크 스택 간의 동기화 문제
  - `wait_event_interruptible()`과 `wake_up_interruptible()` 사용으로 인한 경합 상태

### 4. **증상 4: 시스템 먹통 및 lock 발생**
- **원인**: 데드락 및 리소스 경합
- **구체적 문제**:
  ```c:152-153:lan1qtsn_spi.c
  // Check if we're already holding the lock to prevent deadlock
  if (mutex_is_locked(&tc6->spi_ctrl_lock)) {
  ```
  - `spi_ctrl_lock` 뮤텍스의 데드락 방지 로직이 불완전
  - `spin_lock_bh(&tc6->tx_skb_lock)`과 `mutex_lock(&tc6->spi_ctrl_lock)` 간의 교착 상태 가능성
  - `test_and_set_bit_lock()`과 `clear_bit_unlock()` 사용으로 인한 비트 락 경합

## 해결 방안

### 1. **인터럽트 및 스케줄링 최적화**
```c
// 실시간 우선순위를 일반 우선순위로 변경
// sched_set_fifo(tc6->spi_thread); // 주석 처리
// tc6->spi->rt = true; // 주석 처리
```

### 2. **락 메커니즘 개선**
```c
// 데드락 방지를 위한 타임아웃 추가
if (mutex_trylock(&tc6->spi_ctrl_lock)) {
    // 락 획득 성공
} else {
    // 타임아웃 후 재시도 또는 에러 처리
    return -EBUSY;
}
```

### 3. **MAC 주소 검증 로직 강화**
```c
// MAC 주소 에러 시 로깅만 하고 패킷 처리 계속
if (!is_broadcast && !is_multicast && !is_for_me) {
    atomic_inc(&tc6->wrong_mac_count);
    dev_dbg(&tc6->spi->dev, "Wrong MAC, but continuing...\n");
    // 패킷을 드롭하지 않고 계속 처리
}
```

### 4. **네트워크 큐 관리 개선**
```c
// 큐 상태 체크를 더 안전하게
if (netif_queue_stopped(ndev) && !tc6->waiting_tx_skb) {
    netif_wake_queue(ndev);
}
```

이러한 문제들은 주로 실시간 처리와 일반 프로세스 간의 리소스 경합, 그리고 네트워크 스택의 동기화 문제에서 발생합니다. 특히 ping과 같은 네트워크 트래픽이 많을 때 이러한 문제가 더욱 심화됩니다.

```c
  dev_info(&tc6->spi->dev, "Wrong MAC detected (count: %d)\n", atomic_read(&tc6->wrong_mac_count));
```

```c
  // Check if we're already holding the lock to prevent deadlock
  if (mutex_is_locked(&tc6->spi_ctrl_lock)) {
```

```c
// 실시간 우선순위를 일반 우선순위로 변경
// sched_set_fifo(tc6->spi_thread); // 주석 처리
// tc6->spi->rt = true; // 주석 처리
```

```c
// 데드락 방지를 위한 타임아웃 추가
if (mutex_trylock(&tc6->spi_ctrl_lock)) {
    // 락 획득 성공
} else {
    // 타임아웃 후 재시도 또는 에러 처리
    return -EBUSY;
}
```

```c
// MAC 주소 에러 시 로깅만 하고 패킷 처리 계속
if (!is_broadcast && !is_multicast && !is_for_me) {
    atomic_inc(&tc6->wrong_mac_count);
    dev_dbg(&tc6->spi->dev, "Wrong MAC, but continuing...\n");
    // 패킷을 드롭하지 않고 계속 처리
}
```

```c
// 큐 상태 체크를 더 안전하게
if (netif_queue_stopped(ndev) && !tc6->waiting_tx_skb) {
    netif_wake_queue(ndev);
}
```

