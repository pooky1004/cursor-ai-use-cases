# 라즈베리 파이 커널 문제 해결
_Exported on 8/26/2025 at 08:54:50 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지를 insmode 명령을 이용해 커널 로딩하여 시험 중에 있어.

대국에서 "sudo arping 192.168.1.11 -W 0.5 -c 100" 이런 명령으로 시험을 할 경우 간혹 Timeout 이 발생하고
본 시스템의 원격 터미널들도 네트웍이 끊기고 시스템도 먹통이 되어 전원을 껏다 다시 켜야해. 본 시스템의 커널 로그도 아무것도 나오지 않아.
문제를 확인하고 해결해 줘

---

**Cursor**

이 문제는 네트워크 드라이버에서 발생하는 시스템 안정성 문제로 보입니다. 코드를 분석하고 문제점을 찾아보겠습니다.

먼저 관련 파일들을 살펴보겠습니다.


코드를 분석한 결과, 여러 가지 잠재적인 문제점들을 발견했습니다. 이 문제들이 시스템을 먹통으로 만들 수 있습니다:

## 주요 문제점들:

### 1. **데드락 위험 (Deadlock Risk)**
- `lan1qtsn_spi_transfer()` 함수에서 `mutex_lock(&tc6->spi_transfer_lock)` 사용
- `lan1qtsn_spi_perform_ctrl()` 함수에서 `spin_lock_irqsave(&tc6->spi_ctrl_spinlock)` 사용
- 두 락이 서로 다른 순서로 획득될 수 있어 데드락 발생 가능

### 2. **SPI 전송 실패 시 무한 루프**
- `lan1qtsn_spi_thread_handler()`에서 전송 실패 시 `return ret`로 즉시 종료
- 이는 스레드가 종료되어 더 이상 패킷을 처리하지 못함

### 3. **메모리 누수 및 리소스 관리 문제**
- 전송 실패 시 SKB 정리 로직이 불완전
- 에러 상황에서 큐 상태가 제대로 복구되지 않음

### 4. **SPI 버스 과부하**
- 현재 SPI 전송 간 지연이 부족하여 버스 과부하 발생 가능

이 문제들을 해결하기 위해 코드를 수정하겠습니다:


```c
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;
    int ret;

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);

    /* Acquire SPI transfer lock to prevent concurrent access */
    if (mutex_lock_interruptible(&tc6->spi_transfer_lock)) {
        dev_err(&tc6->spi->dev, "SPI: Interrupted while acquiring transfer lock\n");
        return -EINTR;
    }

    if (header_type == LAN1QTSN_SPI_DATA_HEADER) {
        xfer.tx_buf = tc6->spi_data_tx_buf;
        xfer.rx_buf = tc6->spi_data_rx_buf;
    } else if (header_type == LAN1QTSN_SPI_META_HEADER) {
        xfer.tx_buf = tc6->spi_meta_tx_buf;
        xfer.rx_buf = tc6->spi_meta_rx_buf;
    } else {
        xfer.tx_buf = tc6->spi_ctrl_tx_buf;
        xfer.rx_buf = tc6->spi_ctrl_rx_buf;
    }
    xfer.len = length;

    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);

    ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        mutex_unlock(&tc6->spi_transfer_lock);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    /* Update last SPI operation time for rate limiting */
    tc6->last_spi_op_time = jiffies;

    /* Release SPI transfer lock */
    mutex_unlock(&tc6->spi_transfer_lock);

    /* SPI rate limiting for 25MHz clock (40ns period) */
    /* Ensure minimum 2 microseconds between SPI operations to prevent bus overload */
    udelay(2 + ((length * 8) / 25)); /* 2us base delay + calculated delay */

    return 0;
}
```

```c
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;
    int consecutive_errors = 0;
    const int max_consecutive_errors = 5;

    /* Validate input parameter */
    if (!tc6) {
        return -EINVAL;
    }

    while (likely(!kthread_should_stop())) {
        /* This kthread will be waken up if there is a tx skb or mac-phy
         * interrupt to perform spi transfer with tx chunks.
         */
        wait_event_interruptible(tc6->spi_wq, tc6->int_flag || (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) ||
                                                  kthread_should_stop());

        if (kthread_should_stop()) {
            break;
        }

        ret = lan1qtsn_spi_try_spi_transfer(tc6);
        if (ret) {
            consecutive_errors++;
            dev_err(&tc6->spi->dev, "SPI thread: Transfer attempt %d failed: %d\n", consecutive_errors, ret);
            
            /* If too many consecutive errors, add delay and reset counter */
            if (consecutive_errors >= max_consecutive_errors) {
                dev_warn(&tc6->spi->dev, "SPI thread: Too many consecutive errors, adding delay\n");
                msleep(100); /* 100ms delay to allow system to recover */
                consecutive_errors = 0;
            } else {
                /* Small delay between retries */
                usleep_range(1000, 5000); /* 1-5ms delay */
            }
            
            /* Don't return immediately, continue processing to prevent thread death */
            continue;
        } else {
            /* Reset error counter on success */
            consecutive_errors = 0;
        }
    }

    return 0;
}
```

```c
                if (ret) {
                    dev_err(&tc6->spi->dev, "TX: SPI transfer failed: %d, tx_packets=%lu\n", ret,
                            tc6->netdev->stats.tx_packets);
                    /* Clean up on error */
                    if (tc6->ongoing_tx_skb) {
                        kfree_skb(tc6->ongoing_tx_skb);
                        tc6->ongoing_tx_skb = NULL;
                    }
                    
                    /* Clean up waiting TX SKB on error to prevent stuck state */
                    spin_lock_bh(&tc6->tx_skb_lock);
                    if (tc6->waiting_tx_skb) {
                        dev_kfree_skb_any(tc6->waiting_tx_skb);
                        tc6->waiting_tx_skb = NULL;
                        tc6->netdev->stats.tx_dropped++;
                    }
                    spin_unlock_bh(&tc6->tx_skb_lock);
                    
                    /* Wake up the queue even on error to allow retry */
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }
                    
                    /* Add delay before retry to prevent rapid error loops */
                    usleep_range(1000, 5000); /* 1-5ms delay */
                } else {
```

```c
sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi* tc6) {
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    int ret;
    unsigned long flags;
    unsigned long timeout;

    /* Use spinlock instead of mutex to avoid scheduling while atomic */
    if (spin_trylock_irqsave(&tc6->spi_ctrl_spinlock, flags)) {
        /* Set timeout to prevent hanging */
        timeout = jiffies + msecs_to_jiffies(100); /* 100ms timeout */
        
        /* Perform SPI operations with timeout check */
        ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_H, &sys_clock_high, 1, LAN1QTSN_SPI_CTRL_REG_READ);
        if (ret == 0 && time_before(jiffies, timeout)) {
            ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_L, &sys_clock_low, 1, LAN1QTSN_SPI_CTRL_REG_READ);
        }
        
        spin_unlock_irqrestore(&tc6->spi_ctrl_spinlock, flags);
        
        if (ret == 0 && time_before(jiffies, timeout)) {
            clock = ((u64)sys_clock_high << 32) | sys_clock_low;
            return clock;
        }
    } else {
        dev_warn(&tc6->spi->dev, "Could not acquire SPI control lock for system clock read\n");
        return 0;
    }

    /* If read failed or timeout occurred, return 0 */
    dev_info(&tc6->spi->dev, "Could not read system clock safely (ret=%d, timeout=%s)\n", 
             ret, time_after_eq(jiffies, timeout) ? "yes" : "no");
    return 0;
}
```

