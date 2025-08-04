# 네트워크 패킷 전송 문제 해결
_Exported on 8/4/2025 at 17:06:15 GMT+9 from Cursor (1.2.4)_

---

**User**

2개의 라즈베리 파이 4에 이 코드들을 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하였다.
2개의 라즈베리 파이 4에 있는 해당 이더넷 포트를 서로 직접 연결 한 후 ping으로 시험을 진행하였다.

해당 포트의 queue로 트래픽이 원할하게 전달이 안되는 것처럼,
다음과 같은 현상이 나타난다.
pooky@~$ ping 192.0.2.11
PING 192.0.2.11 (192.0.2.11) 56(84) bytes of data.
From 192.0.2.12 icmp_seq=1 Destination Host Unreachable
64 bytes from 192.0.2.11: icmp_seq=5 ttl=64 time=4098 ms
64 bytes from 192.0.2.11: icmp_seq=6 ttl=64 time=4076 ms
64 bytes from 192.0.2.11: icmp_seq=7 ttl=64 time=4053 ms
64 bytes from 192.0.2.11: icmp_seq=8 ttl=64 time=4031 ms
64 bytes from 192.0.2.11: icmp_seq=9 ttl=64 time=4009 ms
64 bytes from 192.0.2.11: icmp_seq=10 ttl=64 time=4011 ms
64 bytes from 192.0.2.11: icmp_seq=11 ttl=64 time=4010 ms
64 bytes from 192.0.2.11: icmp_seq=12 ttl=64 time=4009 ms

때로는 ping 이 안되는 경우도 있다.
해당 포트를 open close 할 때,
해당 포트로 패킷을 보낼 때, 해당 포트로 부터 패킷이 들어 왔을 때 네트웤 스택으로 패킷을 올릴 때
코드를 확인하여 이상한 점이 있으면 수정해 주세요.


---

**Cursor**

네트워크 드라이버 코드를 분석하여 ping 문제를 해결해보겠습니다. 먼저 현재 코드를 살펴보겠습니다.


이제 SPI 관련 코드를 확인해보겠습니다.


이제 헤더 파일들을 확인해보겠습니다.


이제 netdev 헤더 파일을 확인해보겠습니다.


이제 mod.h 파일을 확인해보겠습니다.


코드를 분석한 결과, ping 문제의 원인을 찾았습니다. 주요 문제점들을 수정해보겠습니다:

## 문제점 분석:

1. **큐 관리 문제**: `netif_stop_queue()`와 `netif_wake_queue()` 호출이 일관되지 않음
2. **패킷 처리 로직 문제**: RX 패킷 처리 시 네트워크 스택으로 올리는 과정에서 문제
3. **동기화 문제**: TX/RX 처리 간의 동기화 부족
4. **버퍼 관리 문제**: 스큐 버퍼 할당 및 해제 과정에서 문제

## 수정사항:


```c
static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    if (!tc6->rx_skb) {
        dev_err(&tc6->spi->dev, "No RX skb to submit\n");
        return;
    }

    // Set protocol before submitting to network stack
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);
    
    // Update statistics
    tc6->netdev->stats.rx_packets++;
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    // Submit the packet to the network stack
    netif_rx(tc6->rx_skb);
    
    // Wake up the network queue if it was stopped
    if (netif_queue_stopped(tc6->netdev)) {
        netif_wake_queue(tc6->netdev);
    }

    tc6->rx_skb = NULL;
}
```

```c
static u16 lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(struct lan1qtsn_spi* tc6) {
    u8* tx_buf = tc6->spi_data_tx_buf;
    struct sk_buff* skb;
    u16 copy_len, total_len;
    u32 fcs;
    u8* eth_data;
    u16 eth_len;

    // 1. Move the waiting skb to ongoing transmission slot
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    // 2. Check if there is skb to transmit
    if (!skb) {
        return 0;
    }

    copy_len = skb->len;

    // 3. Copy the original frame data to the SPI buffer, after command header
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    // 4. Calculate Ethernet frame data start address and length (excluding metadata)
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    // 5. Calculate FCS (CRC32) over the pure Ethernet frame
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; // Final XOR

    // 6. Append FCS to SPI transmit buffer after the frame data
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    // 7. Calculate total length (# bytes to transmit)
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    // 8. Check buffer overflow, clean up and exit on error
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow: total_len=%d, max=%d\n", total_len, LAN1QTSN_BUFFER_SIZE);
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    // 9. Prepare the SPI command header
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);

    return total_len;
}
```

