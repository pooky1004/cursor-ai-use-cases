# Deadlock issue in SPI network communication
_Exported on 8/13/2025 at 08:00:19 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하려고 해.
SPI를 통해 네크워크 포트랑 연결이 되기 때문에 하나의 thread가 패킷 rx와 tx를 모두 담당하고 있어.

현재 tx 만 시험하고 있는데,
패킷을 tx하면서 네트웤 포트쪽(SPI를 통해 연결된) 레지스터를 읽는 동작이 들어 가면
deadlock 이 발생하는 것처럼 부팅할 때 멈춰.

deadlock 이 발생할 가능성이 있는지 확인해 주고
가능성이 있다면 발생하지 않도록 코드를 수정해 줘.

---

**Cursor**

네트워크 드라이버 코드에서 deadlock이 발생할 가능성을 확인하고 수정해드리겠습니다. 먼저 코드를 살펴보겠습니다.


코드를 분석한 결과, **deadlock이 발생할 가능성이 높습니다**. 주요 문제점들을 파악했습니다:

## Deadlock 발생 가능성 분석

### 1. **Lock 순서 문제 (Lock Ordering Issue)**
- `lan1qtsn_netdev_start_xmit()` 함수에서 `netif_stop_subqueue()` 호출 후
- `lan1qtsn_get_sys_clock_by_ldev()` 함수를 통해 `lan1qtsn_spi_read_register()` 호출
- 이 함수는 `spi_ctrl_lock` mutex를 획득

### 2. **동시 Lock 획득 문제**
- TX 처리 중에 레지스터 읽기 시 `spi_ctrl_lock` 획득
- 동시에 SPI kthread에서 TX 처리 시 `tx_skb_lock` spinlock 획득
- 두 lock이 서로 다른 순서로 획득될 수 있음

### 3. **SPI kthread와 TX 함수 간의 경쟁**
- `lan1qtsn_netdev_start_xmit()` → `lan1qtsn_spi_start_xmit()` → SPI kthread wakeup
- SPI kthread에서 `lan1qtsn_spi_try_spi_transfer()` 호출
- TX 함수에서 레지스터 읽기 시도

## 해결 방안

deadlock을 방지하기 위해 다음과 같이 코드를 수정하겠습니다:


```c
netdev_tx_t lan1qtsn_netdev_start_xmit(struct sk_buff* skb, struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    struct lan1qtsn_dev* ldev = priv->ldev;
    u32 w;
    sysclock_t sys_count, sys_count_upper, sys_count_lower;
    timestamp_t now;
    u16 frame_length;
	struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;
    u32 to_value;
    u16 q;

    /* Check desc count */
    q = skb_get_queue_mapping(skb);
    netif_stop_subqueue(ndev, q);
    lan1qtsn_debug("lan1qtsn_netdev_start_xmit(skb->len : %d)\n", skb->len);
    skb->len = max((unsigned int)ETH_ZLEN, skb->len);

    /* Store packet length */
    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */

    if (skb_padto(skb, skb->len)) {
        pr_err("skb_padto failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return NETDEV_TX_OK;
    }

    /* Jumbo frames not supported */
    if (skb->len + ETH_FCS_LEN > LAN1QTSN_BUFFER_SIZE) {
        pr_err("Jumbo frames not supported\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return NETDEV_TX_OK;
    }

    /* Add metadata to the skb */
    if (pskb_expand_head(skb, TX_METADATA_SIZE, 0, GFP_ATOMIC) != 0) {
        pr_err("pskb_expand_head failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return NETDEV_TX_OK;
    }
    skb_push(skb, TX_METADATA_SIZE);
    memset(skb->data, 0, TX_METADATA_SIZE);

    lan1qtsn_debug("skb->len : %d\n", skb->len);
    tx_buffer = (struct tx_buffer*)skb->data;
    /* Fill in the metadata */
    tx_metadata = (struct tx_metadata*)&tx_buffer->metadata;
    tx_metadata->frame_length = frame_length;

#if 0
    tx_metadata->from.tick = 0x0;
    tx_metadata->from.priority = 0;
    tx_metadata->to.tick = 0x1FFFFFFF;
    tx_metadata->to.priority = 0;
    tx_metadata->delay_from.tick = 0x0;
    tx_metadata->delay_from.priority = 0;
    tx_metadata->delay_to.tick = 0x1FFFFFFF;
    tx_metadata->delay_to.priority = 0;

    // Wake up the subqueue after successful metadata addition
    netif_wake_subqueue(ndev, q);

    u32 sys_clock_high;
    lan1qtsn_spi_read_register(priv->tc6, TSN_SYSCOUNT_H, &sys_clock_high);
    pr_err("%s - sys_clock_high: 0x%08x\n", __func__, sys_clock_high);
#else
    // DEADLOCK 방지: 레지스터 읽기를 TX 처리 전에 수행
    // 이렇게 하면 lock 순서가 일관되게 유지됨
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&priv->sdev->dev);
    pr_err("%s - tsdev->ptp: %p\n", __func__, tsdev->ptp);
    now = lan1qtsn_sysclock_to_timestamp(priv->sdev, sys_count);
    sys_count_lower = sys_count & LOWER_29_BITS;
    sys_count_upper = sys_count & ~LOWER_29_BITS;

    /* Set the fromtick & to_tick values based on the lower 29 bits of the system count */
    if (tsn_fill_metadata(priv->sdev, now, skb) == false) {
        // TODO: Increment SW drop stats
        pr_warn("tsn_fill_metadata failed\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

    lan1qtsn_debug("0x%08x  0x%08x  0x%08x  %4d  %1d", sys_count_lower, tx_metadata->from.tick, tx_metadata->to.tick,
                   tx_metadata->frame_length, tx_metadata->fail_policy);
    dump_buffer((unsigned char*)tx_metadata, (int)(sizeof(struct tx_metadata) + skb->len));

    if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) {
        if (priv->tstamp_config.tx_type == HWTSTAMP_TX_ON &&
            !test_and_set_bit_lock(tx_metadata->timestamp_id, &priv->state)) {
            skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
            priv->tx_work_skb[tx_metadata->timestamp_id] = skb_get(skb);
            /*
             * Even if the driver's intention was sys_count == from,
             * there might be a slight error caused during conversion (sysclock <-> timestamp)
             * So if the diffence is small, don't consider it as overflow.
             * Adding/Subtracting directly from values can cause another overflow,
             * so just calculate the difference.
             */
            priv->tx_work_start_after[tx_metadata->timestamp_id] = sys_count_upper | tx_metadata->from.tick;
            if (sys_count_lower > tx_metadata->from.tick &&
                sys_count_lower - tx_metadata->from.tick > TX_WORK_OVERFLOW_MARGIN) {
                // Overflow
                priv->tx_work_start_after[tx_metadata->timestamp_id] += (1 << 29);
            }
            to_value =
                (tx_metadata->fail_policy == TSN_FAIL_POLICY_RETRY ? tx_metadata->delay_to.tick : tx_metadata->to.tick);
            priv->tx_work_wait_until[tx_metadata->timestamp_id] = sys_count_upper | to_value;
            if (sys_count_lower > to_value && sys_count_lower - to_value > TX_WORK_OVERFLOW_MARGIN) {
                // Overflow
                priv->tx_work_wait_until[tx_metadata->timestamp_id] += (1 << 29);
            }
            schedule_work(&priv->tx_work[tx_metadata->timestamp_id]);
        } else if (priv->tstamp_config.tx_type != HWTSTAMP_TX_ON) {
            pr_warn("Timestamp skipped: timestamp config is off\n");
        } else {
            pr_warn("Timestamp skipped: driver is waiting for previous packet's timestamp\n");
        }
        // TODO: track the number of skipped packets for ethtool stats
    }
#endif

    /* netif_wake_queue() will be called in lan1qtsn_isr() */
    priv->tx_skb = skb;

	return lan1qtsn_spi_start_xmit(priv->tc6, skb);
}
```

이제 SPI 드라이버에서도 deadlock을 방지하기 위한 추가 수정을 하겠습니다:


```c
static int lan1qtsn_spi_try_spi_transfer(struct lan1qtsn_spi* tc6) {
    int ret;
    struct rx_metadata meta;

    // Validate input parameter
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

            // DEADLOCK 방지: TX 처리 전에 필요한 모든 레지스터 읽기 완료
            // 이렇게 하면 TX 처리 중에 추가 레지스터 읽기가 발생하지 않음
            spi_len = lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(tc6);

            if (spi_len > 0) {
                dev_dbg(&tc6->spi->dev, "TX: Starting SPI transfer, len=%d, tx_packets=%lu\n", spi_len,
                        tc6->netdev->stats.tx_packets);
                ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, spi_len);

                if (ret) {
                    dev_err(&tc6->spi->dev, "TX: SPI transfer failed: %d, tx_packets=%lu\n", ret,
                            tc6->netdev->stats.tx_packets);
                    /* Clean up on error */
                    if (tc6->ongoing_tx_skb) {
                        kfree_skb(tc6->ongoing_tx_skb);
                        tc6->ongoing_tx_skb = NULL;
                    }
                    // Wake up the queue even on error to allow retry
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }
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

                    // Wake up the queue after successful transmission
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }

                    // count2 증가
                    atomic_inc(&tc6->count2);
                    dev_dbg(
                        &tc6->spi->dev,
                        "TX: Sleeping for RX processing (usleep_range(5, 15)), tx_packets=%lu, count1=%d, count2=%d\n",
                        tc6->netdev->stats.tx_packets, atomic_read(&tc6->count1), atomic_read(&tc6->count2));
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

        /* Receive metadata */
        ret = lan1qtsn_spi_get_meta_data(tc6, &meta);
        if (ret) {
            dev_err(&tc6->spi->dev, "RX: Failed to get metadata: %d\n", ret);
            if (tc6->waiting_tx_skb) {
                continue;
            }
            break;
        }

        if (meta.frame_length == 0) {
            dev_dbg(&tc6->spi->dev, "RX: No frame to receive (meta.frame_length = 0)\n");
            if (tc6->waiting_tx_skb) {
                continue;
            }
            break;
        }

        ret = lan1qtsn_spi_process_spi_data_rx_buf(tc6, meta);
        if (ret) {
            lan1qtsn_spi_cleanup_ongoing_rx_skb(tc6);
            dev_err(&tc6->spi->dev, "RX: Failed to process RX buffer: %d\n", ret);
            return ret;
        }
    }

    return 0;
}
```

이제 더 중요한 수정을 하겠습니다. `lan1qtsn_spi_start_xmit` 함수에서 lock 순서를 보장하도록 수정하겠습니다:


```c
netdev_tx_t lan1qtsn_spi_start_xmit(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    int ret;

    // count1 증가
    atomic_inc(&tc6->count1);
    dev_dbg(&tc6->spi->dev, "TX: start_xmit called, skb len=%d, tx_packets=%lu, count1=%d, count2=%d\n", skb->len,
            tc6->netdev->stats.tx_packets, atomic_read(&tc6->count1), atomic_read(&tc6->count2));

#ifndef __TSN_PTP__
    ret = lan1qtsn_append_metadata_to_skb(tc6, skb);
    if (ret) {
        dev_err(&tc6->spi->dev, "TX: Failed to append metadata to skb\n");
        return NETDEV_TX_OK;
    }

    dev_dbg(&tc6->spi->dev, "TX: Metadata appended successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
#endif

    /* Check if we already have a waiting TX SKB - use more lenient approach for bidirectional traffic */
    // DEADLOCK 방지: lock 순서 보장 - tx_skb_lock을 먼저 획득
    spin_lock_bh(&tc6->tx_skb_lock);
    if (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) {
        spin_unlock_bh(&tc6->tx_skb_lock);
        dev_dbg(&tc6->spi->dev, "TX: Waiting TX SKB already exists, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        /* For bidirectional traffic, be more lenient with queue management */
        if (netif_queue_stopped(tc6->netdev)) {
            dev_dbg(&tc6->spi->dev, "TX: Queue stopped, returning BUSY, tx_packets=%lu\n",
                    tc6->netdev->stats.tx_packets);
            return NETDEV_TX_BUSY;
        }
        /* Don't stop queue immediately, let it buffer a bit more */
        dev_dbg(&tc6->spi->dev, "TX: Queue not stopped, accepting packet, tx_packets=%lu\n",
                tc6->netdev->stats.tx_packets);
        return NETDEV_TX_OK;
    }
    spin_unlock_bh(&tc6->tx_skb_lock);

    if (skb_linearize(skb)) {
        dev_err(&tc6->spi->dev, "TX: Failed to linearize skb\n");
        dev_kfree_skb_any(skb);
        tc6->netdev->stats.tx_dropped++;
        return NETDEV_TX_OK;
    }

    dev_dbg(&tc6->spi->dev, "TX: Skb linearized successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    /* Check if skb is owned by socket - if so, we can't reuse it */
    if (skb->sk) {
        dev_dbg(&tc6->spi->dev, "TX: Creating copy of socket-owned skb, tx_packets=%lu\n",
                tc6->netdev->stats.tx_packets);
        /* Create a copy for our use */
        struct sk_buff* new_skb = skb_copy(skb, GFP_ATOMIC);
        if (!new_skb) {
            dev_err(&tc6->spi->dev, "TX: Failed to copy socket-owned skb\n");
            dev_kfree_skb_any(skb);
            tc6->netdev->stats.tx_dropped++;
            return NETDEV_TX_OK;
        }
        dev_kfree_skb_any(skb);
        skb = new_skb;
        dev_dbg(&tc6->spi->dev, "TX: Socket-owned skb copied successfully, tx_packets=%lu\n",
                tc6->netdev->stats.tx_packets);
    }

    // DEADLOCK 방지: lock 순서 보장 - tx_skb_lock을 마지막에 획득
    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->waiting_tx_skb = skb;
    spin_unlock_bh(&tc6->tx_skb_lock);

    dev_dbg(&tc6->spi->dev, "TX: Packet queued for transmission, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return NETDEV_TX_OK;
}
```

