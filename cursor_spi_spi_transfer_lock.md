# SPI 통신을 위한 동기화 메커니즘 구현
_Exported on 8/19/2025 at 20:58:44 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있다.

SPI 통신으로 packet을 rx/tx하고 레지스터도 read/write 하고 있다.

여러 worker(thread or task)가 동시에 SPI를 통해 레지스터를 읽거나 packet을 송수신 할 가능성이 있다.

lan1qtsn_spi_transfer함수에 대한 spin lock(or mutex)을 사용하여 spi 통신을 하는 중에 다른 것이 spi를 점유하지 못하도록 조치해 주시고,
lan1qtsn_spi_perform_ctrl 함수에 대한 spin lock(or mutex)을 사용하여 레지스터 값을 읽으려 할 때 다른 것이 레지스터 읽는 동작을 못하도록 조치해 주세요.


---

**Cursor**

먼저 현재 코드 구조를 파악하기 위해 디렉토리를 살펴보겠습니다.


drivers 디렉토리를 확인해보겠습니다.


net 디렉토리를 확인해보겠습니다.


ethernet 디렉토리를 확인해보겠습니다.


tsnlab 디렉토리를 확인해보겠습니다.


lan1qtsn 디렉토리를 확인해보겠습니다.


이제 `lan1qtsn_spi.c` 파일에서 `lan1qtsn_spi_transfer` 함수와 `lan1qtsn_spi_perform_ctrl` 함수를 찾아보겠습니다.


이제 `lan1qtsn_spi.c` 파일을 자세히 살펴보겠습니다.


이제 `lan1qtsn_spi_perform_ctrl` 함수를 확인해보겠습니다.


이제 헤더 파일을 확인하여 구조체 정의를 살펴보겠습니다.


이제 코드를 분석해보니, 이미 `spi_ctrl_lock`이라는 mutex가 정의되어 있고, `lan1qtsn_spi_read_registers`와 `lan1qtsn_spi_write_registers` 함수에서 사용되고 있습니다. 하지만 `lan1qtsn_spi_transfer` 함수 자체에는 lock이 없습니다.

현재 상황을 정리하면:
1. `lan1qtsn_spi_transfer` 함수: SPI 통신을 담당하는 핵심 함수로, 현재는 lock이 없음
2. `lan1qtsn_spi_perform_ctrl` 함수: 레지스터 읽기/쓰기를 담당하는 함수로, 이미 `spi_ctrl_lock` mutex로 보호됨

하지만 `lan1qtsn_spi_transfer` 함수는 다른 곳에서도 직접 호출될 수 있으므로, 이 함수 자체에도 lock을 추가해야 합니다. 

이제 필요한 lock을 추가하겠습니다:

1. `lan1qtsn_spi_transfer` 함수에 대한 spin lock 추가
2. `lan1qtsn_spi_perform_ctrl` 함수는 이미 mutex로 보호되어 있지만, 더 명확하게 하기 위해 주석 추가


```cpp
struct lan1qtsn_spi {
	struct spi_device *spi;
	struct lan1qtsn_spi_dev *tsdev;
	struct net_device *netdev;

	// 디버깅용 카운터
	atomic_t wrong_mac_count;
	atomic_t count1; // start_xmit 호출 카운트
	atomic_t count2; // SPI 전송 완료 카운트

	struct mutex spi_ctrl_lock; /* Protects spi control transfer */
	spinlock_t tx_skb_lock; /* Protects tx skb handling */

->

struct lan1qtsn_spi {
	struct spi_device *spi;
	struct lan1qtsn_spi_dev *tsdev;
	struct net_device *netdev;

	// 디버깅용 카운터
	atomic_t wrong_mac_count;
	atomic_t count1; // start_xmit 호출 카운트
	atomic_t count2; // SPI 전송 완료 카운트

	struct mutex spi_ctrl_lock; /* Protects spi control transfer */
	spinlock_t tx_skb_lock; /* Protects tx skb handling */
	spinlock_t spi_transfer_lock; /* Protects all SPI transfers */
```

