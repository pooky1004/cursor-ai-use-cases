# Modify register access in Raspberry Pi kernel
_Exported on 8/22/2025 at 21:05:10 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있다.



drivers/net/ethernet/tsnlab/lan1qtsn/ 디렉토리 내 lan1qtsn_arch.c 파일에

lan1qtsn_set_pulse_at_by_ldev,  lan1qtsn_set_cycle_1s_by_ldev 함수는 연속된 2개의 4바이트 레지스터에 각각 값을 설정하고,



lan1qtsn_get_sys_clock_by_ldev 
lan1qtsn_read_tx_timestamp_by_ldev
lan1qtsn_get_buffer_write_status_by_ldev
lan1qtsn_get_total_new_entry_by_ldev
lan1qtsn_get_total_valid_entry_by_ldev
lan1qtsn_get_total_drop_entry_by_ldev 함수는 연속된 2개의 4바이트 레지스터의 값을 각각 읽고 있다.

lan1qtsn_spi.h 파일 내 LAN1QTSN_SPI_CTRL_MAX_REGISTERS 값을 2로 변경하면 한번에 2개의 4바이트 레지스터에 값을 쓰거나 읽을 수 있다.

그렇게 하도록 코드를 수정해 주세요.


---

**Cursor**

네, 이해했습니다. `lan1qtsn_arch.c` 파일의 함수들을 수정하여 연속된 2개의 4바이트 레지스터를 한 번에 처리하도록 하고, `lan1qtsn_spi.h` 파일에서 `LAN1QTSN_SPI_CTRL_MAX_REGISTERS` 값을 2로 변경하겠습니다.

먼저 현재 파일들을 확인해보겠습니다.


이제 파일의 나머지 부분도 확인해보겠습니다.


이제 `lan1qtsn_spi.h` 파일을 확인해보겠습니다.


이제 `LAN1QTSN_SPI_CTRL_MAX_REGISTERS` 값을 찾았습니다. 42번째 줄에 `#define LAN1QTSN_SPI_CTRL_MAX_REGISTERS 1`로 정의되어 있습니다. 이를 2로 변경하고, `lan1qtsn_arch.c` 파일의 함수들을 수정하겠습니다.

먼저 `lan1qtsn_spi.h` 파일을 수정하겠습니다:


```cpp
#define LAN1QTSN_SPI_CTRL_MAX_REGISTERS 1

->

#define LAN1QTSN_SPI_CTRL_MAX_REGISTERS 2
```

이제 `lan1qtsn_arch.c` 파일의 함수들을 수정하겠습니다. 연속된 2개의 4바이트 레지스터를 한 번에 처리하도록 변경하겠습니다:


```c
void lan1qtsn_set_pulse_at_by_ldev(struct lan1qtsn_spi* tc6, sysclock_t time) {
    lan1qtsn_spi_write_register(tc6, PPS_PULSE_AT_H, (u32)(time >> 32));
    lan1qtsn_spi_write_register(tc6, PPS_PULSE_AT_L, (u32)(time));
}

->

void lan1qtsn_set_pulse_at_by_ldev(struct lan1qtsn_spi* tc6, sysclock_t time) {
    u32 values[2];
    values[0] = (u32)(time >> 32);
    values[1] = (u32)(time);
    lan1qtsn_spi_perform_ctrl(tc6, PPS_PULSE_AT_H, values, 2, LAN1QTSN_SPI_CTRL_REG_WRITE);
}
```

```c
void lan1qtsn_set_cycle_1s_by_ldev(struct lan1qtsn_spi* tc6, u32 cycle_1s) {
    lan1qtsn_spi_write_register(tc6, PPS_CYCLE_1S_H, 0);
    lan1qtsn_spi_write_register(tc6, PPS_CYCLE_1S_L, cycle_1s);
}

->

void lan1qtsn_set_cycle_1s_by_ldev(struct lan1qtsn_spi* tc6, u32 cycle_1s) {
    u32 values[2];
    values[0] = 0;
    values[1] = cycle_1s;
    lan1qtsn_spi_perform_ctrl(tc6, PPS_CYCLE_1S_H, values, 2, LAN1QTSN_SPI_CTRL_REG_WRITE);
}
```