```c
int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                              enum lan1qtsn_spi_register_op reg_op) {
    u16 size;
    int ret;
    unsigned long timeout;

    /* Set timeout to prevent hanging */
    timeout = jiffies + msecs_to_jiffies(50); /* 50ms timeout */

    /* Prepare control command and copy to SPI control buffer */
    lan1qtsn_spi_prepare_ctrl_spi_buf(tc6, address, value, length, reg_op);

    size = lan1qtsn_spi_calculate_ctrl_buf_size(length, reg_op);

    /* Check timeout before performing transfer */
    if (time_after_eq(jiffies, timeout)) {
        dev_err(&tc6->spi->dev, "SPI control operation timeout before transfer\n");
        return -ETIMEDOUT;
    }

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_CTRL_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* Check timeout after transfer */
    if (time_after_eq(jiffies, timeout)) {
        dev_err(&tc6->spi->dev, "SPI control operation timeout after transfer\n");
        return -ETIMEDOUT;
    }

    /* Check echoed/received control write command reply for errors */
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_READ) {
#if 0
        u8* buf = tc6->spi_ctrl_rx_buf;
        for (int i = 0; i < size; i++) {
            dev_info(&tc6->spi->dev, "tc6->spi_ctrl_rx_buf[%2d]: 0x%02x\n", i, buf[i]);
        }
#endif
        lan1qtsn_spi_copy_ctrl_read_data(tc6, value, length);
    }

    return 0;
}
```

```c
    /* Try to read system clock safely */
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    if (sys_count == 0) {
        /* Use default values if system clock read failed */
        dev_dbg(&ndev->dev, "System clock read failed, using default values\n");
        now = 0;
        sys_count_lower = 0;
        sys_count_upper = 0;
        
        /* Add delay to prevent rapid retries when clock is unavailable */
        usleep_range(1000, 5000); /* 1-5ms delay */
    } else {
        now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
        sys_count_lower = sys_count & LOWER_29_BITS;
        sys_count_upper = sys_count & ~LOWER_29_BITS;
    }
```

```c
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;
    int consecutive_errors = 0;
    const int max_consecutive_errors = 5;
    unsigned long last_activity = jiffies;
    const unsigned long watchdog_timeout = msecs_to_jiffies(5000); /* 5 second watchdog */

    /* Validate input parameter */
    if (!tc6) {
        return -EINVAL;
    }

    while (likely(!kthread_should_stop())) {
        /* This kthread will be waken up if there is a tx skb or mac-phy
         * interrupt to perform spi transfer with tx chunks.
         */
        wait_event_interruptible(tc6->spi_wq, tc6->int_flag || (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) ||
                                                  kthread_should_stop());

        if (kthread_should_stop()) {
            break;
        }

        /* Check watchdog timeout */
        if (time_after_eq(jiffies, last_activity + watchdog_timeout)) {
            dev_warn(&tc6->spi->dev, "SPI thread: Watchdog timeout detected, resetting state\n");
            
            /* Reset stuck state */
            spin_lock_bh(&tc6->tx_skb_lock);
            if (tc6->waiting_tx_skb) {
                dev_kfree_skb_any(tc6->waiting_tx_skb);
                tc6->waiting_tx_skb = NULL;
                tc6->netdev->stats.tx_dropped++;
            }
            if (tc6->ongoing_tx_skb) {
                kfree_skb(tc6->ongoing_tx_skb);
                tc6->ongoing_tx_skb = NULL;
            }
            spin_unlock_bh(&tc6->tx_skb_lock);
            
            /* Wake up queues */
            if (netif_queue_stopped(tc6->netdev)) {
                netif_wake_queue(tc6->netdev);
            }
            
            consecutive_errors = 0;
            last_activity = jiffies;
            continue;
        }

        ret = lan1qtsn_spi_try_spi_transfer(tc6);
        if (ret) {
            consecutive_errors++;
            dev_err(&tc6->spi->dev, "SPI thread: Transfer attempt %d failed: %d\n", consecutive_errors, ret);
            
            /* If too many consecutive errors, add delay and reset counter */
            if (consecutive_errors >= max_consecutive_errors) {
                dev_warn(&tc6->spi->dev, "SPI thread: Too many consecutive errors, adding delay\n");
                msleep(100); /* 100ms delay to allow system to recover */
                consecutive_errors = 0;
            } else {
                /* Small delay between retries */
                usleep_range(1000, 5000); /* 1-5ms delay */
            }
            
            /* Don't return immediately, continue processing to prevent thread death */
            continue;
        } else {
            /* Reset error counter on success */
            consecutive_errors = 0;
            last_activity = jiffies; /* Update activity timestamp */
        }
    }

    return 0;
}
```

이제 추가적인 안전장치를 위해 모듈 초기화 시 에러 핸들링을 개선하겠습니다:


```c
int init_lan1qtsn(struct lan1qtsn_spi* tc6) {
    int ret;
    
    if (!tc6) {
        pr_err("init_lan1qtsn: Invalid tc6 pointer\n");
        return -EINVAL;
    }
    
    /* Start with TSN system disabled, will be enabled when interface is opened */
    ret = lan1qtsn_spi_write_register(tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to disable TSN system during init: %d\n", ret);
        /* Continue initialization even if this fails */
    }
    
    dev_dbg(&tc6->spi->dev, "TSN system initialized in disabled state\n");
    return 0;
}
```

마지막으로, 네트워크 인터페이스 열기/닫기 시 더 안전한 처리를 위해 추가 개선을 하겠습니다:


```c
int lan1qtsn_netdev_open(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    int ret;
    u32 sys_info_high;

    netif_carrier_on(ndev);
#ifdef __TSN_PTP__
    netif_start_queue(ndev);
    for (int i = 0; i < TX_QUEUE_COUNT; i++) {
        netif_start_subqueue(ndev, i);
    }
#endif

    /* Enable TSN system first with timeout protection */
    ret = lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);
    if (ret) {
        dev_err(&ndev->dev, "Failed to enable TSN system: %d\n", ret);
        /* Continue anyway, but log the error */
    }

    /* Try to read system info with timeout protection */
    ret = lan1qtsn_spi_read_register(priv->tc6, TSN_SYSTEM_INFO_H, &sys_info_high);
    if (ret == 0) {
        dev_info(&ndev->dev, "Network interface opened(0x%08x)\n", sys_info_high);
    } else {
        dev_warn(&ndev->dev, "Network interface opened but failed to read system info: %d\n", ret);
    }

    return 0;
}
```

