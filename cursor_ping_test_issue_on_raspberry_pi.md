# Ping test issue on raspberry pi 4
_Exported on 9/2/2025 at 14:09:24 GMT+9 from Cursor (1.2.4)_

---

**User**

driver/linux/ 디렉토리에 있는 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있다.

두 개의 라즈베리 파이 4에 해당 커널로 부팅 시킨 후,
해당 드라이버에 대응하는 이더넷 포트를 서로 연결 시킨 후,
양쪽에서 ping test(ping 192.168.1.12 -i 0.002 -s 1472, ping 192.168.1.11 -i 0.002 -s 1472)를 진행한다.
ping test 진행 중 하나의 라즈베리 파이 4에서 ping이 멈추고 아무 반응을 하지 않는 경우가 발생한다.
이 때 다른 라즈베리 파이 4의 ping 시험은 전상적으로 진행되고 있다.
ping 시험이 멈춘 라즈베리 파이 4의 터미널에서  Ctrl + C를 눌러 시험하던 것을 멈추고 동일한 명령을 다시 실행하면 ping 시험이 다시 시작한다.

무슨 이유에서 ping이 멈췄는지 가능성 있는 이유를 설명해 주고 문제를 해결해 줘

---

**Cursor**

이 문제는 네트워크 드라이버나 커널에서 발생하는 것으로 보입니다. 먼저 현재 상황을 파악하기 위해 코드를 살펴보겠습니다.


이제 주요 드라이버 파일들을 살펴보겠습니다. ping이 멈추는 문제는 주로 네트워크 인터럽트 처리나 패킷 수신 처리에서 발생할 수 있습니다.


이제 인터럽트 처리 부분을 확인해보겠습니다.


이제 SPI 스레드 핸들러를 확인해보겠습니다.


이제 `lan1qtsn_spi_try_spi_transfer` 함수를 확인해보겠습니다.


이제 RX 스레드 핸들러도 확인해보겠습니다.


이제 문제의 원인을 파악했습니다. 코드를 분석한 결과, ping이 멈추는 주요 원인들과 해결 방법을 제시하겠습니다.

## 문제 분석

ping이 멈추는 주요 원인들은 다음과 같습니다:

### 1. **SPI 스레드 블로킹 문제**
- `lan1qtsn_spi_thread_handler`에서 무한 루프가 발생할 수 있음
- `lan1qtsn_spi_try_spi_transfer` 함수에서 에러 발생 시 적절한 복구가 되지 않음

### 2. **인터럽트 처리 문제**
- MAC-PHY 인터럽트가 제대로 처리되지 않아 스레드가 깨어나지 않음
- `int_flag`가 false로 설정된 후 적절히 복구되지 않음

### 3. **RX 패킷 처리 블로킹**
- RX 스레드에서 패킷 처리 중 블로킹이 발생할 수 있음
- 큐가 비어있을 때 적절한 대기 처리가 되지 않음

### 4. **SPI 전송 실패 시 복구 부족**
- SPI 전송 실패 시 재시도 로직이 부족함
- 에러 상황에서 스레드가 블로킹 상태로 남아있음

## 해결 방법

다음과 같은 수정을 통해 문제를 해결할 수 있습니다:


```c
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;
    unsigned long timeout_jiffies;
    bool should_retry = false;

    /* Validate input parameter */
    if (!tc6) {
        return -EINVAL;
    }

    while (likely(!kthread_should_stop())) {
        /* This kthread will be waken up if there is a tx skb or mac-phy
         * interrupt to perform spi transfer with tx chunks.
         */
        timeout_jiffies = msecs_to_jiffies(1000); /* 1 second timeout */
        
        ret = wait_event_interruptible_timeout(tc6->spi_wq, 
            tc6->int_flag || (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) ||
            kthread_should_stop(), timeout_jiffies);

        if (kthread_should_stop()) {
            break;
        }

        /* Check for timeout */
        if (ret == 0) {
            dev_warn(&tc6->spi->dev, "SPI thread timeout, checking for stuck conditions\n");
            
            /* Force wake up if stuck */
            if (tc6->waiting_tx_skb && !tc6->int_flag) {
                dev_info(&tc6->spi->dev, "Forcing SPI transfer for stuck TX packet\n");
                tc6->int_flag = true;
            }
            
            /* Reset interrupt flag if stuck */
            if (tc6->int_flag && !tc6->waiting_tx_skb) {
                tc6->int_flag = false;
            }
            
            should_retry = true;
        }

        ret = lan1qtsn_spi_try_spi_transfer(tc6);
        if (ret) {
            dev_err(&tc6->spi->dev, "SPI transfer failed: %d, attempting recovery\n", ret);
            
            /* Implement recovery logic */
            if (tc6->waiting_tx_skb) {
                dev_info(&tc6->spi->dev, "Recovering from TX failure\n");
                spin_lock_bh(&tc6->tx_skb_lock);
                if (tc6->waiting_tx_skb) {
                    dev_kfree_skb_any(tc6->waiting_tx_skb);
                    tc6->waiting_tx_skb = NULL;
                }
                spin_unlock_bh(&tc6->tx_skb_lock);
                
                /* Wake up the queue */
                if (netif_queue_stopped(tc6->netdev)) {
                    netif_wake_queue(tc6->netdev);
                }
            }
            
            /* Don't return immediately, continue processing */
            if (!should_retry) {
                usleep_range(1000, 2000); /* 1-2ms delay for recovery */
            }
            should_retry = false;
        } else {
            should_retry = false;
        }
    }

    return 0;
}
```

이제 `lan1qtsn_spi_try_spi_transfer` 함수에도 에러 복구 로직을 개선하겠습니다:


```c
static int lan1qtsn_spi_try_spi_transfer(struct lan1qtsn_spi* tc6) {
    int ret;
    struct rx_metadata meta;
    int retry_count = 0;
    const int max_retries = 3;

    /* Validate input parameter */
    if (!tc6) {
        return -EINVAL;
    }

    while (true) {
        u16 spi_len = 0;

        if (tc6->int_flag) {
            tc6->int_flag = false;
        }

        /* Transmit */
        if (tc6->waiting_tx_skb) {
            dev_dbg(&tc6->spi->dev, "TX: Processing waiting TX SKB, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

            spi_len = lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(tc6);

            if (spi_len > 0) {
                dev_dbg(&tc6->spi->dev, "TX: Starting SPI transfer, len=%d, tx_packets=%lu\n", spi_len,
                        tc6->netdev->stats.tx_packets);
#if 1
                char* t_b = tc6->spi_data_tx_buf;
                lan1qtsn_debug("[Command]\n");
                lan1qtsn_debug("%02x %02x %02x\n", t_b[0], t_b[1], t_b[2]);
                dump_buffer((unsigned char*)&t_b[3], spi_len - 3);
#endif
                
                /* Retry logic for SPI transfer */
                do {
                    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, spi_len);
                    if (ret && retry_count < max_retries) {
                        dev_warn(&tc6->spi->dev, "TX: SPI transfer failed, retry %d/%d: %d\n", 
                                retry_count + 1, max_retries, ret);
                        usleep_range(1000, 2000); /* 1-2ms delay before retry */
                        retry_count++;
                    } else {
                        break;
                    }
                } while (retry_count < max_retries);

                if (ret) {
                    dev_err(&tc6->spi->dev, "TX: SPI transfer failed after %d retries: %d, tx_packets=%lu\n", 
                            retry_count, ret, tc6->netdev->stats.tx_packets);
                    /* Clean up on error */
                    if (tc6->ongoing_tx_skb) {
                        kfree_skb(tc6->ongoing_tx_skb);
                        tc6->ongoing_tx_skb = NULL;
                    }
                    /* Wake up the queue even on error to allow retry */
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }
                    /* Reset retry count for next operation */
                    retry_count = 0;
                } else {
                    dev_dbg(&tc6->spi->dev, "TX: SPI transfer completed successfully, tx_packets=%lu\n",
                            tc6->netdev->stats.tx_packets);
                    /* Update stats and free skb after successful transfer */
                    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len + ETH_FCS_LEN;
                    tc6->netdev->stats.tx_packets++;
                    dev_dbg(&tc6->spi->dev, "TX: Stats updated - packets: %lu, bytes: %lu\n",
                            tc6->netdev->stats.tx_packets, tc6->netdev->stats.tx_bytes);
                    kfree_skb(tc6->ongoing_tx_skb);
                    tc6->ongoing_tx_skb = NULL;

                    /* Wake up the queue after successful transmission */
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }

                    /* Reset retry count for next operation */
                    retry_count = 0;

                    /* increment count2 */
#if 0
                    atomic_inc(&tc6->count2);
                    dev_dbg(
                        &tc6->spi->dev,
                        "TX: Sleeping for RX processing (usleep_range(5, 15)), tx_packets=%lu, count1=%d, count2=%d\n",
                        tc6->netdev->stats.tx_packets, atomic_read(&tc6->count1), atomic_read(&tc6->count2));
#endif
                    /* Small delay to allow RX processing in bidirectional traffic */
                    /* usleep_range(5, 15); */
                }
            } else {
                dev_dbg(&tc6->spi->dev, "TX: No SPI data to transfer (spi_len=0), tx_packets=%lu\n",
                        tc6->netdev->stats.tx_packets);
            }
        } else {
            dev_dbg(&tc6->spi->dev, "TX: No waiting TX SKB, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        }

        /* Add delay between TX and RX to prevent SPI bus overload */
        if (tc6->waiting_tx_skb) {
            /* usleep_range(20, 40); */
        }

        /* Receive metadata with timeout protection */
        ret = lan1qtsn_spi_get_meta_data(tc6, &meta);
        if (ret) {
            dev_err(&tc6->spi->dev, "RX: Failed to get metadata: %d\n", ret);
            if (tc6->waiting_tx_skb) {
                /* Add small delay to prevent tight loop */
                usleep_range(100, 200);
                continue;
            }
            break;
        }

        if (meta.frame_length == 0) {
            dev_dbg(&tc6->spi->dev, "RX: No frame to receive (meta.frame_length = 0)\n");
            if (tc6->waiting_tx_skb) {
                /* Add small delay to prevent tight loop */
                usleep_range(100, 200);
                continue;
            }
            break;
        }

        ret = lan1qtsn_spi_process_spi_data_rx_buf(tc6, meta);
        if (ret) {
            lan1qtsn_spi_cleanup_ongoing_rx_skb(tc6);
            dev_err(&tc6->spi->dev, "RX: Failed to process RX buffer: %d\n", ret);
            /* Don't return immediately, try to continue */
            if (tc6->waiting_tx_skb) {
                usleep_range(100, 200);
                continue;
            }
            break;
        }

        if (!tc6->waiting_tx_skb && netif_queue_stopped(tc6->netdev)) {
            netif_wake_queue(tc6->netdev);
        }

        /* Add small delay to prevent CPU overload */
        /* usleep_range(10, 20); */
    }

    return 0;
}
```

이제 RX 스레드 핸들러에도 타임아웃과 에러 복구 로직을 추가하겠습니다:


```c
int lan1qtsn_rx_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = (struct lan1qtsn_spi*)data;
    struct lan1qtsn_rx_packet* rx_pkt;
    unsigned long flags;
    unsigned long timeout_jiffies;
    int ret;

    while (!kthread_should_stop() && !tc6->rx_thread_should_stop) {
        /* Wait for packets or stop signal with timeout */
        timeout_jiffies = msecs_to_jiffies(5000); /* 5 second timeout */
        
        ret = wait_event_interruptible_timeout(tc6->rx_wq, 
            !list_empty(&tc6->rx_packet_queue) || kthread_should_stop() ||
            tc6->rx_thread_should_stop, timeout_jiffies);

        if (kthread_should_stop() || tc6->rx_thread_should_stop)
            break;

        /* Check for timeout */
        if (ret == 0) {
            dev_dbg(&tc6->spi->dev, "RX thread timeout, checking for stuck conditions\n");
            continue;
        }

        /* Process all queued packets */
        while (true) {
            spin_lock_irqsave(&tc6->rx_queue_lock, flags);
            if (list_empty(&tc6->rx_packet_queue)) {
                spin_unlock_irqrestore(&tc6->rx_queue_lock, flags);
                break;
            }

            /* Get the first packet from queue */
            rx_pkt = list_first_entry(&tc6->rx_packet_queue, struct lan1qtsn_rx_packet, list);
            list_del(&rx_pkt->list);
            spin_unlock_irqrestore(&tc6->rx_queue_lock, flags);

            /* Process the packet safely in process context */
            if (rx_pkt->skb && rx_pkt->netdev) {
                /* Add error handling for netif_rx */
                if (netif_rx(rx_pkt->skb) == NET_RX_DROP) {
                    dev_warn(&tc6->spi->dev, "RX packet dropped by network stack\n");
                }
            } else {
                dev_warn(&tc6->spi->dev, "Invalid RX packet structure\n");
                if (rx_pkt->skb) {
                    dev_kfree_skb_any(rx_pkt->skb);
                }
            }

            /* Free the packet structure */
            kfree(rx_pkt);
            
            /* Add small delay to prevent CPU overload */
            usleep_range(10, 20);
        }
    }

    return 0;
}
```