```c
sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi* tc6) {
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    int ret;

    /* Try to acquire the lock without blocking */
    if (mutex_trylock(&tc6->spi_ctrl_lock)) {
        /* Successfully acquired the lock, safe to read registers */
        ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_H, &sys_clock_high, 1, LAN1QTSN_SPI_CTRL_REG_READ);
        if (ret == 0) {
            ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_L, &sys_clock_low, 1, LAN1QTSN_SPI_CTRL_REG_READ);
        }
        mutex_unlock(&tc6->spi_ctrl_lock);

        if (ret == 0) {
            clock = ((u64)sys_clock_high << 32) | sys_clock_low;
            return clock;
        }
    }

    /* If we couldn't acquire the lock or read failed, return 0 */
    dev_info(&tc6->spi->dev, "Could not read system clock safely\n");
    return 0;
}

->

sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi* tc6) {
    timestamp_t clock;
    u32 values[2];
    int ret;

    /* Try to acquire the lock without blocking */
    if (mutex_trylock(&tc6->spi_ctrl_lock)) {
        /* Successfully acquired the lock, safe to read registers */
        ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_H, values, 2, LAN1QTSN_SPI_CTRL_REG_READ);
        mutex_unlock(&tc6->spi_ctrl_lock);

        if (ret == 0) {
            clock = ((u64)values[0] << 32) | values[1];
            return clock;
        }
    }

    /* If we couldn't acquire the lock or read failed, return 0 */
    dev_info(&tc6->spi->dev, "Could not read system clock safely\n");
    return 0;
}
```

```c
timestamp_t lan1qtsn_read_tx_timestamp_by_ldev(struct lan1qtsn_spi* tc6, int tx_id) {
    u32 timestamp_h, timestamp_l;

    switch (tx_id) {
    case 1:
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP1_H, &timestamp_h);
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP1_L, &timestamp_l);
        break;
    case 2:
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP2_H, &timestamp_h);
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP2_L, &timestamp_l);
        break;
    case 3:
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP3_H, &timestamp_h);
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP3_L, &timestamp_l);
        break;
    case 4:
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP4_H, &timestamp_h);
        lan1qtsn_spi_read_register(tc6, FT_TX_TSTAMP4_L, &timestamp_l);
        break;
    default:
        return 0;
    }

    return ((timestamp_t)timestamp_h << 32 | (timestamp_t)timestamp_l);
}

->

timestamp_t lan1qtsn_read_tx_timestamp_by_ldev(struct lan1qtsn_spi* tc6, int tx_id) {
    u32 values[2];
    u32 base_addr;

    switch (tx_id) {
    case 1:
        base_addr = FT_TX_TSTAMP1_H;
        break;
    case 2:
        base_addr = FT_TX_TSTAMP2_H;
        break;
    case 3:
        base_addr = FT_TX_TSTAMP3_H;
        break;
    case 4:
        base_addr = FT_TX_TSTAMP4_H;
        break;
    default:
        return 0;
    }

    if (lan1qtsn_spi_perform_ctrl(tc6, base_addr, values, 2, LAN1QTSN_SPI_CTRL_REG_READ) == 0) {
        return ((timestamp_t)values[0] << 32 | (timestamp_t)values[1]);
    }

    return 0;
}
```

```c
u64 lan1qtsn_get_buffer_write_status_by_ldev(struct lan1qtsn_spi* tc6) {
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

->

u64 lan1qtsn_get_buffer_write_status_by_ldev(struct lan1qtsn_spi* tc6) {
    u32 values[2];
    if (lan1qtsn_spi_perform_ctrl(tc6, FBW_BUFFER_WRITE_STATUS1_H, values, 2, LAN1QTSN_SPI_CTRL_REG_READ) == 0) {
        return ((u64)values[0] << 32 | (u64)values[1]);
    }
    return 0;
}
```