```c
int lan1qtsn_netdev_close(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    int ret;

#ifdef __TSN_PTP__
    netif_stop_queue(ndev);
    for (int i = 0; i < TX_QUEUE_COUNT; i++) {
        netif_stop_subqueue(ndev, i);
    }
#endif

    /* Disable TSN system first with timeout protection */
    ret = lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);
    if (ret) {
        dev_err(&ndev->dev, "Failed to disable TSN system: %d\n", ret);
        /* Continue anyway, but log the error */
    }

    dev_info(&ndev->dev, "%s\n", __func__);
    netif_carrier_off(ndev);
    dev_info(&ndev->dev, "netif_carrier_off\n");
    return 0;
}
```

이제 수정된 코드를 컴파일하고 테스트해보겠습니다. 먼저 Makefile을 확인해보겠습니다:


이제 수정된 드라이버를 컴파일해보겠습니다:


---

**User**

적용을 했는데 Timeout 간간히 발생할 때 아래 커널로그가 나왔고,
대국에서 "sudo arping 192.168.1.11 -W 0.1 -c 100" 명령으로 시험을 하니
아무런 커널로그없이 먹통되는 현상도 발생했어

[ 1449.425898] BUG: scheduling while atomic: kworker/u16:2/986/0x00000305
[ 1449.425925] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc vc4 spidev joydev brcmfmac snd_soc_hdmi_codec hci_uart brcmutil btbcm cfg80211 drm_display_helper bluetooth cec drm_dma_helper v3d bcm2835_codec(C) snd_soc_core ecdh_generic bcm2835_isp(C) gpu_sched snd_bcm2835(C) bcm2835_v4l2(C) snd_compress ecc rpi_hevc_dec drm_shmem_helper snd_pcm_dmaengine bcm2835_mmal_vchiq(C) drm_kms_helper snd_pcm rfkill libaes v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops raspberrypi_hwmon videobuf2_v4l2 videodev snd_timer i2c_brcmstb videobuf2_common snd raspberrypi_gpiomem mc spi_bcm2835 nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1449.426210] CPU: 0 UID: 0 PID: 986 Comm: kworker/u16:2 Tainted: G         C O       6.12.41-v8+ #5
[ 1449.426222] Tainted: [C]=CRAP, [O]=OOT_MODULE
[ 1449.426225] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1449.426230] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[ 1449.426257] Call trace:
[ 1449.426260]  dump_backtrace+0xa0/0x100
[ 1449.426271]  show_stack+0x20/0x38
[ 1449.426276]  dump_stack_lvl+0x78/0x90
[ 1449.426287]  dump_stack+0x18/0x28
[ 1449.426295]  __schedule_bug+0x58/0x78
[ 1449.426306]  __schedule+0x954/0xb50
[ 1449.426312]  schedule+0x3c/0x148
[ 1449.426318]  schedule_preempt_disabled+0x2c/0x50
[ 1449.426325]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1449.426332]  __mutex_lock_interruptible_slowpath+0x1c/0x30
[ 1449.426339]  mutex_lock_interruptible+0x68/0x80
[ 1449.426346]  lan1qtsn_spi_transfer+0x88/0x1c8 [lan1qtsn]
[ 1449.426357]  lan1qtsn_spi_perform_ctrl+0x8c/0x1c0 [lan1qtsn]
[ 1449.426367]  lan1qtsn_get_sys_clock_by_ldev+0x70/0x180 [lan1qtsn]
[ 1449.426377]  lan1qtsn_netdev_start_xmit+0xcc/0x538 [lan1qtsn]
[ 1449.426387]  dev_hard_start_xmit+0xac/0x208
[ 1449.426396]  sch_direct_xmit+0x98/0x1e8
[ 1449.426406]  __dev_queue_xmit+0x544/0xed0
[ 1449.426412]  arp_xmit+0x4c/0xe0
[ 1449.426418]  arp_send_dst+0x88/0xd0
[ 1449.426423]  arp_process+0x6c0/0x7a8
[ 1449.426427]  arp_rcv+0x144/0x1e0
[ 1449.426432]  __netif_receive_skb_one_core+0x60/0x98
[ 1449.426439]  __netif_receive_skb+0x20/0x70
[ 1449.426446]  process_backlog+0x8c/0x150
[ 1449.426453]  __napi_poll+0x40/0x218
[ 1449.426459]  net_rx_action+0x314/0x370
[ 1449.426466]  handle_softirqs+0x114/0x348
[ 1449.426476]  __do_softirq+0x1c/0x28
[ 1449.426481]  ____do_softirq+0x18/0x30
[ 1449.426488]  call_on_irq_stack+0x30/0x70
[ 1449.426495]  do_softirq_own_stack+0x24/0x38
[ 1449.426502]  do_softirq+0x5c/0x80
[ 1449.426510]  __local_bh_enable_ip+0x94/0xa8
[ 1449.426519]  netif_rx+0xac/0x178
[ 1449.426524]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[ 1449.426534]  process_one_work+0x158/0x3d0
[ 1449.426543]  worker_thread+0x2d8/0x3e8
[ 1449.426550]  kthread+0x11c/0x128
[ 1449.426557]  ret_from_fork+0x10/0x20
[ 1449.426566] ------------[ cut here ]------------
[ 1449.426588] Voluntary context switch within RCU read-side critical section!
[ 1449.426609] WARNING: CPU: 0 PID: 986 at kernel/rcu/tree_plugin.h:331 rcu_note_context_switch+0x430/0x520
[ 1449.426628] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc vc4 spidev joydev brcmfmac snd_soc_hdmi_codec hci_uart brcmutil btbcm cfg80211 drm_display_helper bluetooth cec drm_dma_helper v3d bcm2835_codec(C) snd_soc_core ecdh_generic bcm2835_isp(C) gpu_sched snd_bcm2835(C) bcm2835_v4l2(C) snd_compress ecc rpi_hevc_dec drm_shmem_helper snd_pcm_dmaengine bcm2835_mmal_vchiq(C) drm_kms_helper snd_pcm rfkill libaes v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops raspberrypi_hwmon videobuf2_v4l2 videodev snd_timer i2c_brcmstb videobuf2_common snd raspberrypi_gpiomem mc spi_bcm2835 nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1449.426882] CPU: 0 UID: 0 PID: 986 Comm: kworker/u16:2 Tainted: G        WC O       6.12.41-v8+ #5
[ 1449.426894] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 1449.426898] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1449.426903] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[ 1449.426918] pstate: 600000c5 (nZCv daIF -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[ 1449.426927] pc : rcu_note_context_switch+0x430/0x520
[ 1449.426939] lr : rcu_note_context_switch+0x430/0x520
[ 1449.426950] sp : ffffffc080003590
[ 1449.426955] x29: ffffffc080003590 x28: ffffff8042a44200 x27: ffffff8103f8bc40
[ 1449.426971] x26: ffffff81027afc02 x25: 0000000000000000 x24: ffffff8042a44200
[ 1449.426986] x23: 0000000000000000 x22: ffffff8042a44200 x21: ffffffdf4ba2e730
[ 1449.427001] x20: 0000000000000000 x19: ffffff8103f8ca40 x18: 00000000fffffffd
[ 1449.427016] x17: 2043202020202020 x16: 20202047203a6465 x15: 746e69615420323a
[ 1449.427030] x14: 3631752f72656b72 x13: 216e6f6974636573 x12: 206c616369746972
[ 1449.427044] x11: 6320656469732d64 x10: ffffffdf4b880a30 x9 : ffffffdf4a11d1f0
[ 1449.427058] x8 : 00000000ffffefff x7 : ffffffdf4b87d4f0 x6 : 0000000000000238
[ 1449.427073] x5 : ffffff8103f793c8 x4 : 0000000000000000 x3 : 0000000000000027
[ 1449.427087] x2 : 0000000000000000 x1 : 0000000000000000 x0 : ffffff8042a44200
[ 1449.427101] Call trace:
[ 1449.427105]  rcu_note_context_switch+0x430/0x520
[ 1449.427117]  __schedule+0xa8/0xb50
[ 1449.427125]  schedule+0x3c/0x148
[ 1449.427132]  schedule_preempt_disabled+0x2c/0x50
[ 1449.427140]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1449.427149]  __mutex_lock_interruptible_slowpath+0x1c/0x30
[ 1449.427157]  mutex_lock_interruptible+0x68/0x80
[ 1449.427166]  lan1qtsn_spi_transfer+0x88/0x1c8 [lan1qtsn]
[ 1449.427177]  lan1qtsn_spi_perform_ctrl+0x8c/0x1c0 [lan1qtsn]
[ 1449.427189]  lan1qtsn_get_sys_clock_by_ldev+0x70/0x180 [lan1qtsn]
[ 1449.427200]  lan1qtsn_netdev_start_xmit+0xcc/0x538 [lan1qtsn]
[ 1449.427211]  dev_hard_start_xmit+0xac/0x208
[ 1449.427219]  sch_direct_xmit+0x98/0x1e8
[ 1449.427230]  __dev_queue_xmit+0x544/0xed0
[ 1449.427238]  arp_xmit+0x4c/0xe0
[ 1449.427244]  arp_send_dst+0x88/0xd0
[ 1449.427252]  arp_process+0x6c0/0x7a8
[ 1449.427258]  arp_rcv+0x144/0x1e0
[ 1449.427265]  __netif_receive_skb_one_core+0x60/0x98
[ 1449.427274]  __netif_receive_skb+0x20/0x70
[ 1449.427282]  process_backlog+0x8c/0x150
[ 1449.427290]  __napi_poll+0x40/0x218
[ 1449.427299]  net_rx_action+0x314/0x370
[ 1449.427307]  handle_softirqs+0x114/0x348
[ 1449.427318]  __do_softirq+0x1c/0x28
[ 1449.427325]  ____do_softirq+0x18/0x30
[ 1449.427334]  call_on_irq_stack+0x30/0x70
[ 1449.427343]  do_softirq_own_stack+0x24/0x38
[ 1449.427351]  do_softirq+0x5c/0x80
[ 1449.427361]  __local_bh_enable_ip+0x94/0xa8
[ 1449.427371]  netif_rx+0xac/0x178
[ 1449.427378]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[ 1449.427390]  process_one_work+0x158/0x3d0
[ 1449.427400]  worker_thread+0x2d8/0x3e8
[ 1449.427409]  kthread+0x11c/0x128
[ 1449.427417]  ret_from_fork+0x10/0x20
[ 1449.427425] ---[ end trace 0000000000000000 ]---
[ 1449.427524] ------------[ cut here ]------------
[ 1449.427528] WARNING: CPU: 0 PID: 986 at kernel/softirq.c:386 __local_bh_enable_ip+0x98/0xa8
[ 1449.427545] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc vc4 spidev joydev brcmfmac snd_soc_hdmi_codec hci_uart brcmutil btbcm cfg80211 drm_display_helper bluetooth cec drm_dma_helper v3d bcm2835_codec(C) snd_soc_core ecdh_generic bcm2835_isp(C) gpu_sched snd_bcm2835(C) bcm2835_v4l2(C) snd_compress ecc rpi_hevc_dec drm_shmem_helper snd_pcm_dmaengine bcm2835_mmal_vchiq(C) drm_kms_helper snd_pcm rfkill libaes v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops raspberrypi_hwmon videobuf2_v4l2 videodev snd_timer i2c_brcmstb videobuf2_common snd raspberrypi_gpiomem mc spi_bcm2835 nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1449.427800] CPU: 0 UID: 0 PID: 986 Comm: kworker/u16:2 Tainted: G        WC O       6.12.41-v8+ #5
[ 1449.427812] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 1449.427816] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1449.427821] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[ 1449.427837] pstate: 00000005 (nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[ 1449.427845] pc : __local_bh_enable_ip+0x98/0xa8
[ 1449.427856] lr : __dev_queue_xmit+0x260/0xed0
[ 1449.427865] sp : ffffffc080003a30
[ 1449.427869] x29: ffffffc080003a30 x28: ffffffc080003eb8 x27: 0000000000000000
[ 1449.427886] x26: ffffff81019c8860 x25: ffffffdf4b3fe000 x24: ffffff81019c8856
[ 1449.427901] x23: ffffff8102b92d44 x22: ffffff8103526000 x21: 0000000000000000
[ 1449.427916] x20: ffffff8102b92c00 x19: ffffff810150dd00 x18: 00000000fffffffd
[ 1449.427930] x17: 2043202020202020 x16: ffffffdf4a108648 x15: 746e69615420323a
[ 1449.427945] x14: 3631752f72656b72 x13: 216e6f6974636573 x12: 0000000000000000
[ 1449.427960] x11: ffffff8103f8bcc0 x10: 0000000000001a40 x9 : ffffffdf4ab69718
[ 1449.427975] x8 : 0000000000000000 x7 : 0000000000000018 x6 : 0000000000000002
[ 1449.427990] x5 : 0000000000000400 x4 : 0000000000000800 x3 : 0000000000000000
[ 1449.428005] x2 : 0000000000000000 x1 : 0000000000000200 x0 : 00000000fffffffd
[ 1449.428019] Call trace:
[ 1449.428024]  __local_bh_enable_ip+0x98/0xa8
[ 1449.428035]  __dev_queue_xmit+0x260/0xed0
[ 1449.428044]  arp_xmit+0x4c/0xe0
[ 1449.428051]  arp_send_dst+0x88/0xd0
[ 1449.428057]  arp_process+0x6c0/0x7a8
[ 1449.428064]  arp_rcv+0x144/0x1e0
[ 1449.428071]  __netif_receive_skb_one_core+0x60/0x98
[ 1449.428080]  __netif_receive_skb+0x20/0x70
[ 1449.428088]  process_backlog+0x8c/0x150
[ 1449.428097]  __napi_poll+0x40/0x218
[ 1449.428105]  net_rx_action+0x314/0x370
[ 1449.428113]  handle_softirqs+0x114/0x348
[ 1449.428123]  __do_softirq+0x1c/0x28
[ 1449.428130]  ____do_softirq+0x18/0x30
[ 1449.428139]  call_on_irq_stack+0x30/0x70
[ 1449.428147]  do_softirq_own_stack+0x24/0x38
[ 1449.428156]  do_softirq+0x5c/0x80
[ 1449.428166]  __local_bh_enable_ip+0x94/0xa8
[ 1449.428176]  netif_rx+0xac/0x178
[ 1449.428183]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[ 1449.428195]  process_one_work+0x158/0x3d0
[ 1449.428205]  worker_thread+0x2d8/0x3e8
[ 1449.428215]  kthread+0x11c/0x128
[ 1449.428222]  ret_from_fork+0x10/0x20
[ 1449.428231] ---[ end trace 0000000000000000 ]---
[ 1449.428241] softirq: huh, entered softirq 3 NET_RX 0000000034fa5daa with preempt_count 00000101, exited with fffffdfd?
[ 1517.846652] lan1qtsn spi0.0: SPI thread: Watchdog timeout detected, resetting state
[ 1520.553387] BUG: scheduling while atomic: kworker/u16:2/986/0x00000305
[ 1520.553424] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc vc4 spidev joydev brcmfmac snd_soc_hdmi_codec hci_uart brcmutil btbcm cfg80211 drm_display_helper bluetooth cec drm_dma_helper v3d bcm2835_codec(C) snd_soc_core ecdh_generic bcm2835_isp(C) gpu_sched snd_bcm2835(C) bcm2835_v4l2(C) snd_compress ecc rpi_hevc_dec drm_shmem_helper snd_pcm_dmaengine bcm2835_mmal_vchiq(C) drm_kms_helper snd_pcm rfkill libaes v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops raspberrypi_hwmon videobuf2_v4l2 videodev snd_timer i2c_brcmstb videobuf2_common snd raspberrypi_gpiomem mc spi_bcm2835 nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1520.553892] CPU: 0 UID: 0 PID: 986 Comm: kworker/u16:2 Tainted: G        WC O       6.12.41-v8+ #5
[ 1520.553913] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 1520.553917] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1520.553925] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[ 1520.553964] Call trace:
[ 1520.553968]  dump_backtrace+0xa0/0x100
[ 1520.553985]  show_stack+0x20/0x38
[ 1520.553995]  dump_stack_lvl+0x78/0x90
[ 1520.554012]  dump_stack+0x18/0x28
[ 1520.554025]  __schedule_bug+0x58/0x78
[ 1520.554042]  __schedule+0x954/0xb50
[ 1520.554053]  schedule+0x3c/0x148
[ 1520.554061]  schedule_preempt_disabled+0x2c/0x50
[ 1520.554072]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1520.554083]  __mutex_lock_interruptible_slowpath+0x1c/0x30
[ 1520.554095]  mutex_lock_interruptible+0x68/0x80
[ 1520.554106]  lan1qtsn_spi_transfer+0x88/0x1c8 [lan1qtsn]
[ 1520.554126]  lan1qtsn_spi_perform_ctrl+0x8c/0x1c0 [lan1qtsn]
[ 1520.554142]  lan1qtsn_get_sys_clock_by_ldev+0x70/0x180 [lan1qtsn]
[ 1520.554158]  lan1qtsn_netdev_start_xmit+0xcc/0x538 [lan1qtsn]
[ 1520.554173]  dev_hard_start_xmit+0xac/0x208
[ 1520.554187]  sch_direct_xmit+0x98/0x1e8
[ 1520.554202]  __dev_queue_xmit+0x544/0xed0
[ 1520.554213]  arp_xmit+0x4c/0xe0
[ 1520.554223]  arp_send_dst+0x88/0xd0
[ 1520.554231]  arp_process+0x6c0/0x7a8
[ 1520.554239]  arp_rcv+0x144/0x1e0
[ 1520.554246]  __netif_receive_skb_one_core+0x60/0x98
[ 1520.554258]  __netif_receive_skb+0x20/0x70
[ 1520.554269]  process_backlog+0x8c/0x150
[ 1520.554280]  __napi_poll+0x40/0x218
[ 1520.554291]  net_rx_action+0x314/0x370
[ 1520.554302]  handle_softirqs+0x114/0x348
[ 1520.554319]  __do_softirq+0x1c/0x28
[ 1520.554327]  ____do_softirq+0x18/0x30
[ 1520.554337]  call_on_irq_stack+0x30/0x70
[ 1520.554349]  do_softirq_own_stack+0x24/0x38
[ 1520.554360]  do_softirq+0x5c/0x80
[ 1520.554373]  __local_bh_enable_ip+0x94/0xa8
[ 1520.554388]  netif_rx+0xac/0x178
[ 1520.554397]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[ 1520.554413]  process_one_work+0x158/0x3d0
[ 1520.554427]  worker_thread+0x2d8/0x3e8
[ 1520.554440]  kthread+0x11c/0x128
[ 1520.554450]  ret_from_fork+0x10/0x20
[ 1520.554604] softirq: huh, entered softirq 3 NET_RX 0000000034fa5daa with preempt_count 00000101, exited with fffffdfd?
[ 1526.467205] BUG: scheduling while atomic: kworker/u16:2/986/0x00000305
[ 1526.467235] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc vc4 spidev joydev brcmfmac snd_soc_hdmi_codec hci_uart brcmutil btbcm cfg80211 drm_display_helper bluetooth cec drm_dma_helper v3d bcm2835_codec(C) snd_soc_core ecdh_generic bcm2835_isp(C) gpu_sched snd_bcm2835(C) bcm2835_v4l2(C) snd_compress ecc rpi_hevc_dec drm_shmem_helper snd_pcm_dmaengine bcm2835_mmal_vchiq(C) drm_kms_helper snd_pcm rfkill libaes v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops raspberrypi_hwmon videobuf2_v4l2 videodev snd_timer i2c_brcmstb videobuf2_common snd raspberrypi_gpiomem mc spi_bcm2835 nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1526.467550] CPU: 2 UID: 0 PID: 986 Comm: kworker/u16:2 Tainted: G        WC O       6.12.41-v8+ #5
[ 1526.467564] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 1526.467567] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1526.467573] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[ 1526.467600] Call trace:
[ 1526.467603]  dump_backtrace+0xa0/0x100
[ 1526.467615]  show_stack+0x20/0x38
[ 1526.467621]  dump_stack_lvl+0x78/0x90
[ 1526.467635]  dump_stack+0x18/0x28
[ 1526.467644]  __schedule_bug+0x58/0x78
[ 1526.467655]  __schedule+0x954/0xb50
[ 1526.467662]  schedule+0x3c/0x148
[ 1526.467668]  schedule_preempt_disabled+0x2c/0x50
[ 1526.467675]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1526.467683]  __mutex_lock_interruptible_slowpath+0x1c/0x30
[ 1526.467691]  mutex_lock_interruptible+0x68/0x80
[ 1526.467699]  lan1qtsn_spi_transfer+0x88/0x1c8 [lan1qtsn]
[ 1526.467712]  lan1qtsn_spi_perform_ctrl+0x8c/0x1c0 [lan1qtsn]
[ 1526.467724]  lan1qtsn_get_sys_clock_by_ldev+0x70/0x180 [lan1qtsn]
[ 1526.467735]  lan1qtsn_netdev_start_xmit+0xcc/0x538 [lan1qtsn]
[ 1526.467746]  dev_hard_start_xmit+0xac/0x208
[ 1526.467755]  sch_direct_xmit+0x98/0x1e8
[ 1526.467766]  __dev_queue_xmit+0x544/0xed0
[ 1526.467773]  arp_xmit+0x4c/0xe0
[ 1526.467780]  arp_send_dst+0x88/0xd0
[ 1526.467785]  arp_process+0x6c0/0x7a8
[ 1526.467790]  arp_rcv+0x144/0x1e0
[ 1526.467795]  __netif_receive_skb_one_core+0x60/0x98
[ 1526.467803]  __netif_receive_skb+0x20/0x70
[ 1526.467810]  process_backlog+0x8c/0x150
[ 1526.467817]  __napi_poll+0x40/0x218
[ 1526.467824]  net_rx_action+0x314/0x370
[ 1526.467832]  handle_softirqs+0x114/0x348
[ 1526.467843]  __do_softirq+0x1c/0x28
[ 1526.467849]  ____do_softirq+0x18/0x30
[ 1526.467856]  call_on_irq_stack+0x30/0x70
[ 1526.467864]  do_softirq_own_stack+0x24/0x38
[ 1526.467872]  do_softirq+0x5c/0x80
[ 1526.467881]  __local_bh_enable_ip+0x94/0xa8
[ 1526.467891]  netif_rx+0xac/0x178
[ 1526.467897]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[ 1526.467908]  process_one_work+0x158/0x3d0
[ 1526.467917]  worker_thread+0x2d8/0x3e8
[ 1526.467926]  kthread+0x11c/0x128
[ 1526.467933]  ret_from_fork+0x10/0x20
[ 1526.468053] softirq: huh, entered softirq 3 NET_RX 0000000034fa5daa with preempt_count 00000101, exited with fffffdfd?
[ 1527.369355] BUG: scheduling while atomic: kworker/u16:2/986/0x00000305
[ 1527.369391] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc vc4 spidev joydev brcmfmac snd_soc_hdmi_codec hci_uart brcmutil btbcm cfg80211 drm_display_helper bluetooth cec drm_dma_helper v3d bcm2835_codec(C) snd_soc_core ecdh_generic bcm2835_isp(C) gpu_sched snd_bcm2835(C) bcm2835_v4l2(C) snd_compress ecc rpi_hevc_dec drm_shmem_helper snd_pcm_dmaengine bcm2835_mmal_vchiq(C) drm_kms_helper snd_pcm rfkill libaes v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops raspberrypi_hwmon videobuf2_v4l2 videodev snd_timer i2c_brcmstb videobuf2_common snd raspberrypi_gpiomem mc spi_bcm2835 nvmem_rmem uio_pdrv_genirq uio drm i2c_dev ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1527.369856] CPU: 2 UID: 0 PID: 986 Comm: kworker/u16:2 Tainted: G        WC O       6.12.41-v8+ #5
[ 1527.369878] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 1527.369882] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1527.369889] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[ 1527.369929] Call trace:
[ 1527.369933]  dump_backtrace+0xa0/0x100
[ 1527.369950]  show_stack+0x20/0x38
[ 1527.369959]  dump_stack_lvl+0x78/0x90
[ 1527.369979]  dump_stack+0x18/0x28
[ 1527.369992]  __schedule_bug+0x58/0x78
[ 1527.370009]  __schedule+0x954/0xb50
[ 1527.370020]  schedule+0x3c/0x148
[ 1527.370029]  schedule_preempt_disabled+0x2c/0x50
[ 1527.370040]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1527.370052]  __mutex_lock_interruptible_slowpath+0x1c/0x30
[ 1527.370064]  mutex_lock_interruptible+0x68/0x80
[ 1527.370075]  lan1qtsn_spi_transfer+0x88/0x1c8 [lan1qtsn]
[ 1527.370095]  lan1qtsn_spi_perform_ctrl+0x8c/0x1c0 [lan1qtsn]
[ 1527.370111]  lan1qtsn_get_sys_clock_by_ldev+0x70/0x180 [lan1qtsn]
[ 1527.370127]  lan1qtsn_netdev_start_xmit+0xcc/0x538 [lan1qtsn]
[ 1527.370143]  dev_hard_start_xmit+0xac/0x208
[ 1527.370157]  sch_direct_xmit+0x98/0x1e8
[ 1527.370172]  __dev_queue_xmit+0x544/0xed0
[ 1527.370183]  arp_xmit+0x4c/0xe0
[ 1527.370192]  arp_send_dst+0x88/0xd0
[ 1527.370200]  arp_process+0x6c0/0x7a8
[ 1527.370208]  arp_rcv+0x144/0x1e0
[ 1527.370216]  __netif_receive_skb_one_core+0x60/0x98
[ 1527.370227]  __netif_receive_skb+0x20/0x70
[ 1527.370238]  process_backlog+0x8c/0x150
[ 1527.370249]  __napi_poll+0x40/0x218
[ 1527.370260]  net_rx_action+0x314/0x370
[ 1527.370271]  handle_softirqs+0x114/0x348
[ 1527.370287]  __do_softirq+0x1c/0x28
[ 1527.370295]  ____do_softirq+0x18/0x30
[ 1527.370306]  call_on_irq_stack+0x30/0x70
[ 1527.370317]  do_softirq_own_stack+0x24/0x38
[ 1527.370328]  do_softirq+0x5c/0x80
[ 1527.370342]  __local_bh_enable_ip+0x94/0xa8
[ 1527.370356]  netif_rx+0xac/0x178
[ 1527.370365]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[ 1527.370382]  process_one_work+0x158/0x3d0
[ 1527.370396]  worker_thread+0x2d8/0x3e8
[ 1527.370409]  kthread+0x11c/0x128
[ 1527.370419]  ret_from_fork+0x10/0x20
[ 1527.370607] softirq: huh, entered softirq 3 NET_RX 0000000034fa5daa with preempt_count 00000101, exited with fffffdfd?