이제 인터럽트 핸들러에도 개선사항을 추가하겠습니다:


```c
static irqreturn_t lan1qtsn_spi_macphy_isr(int irq, void* data) {
    struct lan1qtsn_spi* tc6 = data;
    unsigned long flags;

    /* Prevent re-entrancy */
    if (irq == tc6->spi->irq) {
        spin_lock_irqsave(&tc6->irq_lock, flags);
        
        /* Check if interrupt is already being processed */
        if (tc6->int_flag) {
            dev_dbg(&tc6->spi->dev, "Interrupt already being processed, skipping\n");
            spin_unlock_irqrestore(&tc6->irq_lock, flags);
            return IRQ_HANDLED;
        }
        
        /* MAC-PHY interrupt can occur for the following reasons.
         * - availability of tx credits if it was 0 before and not reported in
         *   the previous rx footer.
         * - availability of rx chunks if it was 0 before and not reported in
         *   the previous rx footer.
         * - extended status event not reported in the previous rx footer.
         */
        tc6->int_flag = true;
        tc6->last_irq_time = jiffies;
        
        spin_unlock_irqrestore(&tc6->irq_lock, flags);
        
        /* Wake spi kthread to perform spi transfer */
        wake_up_interruptible(&tc6->spi_wq);
    }

    return IRQ_HANDLED;
}
```

이제 헤더 파일에 필요한 필드들을 추가해야 합니다:


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
    spinlock_t irq_lock;            /* Protects interrupt handling */
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
    unsigned long last_irq_time;    /* Last interrupt timestamp */
    
    /* RX packet processing thread */
    struct task_struct* rx_thread;
    wait_queue_head_t rx_wq;
    spinlock_t rx_queue_lock;
    struct list_head rx_packet_queue;
    bool rx_thread_should_stop;
    

    
    /* SPI rate limiting for 25MHz clock */
    unsigned long last_spi_op_time; /* Last SPI operation timestamp */
    unsigned int spi_op_interval;   /* Minimum interval between SPI operations (jiffies) */
};
```

이제 초기화 함수에서 새로 추가된 필드들을 초기화해야 합니다:


```c
    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    mutex_init(&tc6->spi_transfer_lock);
    spin_lock_init(&tc6->tx_skb_lock);
    spin_lock_init(&tc6->irq_lock);

    /* Initialize RX packet processing thread structures */
    spin_lock_init(&tc6->rx_queue_lock);
    INIT_LIST_HEAD(&tc6->rx_packet_queue);
    init_waitqueue_head(&tc6->rx_wq);
    tc6->rx_thread_should_stop = false;

    /* initialize debugging counters */
    atomic_set(&tc6->wrong_mac_count, 0);
    atomic_set(&tc6->count1, 0);
    atomic_set(&tc6->count2, 0);

    /* Initialize interrupt handling */
    tc6->int_flag = false;
    tc6->last_irq_time = 0;

    /* Initialize SPI rate limiting for 25MHz clock */
    tc6->last_spi_op_time = 0;
    tc6->spi_op_interval = 0; /* 0 jiffy = immediate, but with microsecond delays for 25MHz SPI clock */
```

마지막으로 네트워크 드라이버에 watchdog 타이머를 추가하여 ping이 멈춘 상황을 감지하고 복구할 수 있도록 하겠습니다:


이제 watchdog 타이머를 추가하여 ping이 멈춘 상황을 감지하고 복구할 수 있도록 하겠습니다:


```cpp
#pragma once