이제 `lan1qtsn_spi_transfer` 함수에 spin lock을 추가하겠습니다.


```c
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);

->

static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;
    unsigned long flags;

    /* Acquire spin lock to prevent concurrent SPI access */
    spin_lock_irqsave(&tc6->spi_transfer_lock, flags);

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);
```

이제 함수의 끝 부분에서 spin lock을 해제하도록 수정하겠습니다.


이제 `lan1qtsn_spi_transfer` 함수에서 spin lock을 해제하도록 수정하겠습니다. 에러가 발생하거나 성공적으로 완료된 후에 lock을 해제해야 합니다.


```c
    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
    return 0;

->

    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        /* Release spin lock before returning error */
        spin_unlock_irqrestore(&tc6->spi_transfer_lock, flags);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
    
    /* Release spin lock after successful transfer */
    spin_unlock_irqrestore(&tc6->spi_transfer_lock, flags);
    return 0;
```

이제 `lan1qtsn_spi_perform_ctrl` 함수에 대한 주석을 추가하여 이미 mutex로 보호되고 있음을 명확히 하겠습니다.


```c
static int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                     enum lan1qtsn_spi_register_op reg_op) {
    u16 size;
    int ret;

->

static int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                     enum lan1qtsn_spi_register_op reg_op) {
    /*
     * This function is protected by spi_ctrl_lock mutex in the calling functions
     * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
     * to prevent concurrent register access from multiple workers.
     */
    u16 size;
    int ret;
```

이제 초기화 함수에서 새로운 spin lock을 초기화해야 합니다. `lan1qtsn_spi_init` 함수를 찾아보겠습니다.


`lan1qtsn_spi_init` 함수를 확인해보겠습니다.


이제 `lan1qtsn_spi_init` 함수에서 새로운 spin lock을 초기화하도록 수정하겠습니다.


```c
    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);

->

    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);
    spin_lock_init(&tc6->spi_transfer_lock);
```

이제 수정된 코드를 확인해보겠습니다. 변경사항을 요약하면:

1. **`lan1qtsn_spi_transfer` 함수에 spin lock 추가**: 
   - `spi_transfer_lock`이라는 새로운 spin lock을 사용하여 모든 SPI 통신을 보호
   - 함수 시작 시 `spin_lock_irqsave()`로 lock 획득
   - 함수 종료 시 (성공 또는 실패) `spin_unlock_irqrestore()`로 lock 해제

2. **`lan1qtsn_spi_perform_ctrl` 함수**: 
   - 이미 `spi_ctrl_lock` mutex로 보호되고 있음
   - 주석을 추가하여 보호 상태를 명확히 함

3. **구조체에 새로운 lock 추가**:
   - `lan1qtsn_spi` 구조체에 `spi_transfer_lock` spin lock 추가
   - `lan1qtsn_spi_init` 함수에서 새로운 lock 초기화

이러한 변경으로 인해:
- **SPI 통신 동시성 문제 해결**: `lan1qtsn_spi_transfer` 함수가 실행 중일 때 다른 worker가 SPI를 점유할 수 없음
- **레지스터 접근 동시성 문제 해결**: `lan1qtsn_spi_perform_ctrl` 함수는 이미 mutex로 보호되어 있어 여러 worker가 동시에 레지스터를 읽거나 쓸 수 없음

spin lock을 사용한 이유는 SPI 통신이 빠르게 완료되는 작업이고, 인터럽트 컨텍스트에서도 안전하게 사용할 수 있기 때문입니다. 반면 mutex는 레지스터 접근과 같이 더 긴 시간이 걸릴 수 있는 작업에 적합합니다.

---

**User**

적용하니 이런 에러가 발생합니다.