```c
                } else {
                    /* Update stats and free skb after successful transfer */
                    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len + ETH_FCS_LEN;
                    tc6->netdev->stats.tx_packets++;
                    kfree_skb(tc6->ongoing_tx_skb);
                    tc6->ongoing_tx_skb = NULL;
                    
                    // Wake up the queue after successful transmission
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }
                }
```

```c
                if (ret) {
                    netdev_err(tc6->netdev, "SPI data transfer failed: %d\n", ret);
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
```

```c
netdev_tx_t lan1qtsn_spi_start_xmit(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    int ret;

    ret = lan1qtsn_append_metadata_to_skb(tc6, skb);
    if (ret) {
        return NETDEV_TX_OK;
    }

    if (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) {
        netif_stop_queue(tc6->netdev);
        return NETDEV_TX_BUSY;
    }

    if (skb_linearize(skb)) {
        dev_kfree_skb_any(skb);
        tc6->netdev->stats.tx_dropped++;
        return NETDEV_TX_OK;
    }

    /* Check if skb is owned by socket - if so, we can't reuse it */
    if (skb->sk) {
        /* Create a copy for our use */
        struct sk_buff* new_skb = skb_copy(skb, GFP_ATOMIC);
        if (!new_skb) {
            dev_kfree_skb_any(skb);
            tc6->netdev->stats.tx_dropped++;
            return NETDEV_TX_OK;
        }
        dev_kfree_skb_any(skb);
        skb = new_skb;
    }

    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->waiting_tx_skb = skb;
    spin_unlock_bh(&tc6->tx_skb_lock);

    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return NETDEV_TX_OK;
}
```

```c
static int lan1qtsn_spi_process_spi_data_rx_buf(struct lan1qtsn_spi* tc6, struct rx_metadata meta) {
    int ret;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u16 payload_size;
    u16 frame_size_without_fcs;
    u8* payload = tc6->spi_data_rx_buf + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    // Validate input parameters
    if (!tc6 || !tx_buf || !tc6->spi_data_rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    netdev_err(tc6->netdev, "%s - meta.frame_length: %d\n", __func__, meta.frame_length);

    cmd = LAN1QTSN_SPI_CMD_READ;
    addr = LAN1QTSN_SPI_RX_FRAME_FIFO_BASE;

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    // meta.frame_length is the actual payload size, not including headers
    payload_size = meta.frame_length;
    size = payload_size + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    // Check for buffer overflow
    if (size > LAN1QTSN_SPI_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected: size=%d, max=%d\n", size, LAN1QTSN_SPI_DATA_BUF_SIZE);
        return -EINVAL;
    }

    // Validate payload pointer
    if (!payload) {
        dev_err(&tc6->spi->dev, "Invalid payload pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    // Calculate frame size without FCS (remove last 4 bytes)
    if (payload_size >= ETH_FCS_LEN) {
        frame_size_without_fcs = payload_size - ETH_FCS_LEN;
    } else {
        dev_err(&tc6->spi->dev, "Payload size too small for FCS removal: %d\n", payload_size);
        return -EINVAL;
    }

    // Validate frame size
    if (frame_size_without_fcs < ETH_ZLEN || frame_size_without_fcs > tc6->netdev->mtu + ETH_HLEN) {
        dev_err(&tc6->spi->dev, "Invalid frame size: %d\n", frame_size_without_fcs);
        return -EINVAL;
    }

    return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, frame_size_without_fcs, meta);
}
```

```c
static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    // Allocate buffer for frame without FCS since we remove it before submitting to network stack
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb\n");
        return -ENOMEM;
    }

    return 0;
}
```