#include <linux/etherdevice.h>
#include <linux/mutex.h>
#include <linux/net_tstamp.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

#include "lan1qtsn_mod.h"

#define TX_TSTAMP_MAX_RETRY 5
#define WATCHDOG_TIMEOUT_MS 5000  /* 5 seconds */

struct tick_count {
    uint32_t tick : 29;
    uint32_t priority : 3;
} __attribute__((packed, scalar_storage_order("big-endian")));

struct tx_metadata {
    struct tick_count from;
    struct tick_count to;
    struct tick_count delay_from;
    struct tick_count delay_to;
    uint16_t frame_length;
    uint16_t timestamp_id;
    uint8_t fail_policy;
    uint8_t reserved0[3];
    uint32_t reserved1;
    uint32_t reserved2;
} __attribute__((packed, scalar_storage_order("big-endian")));

struct tx_buffer {
    struct tx_metadata metadata;
    uint8_t data[0];
} __attribute__((packed, scalar_storage_order("big-endian")));

struct rx_metadata {
    uint64_t timestamp;
    uint16_t frame_length;
} __attribute__((packed, scalar_storage_order("big-endian")));

#define TX_METADATA_SIZE (sizeof(struct tx_metadata))

#define _DEFAULT_FROM_MARGIN_ (500)
#define _DEFAULT_TO_MARGIN_ (100000000) /* 50000 -> 1000000 */

int lan1qtsn_netdev_open(struct net_device* ndev);
int lan1qtsn_netdev_close(struct net_device* ndev);

/*
 * lan1qtsn_netdev_setup_tc - TC config handler
 * @dev: Pointer to the network device
 * @type: Tc setup type
 * @type_data: parameters passed to the tc command
 */
int lan1qtsn_netdev_setup_tc(struct net_device* ndev, enum tc_setup_type type, void* type_data);

int lan1qtsn_netdev_ioctl(struct net_device* ndev, struct ifreq* ifr, int cmd);
u16 lan1qtsn_select_queue(struct net_device* ndev, struct sk_buff* skb, struct net_device* sb_dev);

void lan1qtsn_tx_work1(struct work_struct* work);
void lan1qtsn_tx_work2(struct work_struct* work);
void lan1qtsn_tx_work3(struct work_struct* work);
void lan1qtsn_tx_work4(struct work_struct* work);

/* Watchdog timer functions */
void lan1qtsn_watchdog_timer_callback(struct timer_list* t);
void lan1qtsn_start_watchdog(struct net_device* ndev);
void lan1qtsn_stop_watchdog(struct net_device* ndev);
```

이제 watchdog 타이머 구현을 추가하겠습니다:


```c
DEFINE_TX_WORK(1);
DEFINE_TX_WORK(2);
DEFINE_TX_WORK(3);
DEFINE_TX_WORK(4);

/**
 * lan1qtsn_watchdog_timer_callback - Watchdog timer callback function
 * @t: Timer list structure
 *
 * This function is called periodically to check if the network interface
 * is functioning properly and recover from stuck conditions.
 */
void lan1qtsn_watchdog_timer_callback(struct timer_list* t) {
    struct lan1qtsn_priv* priv = from_timer(priv, t, watchdog_timer);
    struct net_device* ndev = priv->ndev;
    struct lan1qtsn_spi* tc6 = priv->tc6;
    unsigned long flags;
    
    if (!tc6 || !ndev) {
        return;
    }
    
    dev_dbg(&ndev->dev, "Watchdog timer triggered, checking interface status\n");
    
    /* Check if interface is stuck */
    if (tc6->waiting_tx_skb && !tc6->int_flag) {
        dev_warn(&ndev->dev, "Watchdog: TX packet stuck, forcing recovery\n");
        
        /* Force interrupt flag to wake up SPI thread */
        spin_lock_irqsave(&tc6->irq_lock, flags);
        tc6->int_flag = true;
        spin_unlock_irqrestore(&tc6->irq_lock, flags);
        
        /* Wake up SPI thread */
        wake_up_interruptible(&tc6->spi_wq);
    }
    
    /* Check if RX thread is stuck */
    if (tc6->rx_thread && !kthread_should_stop()) {
        /* Wake up RX thread */
        wake_up_interruptible(&tc6->rx_wq);
    }
    
    /* Restart watchdog timer */
    lan1qtsn_start_watchdog(ndev);
}