[ 8022.654978] BUG: scheduling while atomic: lan1qtsn-spi-th/85708/0x00000002
[ 8022.655017] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc vc4 brcmfmac snd_soc_hdmi_codec drm_display_helper cec brcmutil drm_dma_helper spidev binfmt_misc snd_soc_core bcm2835_v4l2(C) hci_uart bcm2835_codec(C) bcm2835_isp(C) rpi_hevc_dec btbcm bcm2835_mmal_vchiq(C) v3d bluetooth cfg80211 vc_sm_cma(C) videobuf2_vmalloc v4l2_mem2mem gpu_sched videobuf2_dma_contig snd_compress videobuf2_memops videobuf2_v4l2 ecdh_generic videodev ecc drm_shmem_helper rfkill drm_kms_helper libaes snd_pcm_dmaengine snd_bcm2835(C) snd_pcm raspberrypi_hwmon videobuf2_common snd_timer i2c_brcmstb snd mc spi_bcm2835 raspberrypi_gpiomem nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern fuse drm_panel_orientation_quirks backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 8022.655480] CPU: 1 UID: 0 PID: 85708 Comm: lan1qtsn-spi-th Tainted: G        WC O       6.12.386.12.34+rpt-rpi-v8+ #61
[ 8022.655501] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 8022.655506] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 8022.655512] Call trace:
[ 8022.655517]  dump_backtrace+0xa0/0x100
[ 8022.655536]  show_stack+0x20/0x38
[ 8022.655546]  dump_stack_lvl+0x78/0x90
[ 8022.655563]  dump_stack+0x18/0x28
[ 8022.655575]  __schedule_bug+0x58/0x78
[ 8022.655592]  __schedule+0x964/0xb60
[ 8022.655603]  schedule+0x3c/0x148
[ 8022.655612]  schedule_timeout+0x98/0x1a0
[ 8022.655625]  wait_for_completion_timeout+0x80/0x160
[ 8022.655636]  spi_transfer_one_message+0x73c/0x8a8
[ 8022.655650]  __spi_pump_transfer_message+0x274/0x610
[ 8022.655661]  __spi_sync+0x2b8/0x458
[ 8022.655670]  spi_sync+0x38/0x68
[ 8022.655680]  lan1qtsn_spi_transfer+0xec/0x188 [lan1qtsn]
[ 8022.655710]  lan1qtsn_spi_thread_handler+0x180/0x7f8 [lan1qtsn]
[ 8022.655730]  kthread+0x11c/0x128
[ 8022.655742]  ret_from_fork+0x10/0x20
[ 8022.655810] BUG: scheduling while atomic: lan1qtsn-spi-th/85708/0x00000000
[ 8022.655838] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc vc4 brcmfmac snd_soc_hdmi_codec drm_display_helper cec brcmutil drm_dma_helper spidev binfmt_misc snd_soc_core bcm2835_v4l2(C) hci_uart bcm2835_codec(C) bcm2835_isp(C) rpi_hevc_dec btbcm bcm2835_mmal_vchiq(C) v3d bluetooth cfg80211 vc_sm_cma(C) videobuf2_vmalloc v4l2_mem2mem gpu_sched videobuf2_dma_contig snd_compress videobuf2_memops videobuf2_v4l2 ecdh_generic videodev ecc drm_shmem_helper rfkill drm_kms_helper libaes snd_pcm_dmaengine snd_bcm2835(C) snd_pcm raspberrypi_hwmon videobuf2_common snd_timer i2c_brcmstb snd mc spi_bcm2835 raspberrypi_gpiomem nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern fuse drm_panel_orientation_quirks backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 8022.656256] CPU: 1 UID: 0 PID: 85708 Comm: lan1qtsn-spi-th Tainted: G        WC O       6.12.386.12.34+rpt-rpi-v8+ #61
[ 8022.656273] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 8022.656277] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 8022.656282] Call trace:
[ 8022.656286]  dump_backtrace+0xa0/0x100
[ 8022.656296]  show_stack+0x20/0x38
[ 8022.656304]  dump_stack_lvl+0x78/0x90
[ 8022.656318]  dump_stack+0x18/0x28
[ 8022.656331]  __schedule_bug+0x58/0x78
[ 8022.656345]  __schedule+0x964/0xb60
[ 8022.656354]  schedule+0x3c/0x148
[ 8022.656363]  lan1qtsn_spi_thread_handler+0x5b8/0x7f8 [lan1qtsn]
[ 8022.656382]  kthread+0x11c/0x128
[ 8022.656392]  ret_from_fork+0x10/0x20