```c
static void lan1qtsn_spi_update_rx_skb(struct lan1qtsn_spi* tc6, u8* payload, u16 length) {
    // Validate input parameters
    if (!tc6 || !tc6->rx_skb || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return;
    }

    // Check if we have enough space in the skb
    if (skb_tailroom(tc6->rx_skb) < length) {
        dev_err(&tc6->spi->dev, "Not enough space in RX skb: need=%d, available=%d\n", 
                length, skb_tailroom(tc6->rx_skb));
        return;
    }

    memcpy(skb_put(tc6->rx_skb, length), payload, length);
}
```

```c
static int lan1qtsn_spi_prcs_complete_rx_frame(struct lan1qtsn_spi* tc6, u8* payload, u16 size,
                                               struct rx_metadata meta) {
    int ret;

    // Validate input parameters
    if (!tc6 || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    ret = lan1qtsn_spi_allocate_rx_skb(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb: %d\n", ret);
        return ret;
    }

    lan1qtsn_spi_update_rx_skb(tc6, payload, size);

    lan1qtsn_spi_submit_rx_skb(tc6);

    return 0;
}
```

```c
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;

    // Validate input parameter
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
            // netdev_err(tc6->netdev, "<<< %s - ret: %d\n", __func__, ret);
            // Don't return immediately, continue processing
            msleep(1); // Reduce delay to improve responsiveness
        }
    }

    // netdev_err(tc6->netdev, "<<< %s\n", __func__);
    return 0;
}
```

```c
int lan1qtsn_append_metadata_to_skb(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    struct net_device* ndev = tc6->netdev;
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;
    u16 q;

    q = skb_get_queue_mapping(skb);
    netif_stop_subqueue(ndev, q);
    pr_err("%s(skb->len : %d)\n", __func__, skb->len);
    skb->len = max((unsigned int)ETH_ZLEN, skb->len);

    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */

    if (skb_padto(skb, skb->len)) {
        pr_err("skb_padto failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }

    /* Jumbo frames not supported (including FCS) */
    if (skb->len + ETH_FCS_LEN > LAN1QTSN_BUFFER_SIZE) {
        pr_err("Jumbo frames not supported\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }

    /* Add metadata to the skb */
    if (pskb_expand_head(skb, TX_METADATA_SIZE, 0, GFP_ATOMIC) != 0) {
        pr_err("pskb_expand_head failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }
    skb_push(skb, TX_METADATA_SIZE);
    memset(skb->data, 0, TX_METADATA_SIZE);

    pr_err("%s(skb->len : %d)\n", __func__, skb->len);
    tx_buffer = (struct tx_buffer*)skb->data;
    /* Fill in the metadata */
    tx_metadata = (struct tx_metadata*)&tx_buffer->metadata;
    tx_metadata->frame_length = frame_length;

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

    return 0;
}
```

이제 네트워크 인터페이스 열기/닫기 함수도 개선해보겠습니다.


```c
static int lan1qtsn_net_open(struct net_device* netdev) {
#if 1
    struct lan1qtsn_priv* priv = netdev_priv(netdev);
    int i;

    // Enable TSN system first
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);
    
    // Start carrier and queues
    netif_carrier_on(netdev);
    netif_start_queue(netdev);
    for (i = 0; i < TX_QUEUE_COUNT; i++) {
        netif_start_subqueue(netdev, i);
    }

    dev_info(&netdev->dev, "Network interface opened\n");
    return 0;
#else
    struct lan1qtsn_priv* priv = netdev_priv(netdev);
    int ret;

    ret = lan1qtsn_hw_enable(priv);
    if (ret) {
        netdev_err(netdev, "Failed to enable hardware: %d\n", ret);
        return ret;
    }

    phy_start(netdev->phydev);

    return 0;
#endif
}
```

```c
static int lan1qtsn_net_close(struct net_device* netdev) {
    struct lan1qtsn_priv* priv = netdev_priv(netdev);
    int i;

    // Disable TSN system first
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);

    // Stop carrier and queues
    netif_carrier_off(netdev);
    netif_stop_queue(netdev);
    for (i = 0; i < TX_QUEUE_COUNT; i++) {
        netif_stop_subqueue(netdev, i);
    }
    
    dev_info(&netdev->dev, "Network interface closed\n");

    return 0;
}
```