/**
 * lan1qtsn_start_watchdog - Start watchdog timer
 * @ndev: Network device
 *
 * This function starts the watchdog timer to monitor interface health.
 */
void lan1qtsn_start_watchdog(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    
    if (!priv) {
        return;
    }
    
    /* Initialize watchdog timer if not already done */
    if (!timer_pending(&priv->watchdog_timer)) {
        timer_setup(&priv->watchdog_timer, lan1qtsn_watchdog_timer_callback, 0);
        mod_timer(&priv->watchdog_timer, jiffies + msecs_to_jiffies(WATCHDOG_TIMEOUT_MS));
        dev_dbg(&ndev->dev, "Watchdog timer started\n");
    }
}

/**
 * lan1qtsn_stop_watchdog - Stop watchdog timer
 * @ndev: Network device
 *
 * This function stops the watchdog timer.
 */
void lan1qtsn_stop_watchdog(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    
    if (!priv) {
        return;
    }
    
    if (timer_pending(&priv->watchdog_timer)) {
        del_timer_sync(&priv->watchdog_timer);
        dev_dbg(&ndev->dev, "Watchdog timer stopped\n");
    }
}
```

이제 `lan1qtsn_netdev_open`과 `lan1qtsn_netdev_close` 함수에서 watchdog 타이머를 시작하고 중지하도록 수정하겠습니다:


```c
int lan1qtsn_netdev_open(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);

    netif_carrier_on(ndev);
#if 0 // def __TSN_PTP__
    netif_start_queue(ndev);
    for (int i = 0; i < TX_QUEUE_COUNT; i++) {
        netif_start_subqueue(ndev, i);
    }
#endif

    /* Enable TSN system first */
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);

    u32 sys_info_high;
    lan1qtsn_spi_read_register(priv->tc6, TSN_SYSTEM_INFO_H, &sys_info_high);
    dev_info(&ndev->dev, "Network interface opened(0x%08x)\n", sys_info_high);

#ifdef __TSN_PTP__
    tsn_init_configs(priv->ldev);
#endif

    /* Start watchdog timer to monitor interface health */
    lan1qtsn_start_watchdog(ndev);

    return 0;
}
```

```c
int lan1qtsn_netdev_close(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);

    /* Stop watchdog timer first */
    lan1qtsn_stop_watchdog(ndev);

    /* Disable TSN system first */
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);

#ifdef __TSN_PTP__
    netif_stop_queue(ndev);
    for (int i = 0; i < TX_QUEUE_COUNT; i++) {
        netif_stop_subqueue(ndev, i);
    }
#endif
    dev_info(&ndev->dev, "%s\n", __func__);
    netif_carrier_off(ndev);
    dev_info(&ndev->dev, "netif_carrier_off\n");
    return 0;
}
```

이제 `lan1qtsn_mod.h` 파일에 watchdog 타이머 필드를 추가해야 합니다:


```cpp
#pragma once

#include <linux/cpu.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/random.h>
#include <linux/utsname.h>
#include <linux/timer.h>

#include "lan1qtsn_ptp.h"
#include "liblan1qtsn.h"

#define DRV_NAME "lan1qtsn"

#define LAN1QTSN_BUFFER_SIZE (1564)

#define TX_QUEUE_COUNT 8
#define RX_QUEUE_COUNT 8

struct lan1qtsn_priv {
    uint64_t magic;
    struct spi_device* sdev;
    struct lan1qtsn_spi_dev* tsdev;
    struct net_device* ndev;
    struct lan1qtsn_dev* ldev;

    struct lan1qtsn_spi* tc6;