---

**Cursor**

에러를 분석해보니 "scheduling while atomic" 문제가 발생했습니다. 이는 spin lock을 사용할 때 발생할 수 있는 문제입니다. 

문제의 원인은 `lan1qtsn_spi_transfer` 함수에서 `spi_sync()`를 호출하는 동안 spin lock을 계속 보유하고 있기 때문입니다. `spi_sync()`는 내부적으로 스케줄링을 할 수 있어서 atomic context에서 문제가 됩니다.

해결 방법은 `lan1qtsn_spi_transfer` 함수에서 spin lock 대신 mutex를 사용하는 것입니다. 이렇게 하면 SPI 통신 중에 스케줄링이 가능해집니다.

코드를 수정하겠습니다:


```cpp
	struct mutex spi_ctrl_lock; /* Protects spi control transfer */
	spinlock_t tx_skb_lock; /* Protects tx skb handling */
	spinlock_t spi_transfer_lock; /* Protects all SPI transfers */

->

	struct mutex spi_ctrl_lock; /* Protects spi control transfer */
	spinlock_t tx_skb_lock; /* Protects tx skb handling */
	struct mutex spi_transfer_lock; /* Protects all SPI transfers */
```

이제 `lan1qtsn_spi_transfer` 함수에서 mutex를 사용하도록 수정하겠습니다:


```c
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;
    unsigned long flags;

    /* Acquire spin lock to prevent concurrent SPI access */
    spin_lock_irqsave(&tc6->spi_transfer_lock, flags);

->

static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    /* Acquire mutex to prevent concurrent SPI access */
    mutex_lock(&tc6->spi_transfer_lock);
```

이제 함수의 끝 부분에서 mutex를 해제하도록 수정하겠습니다:


```c
    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        /* Release spin lock before returning error */
        spin_unlock_irqrestore(&tc6->spi_transfer_lock, flags);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
    
    /* Release spin lock after successful transfer */
    spin_unlock_irqrestore(&tc6->spi_transfer_lock, flags);
    return 0;

->

    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        /* Release mutex before returning error */
        mutex_unlock(&tc6->spi_transfer_lock);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
    
    /* Release mutex after successful transfer */
    mutex_unlock(&tc6->spi_transfer_lock);
    return 0;
```

마지막으로 `lan1qtsn_spi_init` 함수에서 mutex를 초기화하도록 수정하겠습니다:


```c
    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);
    spin_lock_init(&tc6->spi_transfer_lock);

->

    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);
    mutex_init(&tc6->spi_transfer_lock);
```

이제 수정이 완료되었습니다. 변경사항을 요약하면:

## 문제 분석
- **"scheduling while atomic" 에러**: `lan1qtsn_spi_transfer` 함수에서 `spi_sync()` 호출 시 spin lock을 보유한 채로 스케줄링이 발생하여 문제가 됨
- **원인**: `spi_sync()`는 내부적으로 스케줄링을 할 수 있는데, spin lock은 atomic context에서만 사용해야 함

## 해결 방법
1. **`spi_transfer_lock`을 spin lock에서 mutex로 변경**:
   - `spinlock_t spi_transfer_lock` → `struct mutex spi_transfer_lock`

2. **`lan1qtsn_spi_transfer` 함수에서 mutex 사용**:
   - `spin_lock_irqsave()` → `mutex_lock()`
   - `spin_unlock_irqrestore()` → `mutex_unlock()`

3. **초기화 함수 수정**:
   - `spin_lock_init()` → `mutex_init()`

## 장점
- **스케줄링 안전성**: mutex는 스케줄링을 허용하므로 `spi_sync()` 호출 시 문제가 발생하지 않음
- **동시성 보호**: 여전히 여러 worker가 동시에 SPI를 점유하는 것을 방지
- **인터럽트 안전성**: mutex는 인터럽트 컨텍스트에서도 안전하게 사용 가능

이제 "scheduling while atomic" 에러 없이 SPI 통신의 동시성을 보호할 수 있습니다.