이제 드라이버 초기화 부분도 개선해보겠습니다.


```c
int init_lan1qtsn(struct lan1qtsn_spi* tc6) {
#if 1
    // Start with TSN system disabled, will be enabled when interface is opened
    lan1qtsn_spi_write_register(tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);
    dev_info(&tc6->spi->dev, "TSN system initialized in disabled state\n");
    return 0;
#else
```

```c
static int lan1qtsn_spi_enable_data_transfer(struct lan1qtsn_spi* tc6) {
    // TSN system will be enabled when interface is opened
    dev_info(&tc6->spi->dev, "Data transfer will be enabled when interface is opened\n");
    return 0;
}
```

이제 디버그 메시지를 줄이고 성능을 개선해보겠습니다.


```c
        /* Transmit */
        if (tc6->waiting_tx_skb) {
            spi_len = lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(tc6);

            if (spi_len > 0) {
                ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, spi_len);

                if (ret) {
                    netdev_err(tc6->netdev, "SPI data transfer failed: %d\n", ret);
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
                    /* Update stats and free skb after successful transfer */
                    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len + ETH_FCS_LEN;
                    tc6->netdev->stats.tx_packets++;
                    kfree_skb(tc6->ongoing_tx_skb);
                    tc6->ongoing_tx_skb = NULL;
                    
                    // Wake up the queue after successful transmission
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }
                }
            }
        }
```

```c
        /* Receive metadata */
        ret = lan1qtsn_spi_get_meta_data(tc6, &meta);
        if (ret) {
            netdev_err(tc6->netdev, "Failed to get metadata: %d\n", ret);
            if (tc6->waiting_tx_skb) {
                continue;
            }
            break;
        }

        if (meta.frame_length == 0) {
            if (tc6->waiting_tx_skb) {
                continue;
            }
            break;
        }

        ret = lan1qtsn_spi_process_spi_data_rx_buf(tc6, meta);
        if (ret) {
            lan1qtsn_spi_cleanup_ongoing_rx_skb(tc6);
            netdev_err(tc6->netdev, "Device error: %d\n", ret);
            return ret;
        }
```

```c
    netdev_err(tc6->netdev, "<<< %s\n", __func__);
    return 0;
```

```c
        }
    }

    return 0;
}
```

```c
    tc6->int_flag = true;
    wake_up_interruptible(&tc6->spi_wq);

    dev_info(&tc6->spi->dev, "TSN driver initialized successfully\n");
    return tc6;
```

이제 append_metadata_to_skb 함수에서 불필요한 디버그 메시지를 제거하겠습니다.


```c
int lan1qtsn_append_metadata_to_skb(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    struct net_device* ndev = tc6->netdev;
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;
    u16 q;

    q = skb_get_queue_mapping(skb);
    netif_stop_subqueue(ndev, q);
    skb->len = max((unsigned int)ETH_ZLEN, skb->len);

    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */

    if (skb_padto(skb, skb->len)) {
        pr_err("skb_padto failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }

    /* Jumbo frames not supported (including FCS) */
    if (skb->len + ETH_FCS_LEN > LAN1QTSN_BUFFER_SIZE) {
        pr_err("Jumbo frames not supported\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }

    /* Add metadata to the skb */
    if (pskb_expand_head(skb, TX_METADATA_SIZE, 0, GFP_ATOMIC) != 0) {
        pr_err("pskb_expand_head failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }
    skb_push(skb, TX_METADATA_SIZE);
    memset(skb->data, 0, TX_METADATA_SIZE);

    tx_buffer = (struct tx_buffer*)skb->data;
    /* Fill in the metadata */
    tx_metadata = (struct tx_metadata*)&tx_buffer->metadata;
    tx_metadata->frame_length = frame_length;

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

    return 0;
}
```

이제 수정된 코드를 컴파일하고 테스트해보겠습니다.