    struct work_struct tx_work[TSN_TIMESTAMP_ID_MAX];
    struct sk_buff* tx_work_skb[TSN_TIMESTAMP_ID_MAX];
    sysclock_t tx_work_start_after[TSN_TIMESTAMP_ID_MAX];
    sysclock_t tx_work_wait_until[TSN_TIMESTAMP_ID_MAX];
    struct hwtstamp_config tstamp_config;
    sysclock_t last_tx_tstamp[TSN_TIMESTAMP_ID_MAX];
    int tstamp_retry[TSN_TIMESTAMP_ID_MAX];

    struct sk_buff* rx_skb;
    struct sk_buff* tx_skb;

    uint64_t total_tx_count;
    uint64_t total_tx_drop_count;
    uint64_t last_normal_timeout;
    uint64_t last_to_overflow_popped;
    uint64_t last_to_overflow_timeout;

    unsigned long state;
    
    /* Watchdog timer for interface health monitoring */
    struct timer_list watchdog_timer;
};
```

마지막으로 모듈 초기화 시 watchdog 타이머를 초기화하도록 수정해야 합니다. 모듈 초기화 파일을 찾아보겠습니다:


```c
    priv->tc6->tsdev = tsdev;
    ldev->tc6 = priv->tc6;

    /* Initialize watchdog timer */
    timer_setup(&priv->watchdog_timer, NULL, 0);

    /* Set the MAC address */
    lan1qtsn_generate_mac_from_system_info(ndev);
```

이제 수정사항을 요약하고 컴파일을 위한 Makefile을 확인해보겠습니다:


## 수정 사항 요약

ping이 멈추는 문제를 해결하기 위해 다음과 같은 수정을 수행했습니다:

### 1. **SPI 스레드 블로킹 방지**
- `lan1qtsn_spi_thread_handler`에 1초 타임아웃 추가
- 무한 블로킹 상황 감지 및 자동 복구 로직 구현
- 에러 발생 시 재시도 메커니즘 추가

### 2. **SPI 전송 에러 복구 강화**
- `lan1qtsn_spi_try_spi_transfer`에 최대 3회 재시도 로직 추가
- 에러 상황에서 적절한 리소스 정리 및 큐 복구
- tight loop 방지를 위한 지연 시간 추가

### 3. **RX 스레드 안정성 향상**
- `lan1qtsn_rx_thread_handler`에 5초 타임아웃 추가
- 패킷 처리 중 에러 발생 시 적절한 복구
- CPU 과부하 방지를 위한 지연 시간 추가

### 4. **인터럽트 처리 개선**
- `lan1qtsn_spi_macphy_isr`에 중복 인터럽트 방지 로직 추가
- 인터럽트 락을 통한 race condition 방지
- 인터럽트 타임스탬프 추적

### 5. **Watchdog 타이머 추가**
- 5초마다 인터페이스 상태를 확인하는 watchdog 타이머 구현
- stuck 상황 감지 시 자동 복구 시도
- SPI 스레드와 RX 스레드 강제 wake-up

### 6. **구조체 필드 추가**
- `irq_lock`: 인터럽트 처리 동기화
- `last_irq_time`: 마지막 인터럽트 시간 추적
- `watchdog_timer`: 인터페이스 상태 모니터링

## 컴파일 및 테스트

이제 수정된 드라이버를 컴파일하고 테스트할 수 있습니다:

```bash
cd driver/linux/drivers/net/ethernet/tsnlab/lan1qtsn
make clean
make
```

컴파일이 성공하면 새로운 커널 이미지를 빌드하고 라즈베리 파이 4에 설치한 후 ping 테스트를 다시 수행해보세요.

## 예상 효과

이러한 수정을 통해 다음과 같은 효과를 기대할 수 있습니다:

1. **ping 멈춤 현상 방지**: 타임아웃과 에러 복구 로직으로 무한 블로킹 방지
2. **자동 복구**: watchdog 타이머가 stuck 상황을 감지하고 자동으로 복구 시도
3. **안정성 향상**: 에러 상황에서도 드라이버가 계속 동작하며 복구 시도
4. **디버깅 개선**: 상세한 로그 메시지로 문제 상황 파악 용이

ping 테스트 중 문제가 발생하면 `dmesg` 명령어로 커널 로그를 확인하여 어떤 복구 동작이 수행되었는지 확인할 수 있습니다.

```shellscript
cd driver/linux/drivers/net/ethernet/tsnlab/lan1qtsn
make clean
make
```

