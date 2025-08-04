# 커널 패닉 문제 해결 요청
_Exported on 7/29/2025 at 09:41:55 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 할 때 커널 패닉이 발생한다.
Call trace 에는
one_q_tsn_sysclock_to_timestamp
lan1qtsn_send_packet

이렇게 화면에 출력하고 있다.

무엇인 문제인지 확인해 주세요. 그리고 필요한 정보가 더 있는지 확인해 주세요.

---

**Cursor**

커널 패닉 문제를 분석하기 위해 먼저 관련 코드를 살펴보겠습니다. Call trace에서 언급된 함수들을 확인해보겠습니다.


이제 `one_q_tsn_sysclock_to_timestamp` 함수를 찾아보겠습니다.


이제 `one_q_tsn_sysclock_to_timestamp` 함수의 구현을 확인해보겠습니다.


이제 `tsnlab_spi.c` 파일에서 `lan1qtsn_send_packet`과 `one_q_tsn_sysclock_to_timestamp`가 어떻게 연결되는지 확인해보겠습니다.


이제 `ptp_device_data` 구조체와 관련 헤더 파일들을 확인해보겠습니다.


이제 `one_q_tsn_arch.h` 파일을 확인해보겠습니다.


이제 `tsnlab_spi.c` 파일에서 `ptp_device_data` 구조체가 어떻게 초기화되는지 확인해보겠습니다.


이제 `tsnlab_spi.c` 파일에서 `tc6->ptp`가 어떻게 설정되는지 확인해보겠습니다.


`tsnlab_spi.c` 파일에서 `tc6->ptp`가 설정되는 부분을 찾아보겠습니다.


`tsnlab_spi.c` 파일에서 `tc6->ptp` 할당을 찾아보겠습니다.


`lan1qtsn.c` 파일에서 `ptp_device_init` 호출 후 `tc6->ptp` 설정을 확인해보겠습니다.


문제를 발견했습니다! `ptp_device_init` 함수가 호출되지만 그 반환값이 `tc6->ptp`에 할당되지 않고 있습니다. 이제 `ptp_device_init` 함수를 다시 확인해보겠습니다.


이제 문제를 명확히 파악했습니다. `ptp_device_init` 함수는 `ptp_device_data` 구조체를 반환하지만, `lan1qtsn.c`에서 이 반환값을 `tc6->ptp`에 할당하지 않고 있습니다. 이로 인해 `one_q_tsn_sysclock_to_timestamp` 함수에서 `tc6->ptp`가 NULL이 되어 커널 패닉이 발생합니다.

문제를 수정해보겠습니다:


```c
    ptp_data = ptp_device_init(&spi->dev, priv->tc6);

    return misc_register(&lan1qtsn_miscdev);

->

    ptp_data = ptp_device_init(&spi->dev, priv->tc6);
    if (ptp_data) {
        priv->tc6->ptp = ptp_data;
    }

    return misc_register(&lan1qtsn_miscdev);
```

이제 `ptp_device_destroy` 함수도 호출되도록 `lan1qtsn_remove` 함수를 수정해야 합니다:


```c
static void lan1qtsn_remove(struct spi_device* spi) {
    struct lan1qtsn_priv* priv = spi_get_drvdata(spi);

    cancel_work_sync(&priv->multicast_work);
    unregister_netdev(priv->netdev);
    tsnlab_spi_exit(priv->tc6);
    free_netdev(priv->netdev);
    misc_deregister(&lan1qtsn_miscdev);
}

->

static void lan1qtsn_remove(struct spi_device* spi) {
    struct lan1qtsn_priv* priv = spi_get_drvdata(spi);

    cancel_work_sync(&priv->multicast_work);
    unregister_netdev(priv->netdev);
    if (priv->tc6->ptp) {
        ptp_device_destroy(priv->tc6->ptp);
    }
    tsnlab_spi_exit(priv->tc6);
    free_netdev(priv->netdev);
    misc_deregister(&lan1qtsn_miscdev);
}
```

이제 `ptp_device_destroy` 함수를 사용하기 위해 헤더 파일을 포함해야 합니다:


```c
#include "lan1qtsn_ioctl.h"
#include "one_q_tsn_ptp.h"

->

#include "lan1qtsn_ioctl.h"
#include "one_q_tsn_ptp.h"
```

헤더 파일이 이미 포함되어 있으므로 추가할 필요가 없습니다. 이제 다른 잠재적인 문제들을 확인해보겠습니다.

`one_q_tsn_sysclock_to_timestamp` 함수에서 NULL 포인터 체크를 추가하는 것이 좋겠습니다:


```c
timestamp_t one_q_tsn_sysclock_to_timestamp(struct tsnlab_spi* tc6, sysclock_t sysclock) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    u64 offset = ptp_data->offset;

    return one_q_tsn_get_timestamp(sysclock, ptp_data->ticks_scale, offset);
}

->

timestamp_t one_q_tsn_sysclock_to_timestamp(struct tsnlab_spi* tc6, sysclock_t sysclock) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    if (!ptp_data) {
        pr_err("ptp_data is NULL in one_q_tsn_sysclock_to_timestamp\n");
        return 0;
    }

    u64 offset = ptp_data->offset;

    return one_q_tsn_get_timestamp(sysclock, ptp_data->ticks_scale, offset);
}
```

다른 PTP 관련 함수들도 NULL 체크를 추가해보겠습니다:


```c
sysclock_t one_q_tsn_timestamp_to_sysclock(struct tsnlab_spi* tc6, timestamp_t timestamp) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    return (timestamp - ptp_data->offset) / ptp_data->ticks_scale;
}

->

sysclock_t one_q_tsn_timestamp_to_sysclock(struct tsnlab_spi* tc6, timestamp_t timestamp) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    if (!ptp_data) {
        pr_err("ptp_data is NULL in one_q_tsn_timestamp_to_sysclock\n");
        return 0;
    }

    return (timestamp - ptp_data->offset) / ptp_data->ticks_scale;
}
```