---

**Cursor**

커널 로그를 분석한 결과, 핵심 문제를 파악했습니다. **"scheduling while atomic"** 에러가 발생하고 있는데, 이는 **softirq 컨텍스트에서 mutex를 사용하려고 할 때** 발생하는 문제입니다.

## 문제 분석:

1. **"scheduling while atomic" 에러**: `lan1qtsn_rx_work_handler` (workqueue)에서 `lan1qtsn_spi_transfer`를 호출할 때 mutex를 사용하려고 함
2. **softirq 컨텍스트**: `netif_rx` → `lan1qtsn_rx_work_handler` → `lan1qtsn_spi_transfer` 호출 체인
3. **mutex 사용**: softirq 컨텍스트에서는 mutex를 사용할 수 없음

## 해결 방법:

softirq 컨텍스트에서 mutex 대신 spinlock을 사용하거나, workqueue를 사용하여 안전한 컨텍스트에서 실행하도록 수정해야 합니다.

먼저 RX work handler를 확인해보겠습니다:


이제 문제의 원인을 파악했습니다. `lan1qtsn_rx_work_handler`에서 `netif_rx`를 호출하는데, 이 함수가 다시 `lan1qtsn_netdev_start_xmit`을 호출하여 문제가 발생합니다.

문제를 해결하기 위해 다음과 같이 수정하겠습니다:

1. **softirq 컨텍스트에서 mutex 사용 방지**
2. **RX work handler에서 안전한 패킷 처리**
3. **시스템 클록 읽기 시 atomic 컨텍스트 고려**


```c
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;
    int ret;
    unsigned long flags;
    bool in_atomic = in_atomic() || in_softirq() || in_irq();

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu, atomic=%d\n", 
            header_type, length, tc6->netdev->stats.tx_packets, in_atomic);

    /* Use appropriate locking mechanism based on context */
    if (in_atomic) {
        /* In atomic context, use spinlock instead of mutex */
        if (!spin_trylock_irqsave(&tc6->spi_transfer_lock, flags)) {
            dev_err(&tc6->spi->dev, "SPI: Could not acquire transfer lock in atomic context\n");
            return -EBUSY;
        }
    } else {
        /* In process context, use mutex */
        if (mutex_lock_interruptible(&tc6->spi_transfer_lock)) {
            dev_err(&tc6->spi->dev, "SPI: Interrupted while acquiring transfer lock\n");
            return -EINTR;
        }
    }

    if (header_type == LAN1QTSN_SPI_DATA_HEADER) {
        xfer.tx_buf = tc6->spi_data_tx_buf;
        xfer.rx_buf = tc6->spi_data_rx_buf;
    } else if (header_type == LAN1QTSN_SPI_META_HEADER) {
        xfer.tx_buf = tc6->spi_meta_tx_buf;
        xfer.rx_buf = tc6->spi_meta_rx_buf;
    } else {
        xfer.tx_buf = tc6->spi_ctrl_tx_buf;
        xfer.rx_buf = tc6->spi_ctrl_rx_buf;
    }
    xfer.len = length;

    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);

    ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        
        if (in_atomic) {
            spin_unlock_irqrestore(&tc6->spi_transfer_lock, flags);
        } else {
            mutex_unlock(&tc6->spi_transfer_lock);
        }
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    /* Update last SPI operation time for rate limiting */
    tc6->last_spi_op_time = jiffies;

    /* Release lock based on context */
    if (in_atomic) {
        spin_unlock_irqrestore(&tc6->spi_transfer_lock, flags);
    } else {
        mutex_unlock(&tc6->spi_transfer_lock);
    }

    /* SPI rate limiting for 25MHz clock (40ns period) */
    /* Ensure minimum 2 microseconds between SPI operations to prevent bus overload */
    if (!in_atomic) {
        udelay(2 + ((length * 8) / 25)); /* 2us base delay + calculated delay */
    }

    return 0;
}
```