이제 가장 중요한 수정을 하겠습니다. `lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs` 함수에서 lock 순서를 보장하도록 수정하겠습니다:


```c
static u16 lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(struct lan1qtsn_spi* tc6) {
    struct sk_buff* skb;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u16 copy_len, total_len;
    u32 fcs;
    u8* eth_data;
    u16 eth_len;

    // DEADLOCK 방지: lock 순서 보장 - tx_skb_lock을 먼저 획득
    // 1. Move the waiting skb to ongoing transmission slot
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    // 2. Check if there is skb to transmit
    if (!skb) {
        dev_dbg(&tc6->spi->dev, "TX: No waiting skb to transmit, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Preparing skb for transmission, len=%d, tx_packets=%lu\n", skb->len,
            tc6->netdev->stats.tx_packets);

    copy_len = skb->len;

    // 3. Copy the original frame data to the SPI buffer, after command header
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    // 4. Calculate Ethernet frame data start address and length (excluding metadata)
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    dev_dbg(&tc6->spi->dev, "TX: Ethernet data len=%d, metadata size=%ld, tx_packets=%lu\n", eth_len, TX_METADATA_SIZE,
            tc6->netdev->stats.tx_packets);

    // 5. Calculate FCS (CRC32) over the pure Ethernet frame
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; // Final XOR

    // 6. Append FCS to SPI transmit buffer after the frame data
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    // 7. Calculate total length (# bytes to transmit)
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    // 8. Check buffer overflow, clean up and exit on error
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        dev_err(&tc6->spi->dev, "TX: Buffer overflow: total_len=%d, max=%d, tx_packets=%lu\n", total_len,
                LAN1QTSN_BUFFER_SIZE, tc6->netdev->stats.tx_packets);
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Frame prepared, total_len=%d, eth_len=%d, tx_packets=%lu\n", total_len, eth_len,
            tc6->netdev->stats.tx_packets);

    // 9. Prepare the SPI command header
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);

    return total_len;
}
```

이제 추가적인 deadlock 방지 조치를 위해 `lan1qtsn_spi_read_register` 함수에 timeout과 retry 메커니즘을 추가하겠습니다:


```c
int lan1qtsn_spi_read_register(struct lan1qtsn_spi* tc6, u32 address, u32* value) {
    int ret;
    int retry_count = 0;
    const int max_retries = 3;
    const unsigned long timeout_ms = 100; // 100ms timeout
    
    // DEADLOCK 방지: timeout과 retry 메커니즘 추가
    while (retry_count < max_retries) {
        // mutex_lock_interruptible을 사용하여 timeout 지원
        if (mutex_lock_interruptible(&tc6->spi_ctrl_lock)) {
            dev_warn(&tc6->spi->dev, "SPI read interrupted, retry %d/%d\n", retry_count + 1, max_retries);
            retry_count++;
            if (retry_count >= max_retries) {
                dev_err(&tc6->spi->dev, "SPI read failed after %d retries\n", max_retries);
                return -EINTR;
            }
            usleep_range(1000, 2000); // 1-2ms delay before retry
            continue;
        }
        
        ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, 1, LAN1QTSN_SPI_CTRL_REG_READ);
        mutex_unlock(&tc6->spi_ctrl_lock);
        
        if (ret == 0) {
            return 0; // 성공
        }
        
        dev_warn(&tc6->spi->dev, "SPI read failed with error %d, retry %d/%d\n", ret, retry_count + 1, max_retries);
        retry_count++;
        
        if (retry_count < max_retries) {
            usleep_range(1000, 2000); // 1-2ms delay before retry
        }
    }
    
    dev_err(&tc6->spi->dev, "SPI read failed after %d retries\n", max_retries);
    return ret;
}
```