```c
u64 lan1qtsn_get_total_new_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

->

u64 lan1qtsn_get_total_new_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 values[2];
    if (lan1qtsn_spi_perform_ctrl(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_H, values, 2, LAN1QTSN_SPI_CTRL_REG_READ) == 0) {
        return ((u64)values[0] << 32 | (u64)values[1]);
    }
    return 0;
}
```

```c
u64 lan1qtsn_get_total_valid_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

->

u64 lan1qtsn_get_total_valid_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 values[2];
    if (lan1qtsn_spi_perform_ctrl(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_H, values, 2, LAN1QTSN_SPI_CTRL_REG_READ) == 0) {
        return ((u64)values[0] << 32 | (u64)values[1]);
    }
    return 0;
}
```

```c
u64 lan1qtsn_get_total_drop_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

->

u64 lan1qtsn_get_total_drop_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 values[2];
    if (lan1qtsn_spi_perform_ctrl(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_H, values, 2, LAN1QTSN_SPI_CTRL_REG_READ) == 0) {
        return ((u64)values[0] << 32 | (u64)values[1]);
    }
    return 0;
}
```

완료되었습니다! 다음과 같이 코드를 수정했습니다:

## 수정된 내용

### 1. `lan1qtsn_spi.h` 파일
- `LAN1QTSN_SPI_CTRL_MAX_REGISTERS` 값을 1에서 2로 변경

### 2. `lan1qtsn_arch.c` 파일의 함수들

**쓰기 함수들 (한 번에 2개 레지스터에 값 설정):**
- `lan1qtsn_set_pulse_at_by_ldev`: PPS_PULSE_AT_H, PPS_PULSE_AT_L 레지스터를 한 번에 설정
- `lan1qtsn_set_cycle_1s_by_ldev`: PPS_CYCLE_1S_H, PPS_CYCLE_1S_L 레지스터를 한 번에 설정

**읽기 함수들 (한 번에 2개 레지스터에서 값 읽기):**
- `lan1qtsn_get_sys_clock_by_ldev`: TSN_SYSCOUNT_H, TSN_SYSCOUNT_L 레지스터를 한 번에 읽기
- `lan1qtsn_read_tx_timestamp_by_ldev`: 각 TX 타임스탬프의 H, L 레지스터를 한 번에 읽기
- `lan1qtsn_get_buffer_write_status_by_ldev`: FBW_BUFFER_WRITE_STATUS1_H, L 레지스터를 한 번에 읽기
- `lan1qtsn_get_total_new_entry_by_ldev`: FSCH_TOTAL_NEW_ENTRY_CNT_H, L 레지스터를 한 번에 읽기
- `lan1qtsn_get_total_valid_entry_by_ldev`: FSCH_TOTAL_VALID_ENTRY_CNT_H, L 레지스터를 한 번에 읽기
- `lan1qtsn_get_total_drop_entry_by_ldev`: FSCH_TOTAL_DROP_ENTRY_CNT_H, L 레지스터를 한 번에 읽기

## 주요 변경사항

1. **배열 사용**: 각 함수에서 `u32 values[2]` 배열을 사용하여 2개의 값을 저장
2. **단일 호출**: `lan1qtsn_spi_perform_ctrl` 함수를 한 번만 호출하여 2개의 레지스터를 처리
3. **효율성 향상**: SPI 통신 횟수를 줄여 성능 향상
4. **에러 처리**: 읽기 함수들에서 실패 시 0 반환하도록 개선

이제 연속된 2개의 4바이트 레지스터를 한 번의 SPI 통신으로 처리할 수 있어 더 효율적으로 동작할 것입니다.