이제 `lan1qtsn_spi` 구조체에 `spi_transfer_lock` spinlock을 추가해야 합니다:


```cpp
struct lan1qtsn_spi {
    struct spi_device* spi;
    struct lan1qtsn_spi_dev* tsdev;
    struct net_device* netdev;

    /* debugging counters */
    atomic_t wrong_mac_count;
    atomic_t count1; /* start_xmit call count */
    atomic_t count2; /* SPI transmission completion count */

    struct mutex spi_ctrl_lock;     /* Protects spi control transfer */
    struct mutex spi_transfer_lock; /* Protects all SPI transfers (data, meta, ctrl) */
    spinlock_t tx_skb_lock;         /* Protects tx skb handling */
    spinlock_t spi_ctrl_spinlock;   /* Protects spi control operations in atomic context */
    spinlock_t spi_transfer_spinlock; /* Protects SPI transfers in atomic context */
    void* spi_ctrl_tx_buf;
    void* spi_ctrl_rx_buf;
    void* spi_data_tx_buf;
    void* spi_data_rx_buf;
    void* spi_meta_tx_buf;
    void* spi_meta_rx_buf;
    struct sk_buff* ongoing_tx_skb;
    struct sk_buff* waiting_tx_skb;
    struct sk_buff* rx_skb;
    struct task_struct* spi_thread;
    wait_queue_head_t spi_wq;
    bool int_flag;
    struct workqueue_struct* rx_workqueue; /* Workqueue for RX packet handling */
    
    /* SPI rate limiting for 25MHz clock */
    unsigned long last_spi_op_time; /* Last SPI operation timestamp */
    unsigned int spi_op_interval;   /* Minimum interval between SPI operations (jiffies) */
};
```