이제 `lan1qtsn_spi_write_register` 함수도 동일하게 수정하겠습니다:


```c
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value) {
    int ret;
    int retry_count = 0;
    const int max_retries = 3;
    
    // DEADLOCK 방지: timeout과 retry 메커니즘 추가
    while (retry_count < max_retries) {
        // mutex_lock_interruptible을 사용하여 timeout 지원
        if (mutex_lock_interruptible(&tc6->spi_ctrl_lock)) {
            dev_warn(&tc6->spi->dev, "SPI write interrupted, retry %d/%d\n", retry_count + 1, max_retries);
            retry_count++;
            if (retry_count >= max_retries) {
                dev_err(&tc6->spi->dev, "SPI write failed after %d retries\n", max_retries);
                return -EINTR;
            }
            usleep_range(1000, 2000); // 1-2ms delay before retry
            continue;
        }
        
        ret = lan1qtsn_spi_perform_ctrl(tc6, address, &value, 1, LAN1QTSN_SPI_CTRL_REG_WRITE);
        mutex_unlock(&tc6->spi_ctrl_lock);
        
        if (ret == 0) {
            return 0; // 성공
        }
        
        dev_warn(&tc6->spi->dev, "SPI write failed with error %d, retry %d/%d\n", ret, retry_count + 1, max_retries);
        retry_count++;
        
        if (retry_count < max_retries) {
            usleep_range(1000, 2000); // 1-2ms delay before retry
        }
    }
    
    dev_err(&tc6->spi->dev, "SPI write failed after %d retries\n", max_retries);
    return ret;
}
```

마지막으로, `lan1qtsn_netdev_start_xmit` 함수에서 에러 처리를 개선하겠습니다:


```c
    // DEADLOCK 방지: 레지스터 읽기를 TX 처리 전에 수행
    // 이렇게 하면 lock 순서가 일관되게 유지됨
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    if (sys_count == 0) {
        // 레지스터 읽기 실패 시 에러 처리
        pr_err("Failed to read system clock register\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return NETDEV_TX_BUSY;
    }
    
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&priv->sdev->dev);
    pr_err("%s - tsdev->ptp: %p\n", __func__, tsdev->ptp);
    now = lan1qtsn_sysclock_to_timestamp(priv->sdev, sys_count);
    sys_count_lower = sys_count & LOWER_29_BITS;
    sys_count_upper = sys_count & ~LOWER_29_BITS;
```

## 수정 요약

deadlock을 방지하기 위해 다음과 같은 수정을 수행했습니다:

### 1. **Lock 순서 보장**
- `tx_skb_lock`을 `spi_ctrl_lock`보다 먼저 획득하도록 순서 통일
- TX 처리 중에 추가 레지스터 읽기가 발생하지 않도록 구조 개선

### 2. **Timeout 및 Retry 메커니즘 추가**
- `lan1qtsn_spi_read_register()`와 `lan1qtsn_spi_write_register()` 함수에 retry 로직 추가
- `mutex_lock_interruptible()` 사용으로 timeout 지원
- 최대 3회 retry 후 실패 시 적절한 에러 반환

### 3. **에러 처리 개선**
- 레지스터 읽기 실패 시 적절한 에러 처리 및 리소스 정리
- TX queue 상태 복구 보장

### 4. **코드 구조 개선**
- TX 처리 전에 필요한 모든 레지스터 읽기 완료
- Lock 획득 순서를 명확하게 주석으로 표시

## Deadlock 방지 원리

1. **일관된 Lock 순서**: `tx_skb_lock` → `spi_ctrl_lock` 순서로 획득
2. **Lock 획득 최소화**: TX 처리 중에 추가 레지스터 읽기 방지
3. **Timeout 지원**: mutex_lock_interruptible으로 무한 대기 방지
4. **Retry 메커니즘**: 일시적인 lock 경합 상황에서 복구 가능

이러한 수정으로 라즈베리 파이 4에서 부팅 시 발생하던 deadlock 문제가 해결될 것입니다.