```c
double one_q_tsn_get_ticks_scale(struct tsnlab_spi* tc6) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    return ptp_data->ticks_scale;
}

->

double one_q_tsn_get_ticks_scale(struct tsnlab_spi* tc6) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    if (!ptp_data) {
        pr_err("ptp_data is NULL in one_q_tsn_get_ticks_scale\n");
        return TICKS_SCALE;
    }

    return ptp_data->ticks_scale;
}
```

```c
void one_q_tsn_set_ticks_scale(struct tsnlab_spi* tc6, double ticks_scale) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    ptp_data->ticks_scale = ticks_scale;
    one_q_tsn_set_cycle_1s(tc6, (double)NS_IN_1S / ptp_data->ticks_scale);
}

->

void one_q_tsn_set_ticks_scale(struct tsnlab_spi* tc6, double ticks_scale) {
    struct ptp_device_data* ptp_data = tc6->ptp;

    if (!ptp_data) {
        pr_err("ptp_data is NULL in one_q_tsn_set_ticks_scale\n");
        return;
    }

    ptp_data->ticks_scale = ticks_scale;
    one_q_tsn_set_cycle_1s(tc6, (double)NS_IN_1S / ptp_data->ticks_scale);
}
```

이제 `tsnlab_spi.c` 파일에서도 NULL 체크를 추가해보겠습니다:


이제 `tsnlab_spi_start_xmit` 함수에서 `one_q_tsn_sysclock_to_timestamp` 호출 전에 NULL 체크를 추가해보겠습니다:


```c
    sys_count = one_q_tsn_get_sys_clock(tc6);
    now = one_q_tsn_sysclock_to_timestamp(tc6, sys_count);
    sys_count_lower = sys_count & LOWER_29_BITS;
    sys_count_upper = sys_count & ~LOWER_29_BITS;

->

    sys_count = one_q_tsn_get_sys_clock(tc6);
    if (tc6->ptp) {
        now = one_q_tsn_sysclock_to_timestamp(tc6, sys_count);
    } else {
        now = 0;
        pr_warn("ptp_data is NULL, using default timestamp\n");
    }
    sys_count_lower = sys_count & LOWER_29_BITS;
    sys_count_upper = sys_count & ~LOWER_29_BITS;
```

이제 추가적인 문제가 있는지 확인해보겠습니다. `tsnlab_spi_init` 함수에서 `tc6->ptp`가 초기화되지 않는 것도 문제일 수 있습니다:


```c
    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);

->

    tc6->spi = spi;
    tc6->netdev = netdev;
    tc6->ptp = NULL;  /* Initialize ptp to NULL */
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);
```

이제 수정사항을 요약해보겠습니다:

## 문제 분석 및 해결책

### 문제 원인
커널 패닉이 발생하는 주요 원인은 다음과 같습니다:

1. **NULL 포인터 역참조**: `ptp_device_init` 함수가 호출되었지만, 그 반환값이 `tc6->ptp`에 할당되지 않아서 `tc6->ptp`가 NULL 상태로 남아있었습니다.

2. **Call trace 분석**:
   - `lan1qtsn_send_packet` → `tsnlab_spi_start_xmit` → `one_q_tsn_sysclock_to_timestamp`
   - `one_q_tsn_sysclock_to_timestamp` 함수에서 `tc6->ptp`가 NULL인 상태로 `ptp_data->offset`에 접근하려고 해서 커널 패닉이 발생했습니다.

### 수정사항

1. **`lan1qtsn.c`에서 `tc6->ptp` 할당 추가**:
   ```c
   ptp_data = ptp_device_init(&spi->dev, priv->tc6);
   if (ptp_data) {
       priv->tc6->ptp = ptp_data;
   }
   ```

2. **`lan1qtsn_remove`에서 PTP 디바이스 정리 추가**:
   ```c
   if (priv->tc6->ptp) {
       ptp_device_destroy(priv->tc6->ptp);
   }
   ```

3. **NULL 포인터 체크 추가**:
   - `one_q_tsn_sysclock_to_timestamp`
   - `one_q_tsn_timestamp_to_sysclock`
   - `one_q_tsn_get_ticks_scale`
   - `one_q_tsn_set_ticks_scale`
   - `tsnlab_spi_start_xmit`에서 PTP 체크

4. **`tsnlab_spi_init`에서 `tc6->ptp` 초기화**:
   ```c
   tc6->ptp = NULL;  /* Initialize ptp to NULL */
   ```

### 추가 확인사항

커널을 다시 빌드하고 테스트할 때 다음 사항들을 확인해주세요:

1. **컴파일 에러**: 수정된 코드가 정상적으로 컴파일되는지 확인
2. **드라이버 로드**: 모듈이 정상적으로 로드되는지 확인
3. **네트워크 인터페이스**: 네트워크 인터페이스가 정상적으로 생성되는지 확인
4. **PTP 기능**: PTP 관련 기능이 정상적으로 작동하는지 확인

이 수정사항들로 커널 패닉 문제가 해결될 것으로 예상됩니다. 만약 여전히 문제가 발생한다면, 추가적인 디버깅 정보가 필요할 수 있습니다.

```c
   ptp_data = ptp_device_init(&spi->dev, priv->tc6);
   if (ptp_data) {
       priv->tc6->ptp = ptp_data;
   }
```

```c
   if (priv->tc6->ptp) {
       ptp_device_destroy(priv->tc6->ptp);
   }
```

```c
   tc6->ptp = NULL;  /* Initialize ptp to NULL */
```