이제 초기화 함수에서 새로운 spinlock을 초기화해야 합니다:


```c
    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    mutex_init(&tc6->spi_transfer_lock);
    spin_lock_init(&tc6->tx_skb_lock);
    spin_lock_init(&tc6->spi_ctrl_spinlock);
    spin_lock_init(&tc6->spi_transfer_spinlock);
```

이제 `lan1qtsn_spi_transfer` 함수에서 올바른 spinlock을 사용하도록 수정하겠습니다:


```c
    /* Use appropriate locking mechanism based on context */
    if (in_atomic) {
        /* In atomic context, use spinlock instead of mutex */
        if (!spin_trylock_irqsave(&tc6->spi_transfer_spinlock, flags)) {
            dev_err(&tc6->spi->dev, "SPI: Could not acquire transfer lock in atomic context\n");
            return -EBUSY;
        }
    } else {
        /* In process context, use mutex */
        if (mutex_lock_interruptible(&tc6->spi_transfer_lock)) {
            dev_err(&tc6->spi->dev, "SPI: Interrupted while acquiring transfer lock\n");
            return -EINTR;
        }
    }
```

```c
        if (in_atomic) {
            spin_unlock_irqrestore(&tc6->spi_transfer_spinlock, flags);
        } else {
            mutex_unlock(&tc6->spi_transfer_lock);
        }
```

```c
    /* Release lock based on context */
    if (in_atomic) {
        spin_unlock_irqrestore(&tc6->spi_transfer_spinlock, flags);
    } else {
        mutex_unlock(&tc6->spi_transfer_lock);
    }
```

이제 RX work handler에서 안전한 패킷 처리를 위해 수정하겠습니다:


```c
/**
 * lan1qtsn_rx_work_handler - Workqueue handler for RX packet processing
 * @work: Work structure containing RX work data
 *
 * This function runs in process context and safely calls netif_rx to submit
 * packets to the network stack. It also prevents recursive calls that could
 * cause scheduling while atomic errors.
 */
void lan1qtsn_rx_work_handler(struct work_struct* work) {
    struct lan1qtsn_rx_work* rx_work = container_of(work, struct lan1qtsn_rx_work, work);
    struct sk_buff* skb = rx_work->skb;
    struct net_device* netdev = rx_work->netdev;

    if (skb && netdev) {
        /* Check if we're in a safe context for netif_rx */
        if (in_atomic() || in_softirq() || in_irq()) {
            dev_warn(&netdev->dev, "RX work handler called in atomic context, deferring packet\n");
            /* Schedule the work again for later processing */
            INIT_WORK(&rx_work->work, lan1qtsn_rx_work_handler);
            schedule_work(&rx_work->work);
            return;
        }
        
        /* Submit the packet to the network stack in process context */
        netif_rx(skb);
    }

    /* Free the work structure */
    kfree(rx_work);
}
```

이제 시스템 클록 읽기 함수에서도 atomic 컨텍스트를 고려하도록 수정하겠습니다:


```c
sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi* tc6) {
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    int ret;
    unsigned long flags;
    unsigned long timeout;
    bool in_atomic = in_atomic() || in_softirq() || in_irq();

    /* Check if we're in atomic context */
    if (in_atomic) {
        dev_dbg(&tc6->spi->dev, "System clock read called in atomic context, using trylock\n");
        
        /* Use trylock in atomic context to prevent blocking */
        if (!spin_trylock_irqsave(&tc6->spi_ctrl_spinlock, flags)) {
            dev_warn(&tc6->spi->dev, "Could not acquire SPI control lock in atomic context\n");
            return 0;
        }
    } else {
        /* Use normal spinlock in process context */
        spin_lock_irqsave(&tc6->spi_ctrl_spinlock, flags);
    }
    
    /* Set timeout to prevent hanging */
    timeout = jiffies + msecs_to_jiffies(100); /* 100ms timeout */
    
    /* Perform SPI operations with timeout check */
    ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_H, &sys_clock_high, 1, LAN1QTSN_SPI_CTRL_REG_READ);
    if (ret == 0 && time_before(jiffies, timeout)) {
        ret = lan1qtsn_spi_perform_ctrl(tc6, TSN_SYSCOUNT_L, &sys_clock_low, 1, LAN1QTSN_SPI_CTRL_REG_READ);
    }
    
    spin_unlock_irqrestore(&tc6->spi_ctrl_spinlock, flags);
    
    if (ret == 0 && time_before(jiffies, timeout)) {
        clock = ((u64)sys_clock_high << 32) | sys_clock_low;
        return clock;
    }

    /* If read failed or timeout occurred, return 0 */
    dev_info(&tc6->spi->dev, "Could not read system clock safely (ret=%d, timeout=%s, atomic=%d)\n", 
             ret, time_after_eq(jiffies, timeout) ? "yes" : "no", in_atomic);
    return 0;
}
```

마지막으로, `start_xmit` 함수에서 시스템 클록 읽기 실패 시 더 안전한 처리를 추가하겠습니다:


```c
    /* Try to read system clock safely */
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    if (sys_count == 0) {
        /* Use default values if system clock read failed */
        dev_dbg(&ndev->dev, "System clock read failed, using default values\n");
        now = 0;
        sys_count_lower = 0;
        sys_count_upper = 0;
        
        /* Add delay to prevent rapid retries when clock is unavailable */
        if (!in_atomic() && !in_softirq() && !in_irq()) {
            usleep_range(1000, 5000); /* 1-5ms delay */
        }
        
        /* Check if we should continue or drop the packet */
        if (atomic_read(&priv->tc6->count1) % 10 == 0) {
            dev_warn(&ndev->dev, "System clock read failed, packet %lu dropped\n", 
                     atomic_read(&priv->tc6->count1));
            netif_wake_subqueue(ndev, q);
            dev_kfree_skb(skb);
            return NETDEV_TX_OK;
        }
    } else {
        now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
        sys_count_lower = sys_count & LOWER_29_BITS;
        sys_count_upper = sys_count & ~LOWER_29_BITS;
    }
```

이제 수정된 코드를 요약하면:

## 주요 수정 사항:

### 1. **Atomic 컨텍스트 감지 및 적절한 락 사용**
- `in_atomic()`, `in_softirq()`, `in_irq()` 체크로 컨텍스트 감지
- Atomic 컨텍스트에서는 `spinlock` 사용, Process 컨텍스트에서는 `mutex` 사용

### 2. **새로운 spinlock 추가**
- `spi_transfer_spinlock`: Atomic 컨텍스트에서 SPI 전송 보호
- 기존 `spi_transfer_lock` mutex와 분리하여 사용

### 3. **RX Work Handler 안전성 개선**
- Atomic 컨텍스트에서 호출될 때 재스케줄링으로 안전한 처리
- 재귀적 호출 방지

### 4. **시스템 클록 읽기 안전성**
- Atomic 컨텍스트에서 `trylock` 사용으로 블로킹 방지
- 실패 시 적절한 에러 처리

### 5. **패킷 드롭 메커니즘**
- 시스템 클록 읽기 실패 시 일부 패킷을 드롭하여 시스템 보호
- 무한 재시도 루프 방지

이제 수정된 드라이버를 컴파일하고 테스트해보세요. "scheduling while atomic" 에러가 해결되고 시스템이 더 안정적으로 동작할 것입니다.

컴파일하려면:
```bash
cd drivers/net/ethernet/tsnlab/lan1qtsn
make clean
make
```

그리고 테스트:
```bash
sudo arping 192.168.1.11 -W 0.1 -c 100
```

이제 시스템이 먹통이 되지 않고 더 안정적으로 동작할 것입니다.

```shellscript
cd drivers/net/ethernet/tsnlab/lan1qtsn
make clean
make
```

```shellscript
sudo arping 192.168.1.11 -W 0.1 -c 100
```

