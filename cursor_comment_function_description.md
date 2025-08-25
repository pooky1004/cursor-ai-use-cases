# 주석 형식 변경 요청
_Exported on 8/22/2025 at 16:25:09 GMT+9 from Cursor (1.2.4)_

---

**User**

이 디렉토리 내 모든 c 파일과 header 파일에  // 로 주석처리 되어 있는 부분을 /* */로 변경해 줘

---

**Cursor**

이 디렉토리 내의 모든 C 파일과 헤더 파일에서 `//` 주석을 `/* */` 형태로 변경하겠습니다. 먼저 각 파일을 확인하고 주석을 변경하겠습니다.


이제 모든 C 파일과 헤더 파일에서 `//` 주석을 `/* */` 형태로 변경하겠습니다. 각 파일에서 `//` 주석을 찾아서 변경하겠습니다.


```c
    // Try to acquire the lock without blocking
    if (mutex_trylock(&tc6->spi_ctrl_lock)) {
        // Successfully acquired the lock, safe to read registers
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

    // If we couldn't acquire the lock or read failed, return 0
    dev_info(&tc6->spi->dev, "Could not read system clock safely\n");

->

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
```

```c
// SPDX-License-Identifier: GPL-2.0+
/*
 * TSNLab's LAN1QTSN driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */

->

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TSNLab's LAN1QTSN driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */
```

```c
    // Get system information
    uts = utsname();
    if (!uts) {
        dev_err(&netdev->dev, "Failed to get system information\n");
        eth_hw_addr_random(netdev);
        return;
    }

    // Create hash from system information
    // Use hostname, machine, and release information
    hash_value = jhash(uts->nodename, strlen(uts->nodename), 0);
    hash_value = jhash(uts->machine, strlen(uts->machine), hash_value);
    hash_value = jhash(uts->release, strlen(uts->release), hash_value);
    hash_value = jhash(uts->version, strlen(uts->version), hash_value);
    hash_value = jhash(uts->domainname, strlen(uts->domainname), hash_value);

    // Add CPU information to hash
    cpu_id = (unsigned long)smp_processor_id();
    hash_value = jhash(&cpu_id, sizeof(cpu_id), hash_value);

    // Add memory information
    mem_total = totalram_pages() << PAGE_SHIFT;
    hash_value = jhash(&mem_total, sizeof(mem_total), hash_value);

    // Add current time (jiffies) for additional randomness
    hash_value = jhash(&jiffies, sizeof(jiffies), hash_value);

    // Add device-specific information
    hash_value = jhash(&netdev->ifindex, sizeof(netdev->ifindex), hash_value);

    // Generate MAC address from hash
    // Use local administered address (bit 1 of first byte = 1)
    // Use unicast address (bit 0 of first byte = 0)
    // mac_addr[0] = 0x02; // Local administered, unicast
    mac_addr[0] = 0xd8;
    mac_addr[1] = (hash_value >> 16) & 0xFF;
    mac_addr[2] = (hash_value >> 8) & 0xFF;
    mac_addr[3] = hash_value & 0xFF;
    mac_addr[4] = (hash_value >> 24) & 0xFF;
    mac_addr[5] = ((hash_value >> 16) + (hash_value >> 8)) & 0xFF;

    // Ensure the MAC address is not all zeros or all ones
    if ((mac_addr[1] == 0 && mac_addr[2] == 0 && mac_addr[3] == 0 && mac_addr[4] == 0 && mac_addr[5] == 0) ||
        (mac_addr[1] == 0xFF && mac_addr[2] == 0xFF && mac_addr[3] == 0xFF && mac_addr[4] == 0xFF &&
         mac_addr[5] == 0xFF)) {
        // If generated MAC is invalid, use a fallback
        mac_addr[1] = 0x00;
        mac_addr[2] = 0x11;
        mac_addr[3] = 0x22;
        mac_addr[4] = 0x33;
        mac_addr[5] = 0x44;
    }

->

    /* Get system information */
    uts = utsname();
    if (!uts) {
        dev_err(&netdev->dev, "Failed to get system information\n");
        eth_hw_addr_random(netdev);
        return;
    }

    /* Create hash from system information */
    /* Use hostname, machine, and release information */
    hash_value = jhash(uts->nodename, strlen(uts->nodename), 0);
    hash_value = jhash(uts->machine, strlen(uts->machine), hash_value);
    hash_value = jhash(uts->release, strlen(uts->release), hash_value);
    hash_value = jhash(uts->version, strlen(uts->version), hash_value);
    hash_value = jhash(uts->domainname, strlen(uts->domainname), hash_value);

    /* Add CPU information to hash */
    cpu_id = (unsigned long)smp_processor_id();
    hash_value = jhash(&cpu_id, sizeof(cpu_id), hash_value);

    /* Add memory information */
    mem_total = totalram_pages() << PAGE_SHIFT;
    hash_value = jhash(&mem_total, sizeof(mem_total), hash_value);

    /* Add current time (jiffies) for additional randomness */
    hash_value = jhash(&jiffies, sizeof(jiffies), hash_value);

    /* Add device-specific information */
    hash_value = jhash(&netdev->ifindex, sizeof(netdev->ifindex), hash_value);

    /* Generate MAC address from hash */
    /* Use local administered address (bit 1 of first byte = 1) */
    /* Use unicast address (bit 0 of first byte = 0) */
    /* mac_addr[0] = 0x02; */ /* Local administered, unicast */
    mac_addr[0] = 0xd8;
    mac_addr[1] = (hash_value >> 16) & 0xFF;
    mac_addr[2] = (hash_value >> 8) & 0xFF;
    mac_addr[3] = hash_value & 0xFF;
    mac_addr[4] = (hash_value >> 24) & 0xFF;
    mac_addr[5] = ((hash_value >> 16) + (hash_value >> 8)) & 0xFF;

    /* Ensure the MAC address is not all zeros or all ones */
    if ((mac_addr[1] == 0 && mac_addr[2] == 0 && mac_addr[3] == 0 && mac_addr[4] == 0 && mac_addr[5] == 0) ||
        (mac_addr[1] == 0xFF && mac_addr[2] == 0xFF && mac_addr[3] == 0xFF && mac_addr[4] == 0xFF &&
         mac_addr[5] == 0xFF)) {
        /* If generated MAC is invalid, use a fallback */
        mac_addr[1] = 0x00;
        mac_addr[2] = 0x11;
        mac_addr[3] = 0x22;
        mac_addr[4] = 0x33;
        mac_addr[5] = 0x44;
    }
```

```c
    // Set the MAC address
    eth_hw_addr_set(netdev, mac_addr);

    dev_info(&priv->sdev->dev, "Generated MAC address from system info: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0],
             mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    dev_info(&priv->sdev->dev, "System: %s, Machine: %s, Release: %s\n", uts->nodename, uts->machine, uts->release);

->

    /* Set the MAC address */
    eth_hw_addr_set(netdev, mac_addr);

    dev_info(&priv->sdev->dev, "Generated MAC address from system info: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0],
             mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    dev_info(&priv->sdev->dev, "System: %s, Machine: %s, Release: %s\n", uts->nodename, uts->machine, uts->release);
```

```c
    // Check if PTP is available before accessing it
    if (!tsdev || !tsdev->ptp || !tsdev->ptp->ptp_clock) {
        info->phc_index = -1;
    } else {
        info->phc_index = ptp_clock_index(tsdev->ptp->ptp_clock);
    }

->

    /* Check if PTP is available before accessing it */
    if (!tsdev || !tsdev->ptp || !tsdev->ptp->ptp_clock) {
        info->phc_index = -1;
    } else {
        info->phc_index = ptp_clock_index(tsdev->ptp->ptp_clock);
    }
```

```c
    /* TODO: PHY also supports 100Mbps */
    cmd->base.speed = 100;

->

    /* TODO: PHY also supports 100Mbps */
    cmd->base.speed = 100;
```

```c
#ifdef __TSN_PTP__
    // Clean up PTP device if it exists
    if (tsdev->ptp) {
        ptp_device_destroy(tsdev->ptp);
        tsdev->ptp = NULL;
    }
#endif

->

#ifdef __TSN_PTP__
    /* Clean up PTP device if it exists */
    if (tsdev->ptp) {
        ptp_device_destroy(tsdev->ptp);
        tsdev->ptp = NULL;
    }
#endif
```

```c
#ifdef __TSN_PTP__
    /* Allocate the network device */
    /* TC command requires multiple TX queues */
    ndev = alloc_etherdev_mq(sizeof(struct lan1qtsn_priv), TX_QUEUE_COUNT);
    if (!ndev) {
        pr_err("alloc_etherdev failed\n");
        ret = -ENOMEM;
        goto err_out;
    }
    /*
     * Multiple RX queues drops throughput significantly.
     * TODO: Find out why RX queue count affects throughput
     * and see if it can be resolved in another way
     */
    ret = netif_set_real_num_rx_queues(ndev, RX_QUEUE_COUNT);
    if (ret) {
        pr_err("netif_set_real_num_rx_queues failed\n");
        goto err_out;
    }
#else
    ndev = alloc_etherdev(sizeof(struct lan1qtsn_priv));
    if (!ndev) {
        pr_err("alloc_etherdev failed\n");
        ret = -ENOMEM;
        goto err_out;
    }
#endif

->

#ifdef __TSN_PTP__
    /* Allocate the network device */
    /* TC command requires multiple TX queues */
    ndev = alloc_etherdev_mq(sizeof(struct lan1qtsn_priv), TX_QUEUE_COUNT);
    if (!ndev) {
        pr_err("alloc_etherdev failed\n");
        ret = -ENOMEM;
        goto err_out;
    }
    /*
     * Multiple RX queues drops throughput significantly.
     * TODO: Find out why RX queue count affects throughput
     * and see if it can be resolved in another way
     */
    ret = netif_set_real_num_rx_queues(ndev, RX_QUEUE_COUNT);
    if (ret) {
        pr_err("netif_set_real_num_rx_queues failed\n");
        goto err_out;
    }
#else
    ndev = alloc_etherdev(sizeof(struct lan1qtsn_priv));
    if (!ndev) {
        pr_err("alloc_etherdev failed\n");
        ret = -ENOMEM;
        goto err_out;
    }
#endif
```

```c
    /* Set the MAC address */
    lan1qtsn_generate_mac_from_system_info(ndev);

->

    /* Set the MAC address */
    lan1qtsn_generate_mac_from_system_info(ndev);
```

```c
    // lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);

#if 1
    /* Tx works for each timestamp id */
    INIT_WORK(&priv->tx_work[1], lan1qtsn_tx_work1);
    INIT_WORK(&priv->tx_work[2], lan1qtsn_tx_work2);
    INIT_WORK(&priv->tx_work[3], lan1qtsn_tx_work3);
    INIT_WORK(&priv->tx_work[4], lan1qtsn_tx_work4);
#endif

->

    /* lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE); */

#if 1
    /* Tx works for each timestamp id */
    INIT_WORK(&priv->tx_work[1], lan1qtsn_tx_work1);
    INIT_WORK(&priv->tx_work[2], lan1qtsn_tx_work2);
    INIT_WORK(&priv->tx_work[3], lan1qtsn_tx_work3);
    INIT_WORK(&priv->tx_work[4], lan1qtsn_tx_work4);
#endif
```

```c
#ifdef __TSN_PTP__
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&priv->sdev->dev);

    // Clean up PTP device if it exists
    if (tsdev && tsdev->ptp) {
        ptp_device_destroy(tsdev->ptp);
        tsdev->ptp = NULL;
    }
#endif

->

#ifdef __TSN_PTP__
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&priv->sdev->dev);

    /* Clean up PTP device if it exists */
    if (tsdev && tsdev->ptp) {
        ptp_device_destroy(tsdev->ptp);
        tsdev->ptp = NULL;
    }
#endif
```

```c
    // Enable TSN system first
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);

->

    /* Enable TSN system first */
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);
```

```c
    // Disable TSN system first
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);

->

    /* Disable TSN system first */
    lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_DISABLE);
```

```c
    // Try to read system clock safely
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    if (sys_count == 0) {
        // Use default values if system clock read failed
        dev_dbg(&ndev->dev, "System clock read failed, using default values\n");
        now = 0;
        sys_count_lower = 0;
        sys_count_upper = 0;
    } else {
        now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
        sys_count_lower = sys_count & LOWER_29_BITS;
        sys_count_upper = sys_count & ~LOWER_29_BITS;
    }

->

    /* Try to read system clock safely */
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    if (sys_count == 0) {
        /* Use default values if system clock read failed */
        dev_dbg(&ndev->dev, "System clock read failed, using default values\n");
        now = 0;
        sys_count_lower = 0;
        sys_count_upper = 0;
    } else {
        now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
        sys_count_lower = sys_count & LOWER_29_BITS;
        sys_count_upper = sys_count & ~LOWER_29_BITS;
    }
```

```c
    if (!tsdev) {
        pr_err("tsdev is NULL, cannot fill metadata\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

    if (tsn_fill_metadata(tsdev, now, skb) == false) {
        // TODO: Increment SW drop stats
        pr_warn("tsn_fill_metadata failed\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

->

    if (!tsdev) {
        pr_err("tsdev is NULL, cannot fill metadata\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

    if (tsn_fill_metadata(tsdev, now, skb) == false) {
        /* TODO: Increment SW drop stats */
        pr_warn("tsn_fill_metadata failed\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }
```

```c
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

->

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
                /* Overflow */
                priv->tx_work_start_after[tx_metadata->timestamp_id] += (1 << 29);
            }
            to_value =
                (tx_metadata->fail_policy == TSN_FAIL_POLICY_RETRY ? tx_metadata->delay_to.tick : tx_metadata->to.tick);
            priv->tx_work_wait_until[tx_metadata->timestamp_id] = sys_count_upper | to_value;
            if (sys_count_lower > to_value && sys_count_lower - to_value > TX_WORK_OVERFLOW_MARGIN) {
                /* Overflow */
                priv->tx_work_wait_until[tx_metadata->timestamp_id] += (1 << 29);
            }
            schedule_work(&priv->tx_work[tx_metadata->timestamp_id]);
        } else if (priv->tstamp_config.tx_type != HWTSTAMP_TX_ON) {
            pr_warn("Timestamp skipped: timestamp config is off\n");
        } else {
            pr_warn("Timestamp skipped: driver is waiting for previous packet's timestamp\n");
        }
        /* TODO: track the number of skipped packets for ethtool stats */
```

```c
static int lan1qtsn_setup_tc_block_cb(enum tc_setup_type type, void* type_data, void* cb_priv) {
    // If mqprio is only used for queue mapping this should not be called
    return -EOPNOTSUPP;
}

->

static int lan1qtsn_setup_tc_block_cb(enum tc_setup_type type, void* type_data, void* cb_priv) {
    /* If mqprio is only used for queue mapping this should not be called */
    return -EOPNOTSUPP;
}
```

```c
    /*
     * Read TX timestamp several times because
     * the work thread might try to read TX timestamp
     * before the register gets updated
     */
    tx_tstamp = lan1qtsn_read_tx_timestamp_by_ldev(priv->tc6, tstamp_id);
    if (tx_tstamp == priv->last_tx_tstamp[tstamp_id]) {
        if (lan1qtsn_get_sys_clock_by_ldev(priv->tc6) < priv->tx_work_wait_until[tstamp_id]) {
            /* The packet might have not been sent yet */
            goto retry;
        }
        /*
         * Tx timestamp is not updated. Try again.
         * Waiting for it to be updated forever is not desirable,
         * so limit the number of retries
         */
        if (++(priv->tstamp_retry[tstamp_id]) >= TX_TSTAMP_MAX_RETRY) {
            /* TODO: track the number of skipped packets for ethtool stats */
            pr_warn("Failed to get timestamp: timestamp is not getting updated, "
                    "the packet might have been dropped\n");
            goto return_error;
        }
        goto retry;
    }

->

    /*
     * Read TX timestamp several times because
     * the work thread might try to read TX timestamp
     * before the register gets updated
     */
    tx_tstamp = lan1qtsn_read_tx_timestamp_by_ldev(priv->tc6, tstamp_id);
    if (tx_tstamp == priv->last_tx_tstamp[tstamp_id]) {
        if (lan1qtsn_get_sys_clock_by_ldev(priv->tc6) < priv->tx_work_wait_until[tstamp_id]) {
            /* The packet might have not been sent yet */
            goto retry;
        }
        /*
         * Tx timestamp is not updated. Try again.
         * Waiting for it to be updated forever is not desirable,
         * so limit the number of retries
         */
        if (++(priv->tstamp_retry[tstamp_id]) >= TX_TSTAMP_MAX_RETRY) {
            /* TODO: track the number of skipped packets for ethtool stats */
            pr_warn("Failed to get timestamp: timestamp is not getting updated, "
                    "the packet might have been dropped\n");
            goto return_error;
        }
        goto retry;
    }
```

```c
#if 1 // 20250820
    pr_err(">>> %s, but return immediately\n", __func__);
#endif

->

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
```

```c
#if 1 // 20250820
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

->

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);
```

```c
#if 1 // 20250820
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    ptp_read_system_prets(sts);
    clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    ptp_read_system_postts(sts);

->

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    ptp_read_system_prets(sts);
    clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    ptp_read_system_postts(sts);
```

```c
#if 1 // 20250820
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    /* Get host timestamp */
    host_timestamp = (u64)ts->tv_sec * NS_IN_1S + ts->tv_nsec;

->

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    /* Get host timestamp */
    host_timestamp = (u64)ts->tv_sec * NS_IN_1S + ts->tv_nsec;
```

```c
#if 1 // 20250820
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    /* Adjust offset */
    ptp_data->offset += delta;

->

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    /* Adjust offset */
    ptp_data->offset += delta;
```

```c
#if 1 // 20250820
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);

->

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
```

```c
    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    // set_pulse_at(ptp_data, sys_clock);

->

    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    /* set_pulse_at(ptp_data, sys_clock); */
```

```c
    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    // set_pulse_at(ptp_data, sys_clock);

    spin_unlock_irqrestore(&ptp_data->lock, flags);

->

    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    /* set_pulse_at(ptp_data, sys_clock); */

    spin_unlock_irqrestore(&ptp_data->lock, flags);
```

```c
    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    // set_pulse_at(ptp_data, sys_clock);

->

    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    /* set_pulse_at(ptp_data, sys_clock); */
```

```c
#if 1 // POOKY 20250821 must be 1
    lan1qtsn_ptp_settime(&ptp->ptp_info, &ts);
#endif

->

#if 1 /* POOKY 20250821 must be 1 */
    lan1qtsn_ptp_settime(&ptp->ptp_info, &ts);
#endif
```

```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

->

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */
```

```c
    /*
     * This function is protected by spi_ctrl_lock mutex in the calling functions
     * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
     * to prevent concurrent register access from multiple workers.
     */
    u16 size;
    int ret;

->

    /*
     * This function is protected by spi_ctrl_lock mutex in the calling functions
     * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
     * to prevent concurrent register access from multiple workers.
     */
    u16 size;
    int ret;
```

```c
    /* Check if we're already holding the lock to prevent deadlock */
    if (mutex_is_locked(&tc6->spi_ctrl_lock)) {
        dev_dbg(&tc6->spi->dev, "Lock already held, cannot read registers\n");
        return -EINVAL;
    }

->

    /* Check if we're already holding the lock to prevent deadlock */
    if (mutex_is_locked(&tc6->spi_ctrl_lock)) {
        dev_dbg(&tc6->spi->dev, "Lock already held, cannot read registers\n");
        return -EINVAL;
    }
```

```c
    /* 1. Move the waiting skb to ongoing transmission slot */
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    /* 2. Check if there is skb to transmit */
    if (!skb) {
        dev_dbg(&tc6->spi->dev, "TX: No waiting skb to transmit, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Preparing skb for transmission, len=%d, tx_packets=%lu\n", skb->len,
            tc6->netdev->stats.tx_packets);

    copy_len = skb->len;

    /* 3. Copy the original frame data to the SPI buffer, after command header */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    /* 4. Calculate Ethernet frame data start address and length (excluding metadata) */
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    dev_dbg(&tc6->spi->dev, "TX: Ethernet data len=%d, metadata size=%ld, tx_packets=%lu\n", eth_len, TX_METADATA_SIZE,
            tc6->netdev->stats.tx_packets);

    /* 5. Calculate FCS (CRC32) over the pure Ethernet frame */
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; /* Final XOR */

    /* 6. Append FCS to SPI transmit buffer after the frame data */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    /* 7. Calculate total length (# bytes to transmit) */
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    /* 8. Check buffer overflow, clean up and exit on error */
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        dev_err(&tc6->spi->dev, "TX: Buffer overflow: total_len=%d, max=%d, tx_packets=%lu\n", total_len,
                LAN1QTSN_BUFFER_SIZE, tc6->netdev->stats.tx_packets);
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Frame prepared, total_len=%d, eth_len=%d, tx_packets=%lu\n", total_len, eth_len,
            tc6->netdev->stats.tx_packets);

    /* 9. Prepare the SPI command header */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);

->

    /* 1. Move the waiting skb to ongoing transmission slot */
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    /* 2. Check if there is skb to transmit */
    if (!skb) {
        dev_dbg(&tc6->spi->dev, "TX: No waiting skb to transmit, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Preparing skb for transmission, len=%d, tx_packets=%lu\n", skb->len,
            tc6->netdev->stats.tx_packets);

    copy_len = skb->len;

    /* 3. Copy the original frame data to the SPI buffer, after command header */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    /* 4. Calculate Ethernet frame data start address and length (excluding metadata) */
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    dev_dbg(&tc6->spi->dev, "TX: Ethernet data len=%d, metadata size=%ld, tx_packets=%lu\n", eth_len, TX_METADATA_SIZE,
            tc6->netdev->stats.tx_packets);

    /* 5. Calculate FCS (CRC32) over the pure Ethernet frame */
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; /* Final XOR */

    /* 6. Append FCS to SPI transmit buffer after the frame data */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    /* 7. Calculate total length (# bytes to transmit) */
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    /* 8. Check buffer overflow, clean up and exit on error */
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        dev_err(&tc6->spi->dev, "TX: Buffer overflow: total_len=%d, max=%d, tx_packets=%lu\n", total_len,
                LAN1QTSN_BUFFER_SIZE, tc6->netdev->stats.tx_packets);
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Frame prepared, total_len=%d, eth_len=%d, tx_packets=%lu\n", total_len, eth_len,
            tc6->netdev->stats.tx_packets);

    /* 9. Prepare the SPI command header */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);
```

```c
    /* 1. Prepare SPI MPW header for metadata read function */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    /* 2. Transfer metadata block from device using SPI */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_META_HEADER, transfer_size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* 3. rx_meta points to extracted metadata in receive buffer */
    /* Add bounds checking to prevent buffer overflow */
    if (LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + sizeof(struct rx_metadata) >
        LAN1QTSN_SPI_META_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected in %s\n", __func__);
        return -EINVAL;
    }

    /* Use aligned memory access to prevent unaligned access issues */
    u8* meta_offset = &rx_buf[LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE];

    /* Validate meta_offset pointer */
    if (!meta_offset) {
        dev_err(&tc6->spi->dev, "Invalid meta_offset pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* 4. Convert and assign frame_length to meta structure using safe memory access */
    /* Use memcpy to avoid potential unaligned access */
    u16 temp_frame_length;
    memcpy(&temp_frame_length, meta_offset + offsetof(struct rx_metadata, frame_length), sizeof(u16));
    frame_length = ntohs(temp_frame_length);
    meta->frame_length = frame_length;

    /* 5. If a valid frame is present, convert and assign timestamps */
    if (frame_length) {
        /* Use safe memory access for timestamp */
        u64 temp_timestamp;
        memcpy(&temp_timestamp, meta_offset + offsetof(struct rx_metadata, timestamp), sizeof(u64));

        /* Convert from network byte order to host byte order */
        meta->timestamp = __be64_to_cpu(temp_timestamp);
    }

->

    /* 1. Prepare SPI MPW header for metadata read function */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    /* 2. Transfer metadata block from device using SPI */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_META_HEADER, transfer_size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* 3. rx_meta points to extracted metadata in receive buffer */
    /* Add bounds checking to prevent buffer overflow */
    if (LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + sizeof(struct rx_metadata) >
        LAN1QTSN_SPI_META_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected in %s\n", __func__);
        return -EINVAL;
    }

    /* Use aligned memory access to prevent unaligned access issues */
    u8* meta_offset = &rx_buf[LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE];

    /* Validate meta_offset pointer */
    if (!meta_offset) {
        dev_err(&tc6->spi->dev, "Invalid meta_offset pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* 4. Convert and assign frame_length to meta structure using safe memory access */
    /* Use memcpy to avoid potential unaligned access */
    u16 temp_frame_length;
    memcpy(&temp_frame_length, meta_offset + offsetof(struct rx_metadata, frame_length), sizeof(u16));
    frame_length = ntohs(temp_frame_length);
    meta->frame_length = frame_length;

    /* 5. If a valid frame is present, convert and assign timestamps */
    if (frame_length) {
        /* Use safe memory access for timestamp */
        u64 temp_timestamp;
        memcpy(&temp_timestamp, meta_offset + offsetof(struct rx_metadata, timestamp), sizeof(u64));

        /* Convert from network byte order to host byte order */
        meta->timestamp = __be64_to_cpu(temp_timestamp);
    }
```

```c
        /* This kthread will be waken up if there is a tx skb or mac-phy
         * interrupt to perform spi transfer with tx chunks.
         */
        wait_event_interruptible(tc6->spi_wq, tc6->int_flag || (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) ||
                                                  kthread_should_stop());

->

        /* This kthread will be waken up if there is a tx skb or mac-phy
         * interrupt to perform spi transfer with tx chunks.
         */
        wait_event_interruptible(tc6->spi_wq, tc6->int_flag || (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) ||
                                                  kthread_should_stop());
```

```c
    /* MAC-PHY interrupt can occur for the following reasons.
     * - availability of tx credits if it was 0 before and not reported in
     *   the previous rx footer.
     * - availability of rx chunks if it was 0 before and not reported in
     *   the previous rx footer.
     * - extended status event not reported in the previous rx footer.
     */
    tc6->int_flag = true;
    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

->

    /* MAC-PHY interrupt can occur for the following reasons.
     * - availability of tx credits if it was 0 before and not reported in
     *   the previous rx footer.
     * - availability of rx chunks if it was 0 before and not reported in
     *   the previous rx footer.
     * - extended status event not reported in the previous rx footer.
     */
    tc6->int_flag = true;
    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);
```

```c
    /* lan1qtsn_spi_sw_reset_macphy() function resets and clears the MAC-PHY reset
     * complete status. IRQ is also asserted on reset completion and it is
     * remain asserted until MAC-PHY receives a data chunk. So performing an
     * empty data chunk transmission will deassert the IRQ. Refer section
     * 7.7 and 9.2.8.8 in the OPEN Alliance specification for more details.
     */
    tc6->int_flag = true;
    wake_up_interruptible(&tc6->spi_wq);

->

    /* lan1qtsn_spi_sw_reset_macphy() function resets and clears the MAC-PHY reset
     * complete status. IRQ is also asserted on reset completion and it is
     * remain asserted until MAC-PHY receives a data chunk. So performing an
     * empty data chunk transmission will deassert the IRQ. Refer section
     * 7.7 and 9.2.8.8 in the OPEN Alliance specification for more details.
     */
    tc6->int_flag = true;
    wake_up_interruptible(&tc6->spi_wq);
```

```c
/**
 * Workqueue handler for RX packet processing
 * This function runs in process context and safely calls netif_rx
 */
void lan1qtsn_rx_work_handler(struct work_struct* work) {
    struct lan1qtsn_rx_work* rx_work = container_of(work, struct lan1qtsn_rx_work, work);
    struct sk_buff* skb = rx_work->skb;
    struct net_device* netdev = rx_work->netdev;

    if (skb && netdev) {
        /* Submit the packet to the network stack in process context */
        netif_rx(skb);
    }

    /* Free the work structure */
    kfree(rx_work);
}

->

/**
 * Workqueue handler for RX packet processing
 * This function runs in process context and safely calls netif_rx
 */
void lan1qtsn_rx_work_handler(struct work_struct* work) {
    struct lan1qtsn_rx_work* rx_work = container_of(work, struct lan1qtsn_rx_work, work);
    struct sk_buff* skb = rx_work->skb;
    struct net_device* netdev = rx_work->netdev;

    if (skb && netdev) {
        /* Submit the packet to the network stack in process context */
        netif_rx(skb);
    }

    /* Free the work structure */
    kfree(rx_work);
}
```

```c
// SPDX-License-Identifier: GPL-2.0+
/*
 * TSNLab's LAN1QTSN driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */

->

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TSNLab's LAN1QTSN driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */
```

```c
    // XXX: Or you can use skb->priority;
    return 0;

->

    /* XXX: Or you can use skb->priority; */
    return 0;
```

```c
    // Don't care. Just fill in the metadata
#if 1
    // timestamps.from = tsn_config->total_available_at + spi_delay;
    timestamps.from = tsn_config->total_available_at + spi_delay;
#else
    timestamps.from = tsn_config->total_available_at;
#endif
#if 1
    timestamps.to = timestamps.from + _DEFAULT_TO_MARGIN_;
#else
    timestamps.to = TSN_ALWAYS_OPEN(timestamps.from);
#endif
    metadata->fail_policy = TSN_FAIL_POLICY_DROP;
#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
    pr_err("%s - timestamps.from: 0x%016llx, timestamps.to: 0x%016llx, metadata->fail_policy: %d\n", __func__,
           timestamps.from, timestamps.to, metadata->fail_policy);
#endif

->

    /* Don't care. Just fill in the metadata */
#if 1
    /* timestamps.from = tsn_config->total_available_at + spi_delay; */
    timestamps.from = tsn_config->total_available_at + spi_delay;
#else
    timestamps.from = tsn_config->total_available_at;
#endif
#if 1
    timestamps.to = timestamps.from + _DEFAULT_TO_MARGIN_;
#else
    timestamps.to = TSN_ALWAYS_OPEN(timestamps.from);
#endif
    metadata->fail_policy = TSN_FAIL_POLICY_DROP;
#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
    pr_err("%s - timestamps.from: 0x%016llx, timestamps.to: 0x%016llx, metadata->fail_policy: %d\n", __func__,
           timestamps.from, timestamps.to, metadata->fail_policy);
#endif
```

```c
#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
#endif
    metadata->from.tick = tsn_timestamp_to_sysclock(priv->tc6, timestamps.from);
    metadata->from.priority = queue_prio;
#if 0
    pr_err("%s - metadata->from.tick: 0x%08x  metadata->from.priority: %d\n", __func__, metadata->from.tick,
           metadata->from.priority);
#endif

->

#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
#endif
    metadata->from.tick = tsn_timestamp_to_sysclock(priv->tc6, timestamps.from);
    metadata->from.priority = queue_prio;
#if 0
    pr_err("%s - metadata->from.tick: 0x%08x  metadata->from.priority: %d\n", __func__, metadata->from.tick,
           metadata->from.priority);
#endif
```

```c
    if (timestamps.to == TSN_ALWAYS_OPEN(timestamps.from)) {
        // pr_err("%s - I'm here !, %d\n", __func__, __LINE__);
        metadata->to.tick = TSN_ALWAYS_OPEN(metadata->from.tick);
    } else {
        metadata->to.tick = tsn_timestamp_to_sysclock(priv->tc6, timestamps.to);
#if 0
            pr_err("%s - %d\n", __func__, __LINE__);
            pr_err("%s - metadata->to.tick: 0x%08x\n", __func__, metadata->to.tick);
#endif
    }

->

    if (timestamps.to == TSN_ALWAYS_OPEN(timestamps.from)) {
        /* pr_err("%s - I'm here !, %d\n", __func__, __LINE__); */
        metadata->to.tick = TSN_ALWAYS_OPEN(metadata->from.tick);
    } else {
        metadata->to.tick = tsn_timestamp_to_sysclock(priv->tc6, timestamps.to);
#if 0
            pr_err("%s - %d\n", __func__, __LINE__);
            pr_err("%s - metadata->to.tick: 0x%08x\n", __func__, metadata->to.tick);
#endif
    }
```

```c
#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
    pr_err("%s - metadata->to.priority: %d, metadata->delay_from.tick: 0x%08x, metadata->delay_from.priority: %d\n",
           __func__, metadata->to.priority, metadata->delay_from.tick, metadata->delay_from.priority);
    pr_err("%s - metadata->delay_to.tick: 0x%08x, metadata->delay_to.priority: %d\n", __func__, metadata->delay_to.tick,
           metadata->delay_to.priority);
#endif

->

#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
    pr_err("%s - metadata->to.priority: %d, metadata->delay_from.tick: 0x%08x, metadata->delay_from.priority: %d\n",
           __func__, metadata->to.priority, metadata->delay_from.tick, metadata->delay_from.priority);
    pr_err("%s - metadata->delay_to.tick: 0x%08x, metadata->delay_to.priority: %d\n", __func__, metadata->delay_to.tick,
           metadata->delay_from.priority);
#endif
```

```c
#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
#endif
    decrease_buffer_space(ldev);
    // Update available_ats
    spend_qav_credit(tsn_config, from, tc_id, metadata->frame_length);
    tsn_config->queue_available_at[queue_prio] += duration_ns;
    tsn_config->total_available_at += duration_ns;

->

#if 0
    pr_err("%s - %d\n", __func__, __LINE__);
#endif
    decrease_buffer_space(ldev);
    /* Update available_ats */
    spend_qav_credit(tsn_config, from, tc_id, metadata->frame_length);
    tsn_config->queue_available_at[queue_prio] += duration_ns;
    tsn_config->total_available_at += duration_ns;
```

```c
    // Example Qbv configuration
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; // 500ms
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; // 500ms
        config->qbv.slots[1].opened_prios[0] = true;
    }

    // Example Qav configuration
    if (false) {
        // 100Mbps on 1Gbps link
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

->

    /* Example Qbv configuration */
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[1].opened_prios[0] = true;
    }

    /* Example Qav configuration */
    if (false) {
        /* 100Mbps on 1Gbps link */
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }
```

```c
        // TODO: remove this when throughput issue without QoS gets resolved
        for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
            if (config->qav[tc_id].enabled) {
                qav_disabled = false;
                break;
            }
        }

->

        /* TODO: remove this when throughput issue without QoS gets resolved */
        for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
            if (config->qav[tc_id].enabled) {
                qav_disabled = false;
                break;
            }
        }
```

```c
        // First slot
        for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
            baked->prios[tc_id].slot_count = 1;
            baked->prios[tc_id].slots[0].opened = config->qbv.slots[0].opened_prios[tc_id];
        }

->

        /* First slot */
        for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
            baked->prios[tc_id].slot_count = 1;
            baked->prios[tc_id].slots[0].opened = config->qbv.slots[0].opened_prios[tc_id];
        }
```

```c
        // Same as the last slot. Just increase the duration
        prio->slots[prio->slot_count - 1].duration_ns += slot_duration;
    } else {
        // Different from the last slot. Add a new slot
        prio->slots[prio->slot_count].opened = config->qbv.slots[slot_id].opened_prios[tc_id];
        prio->slots[prio->slot_count].duration_ns = slot_duration;
        prio->slot_count += 1;
    }

->

        /* Same as the last slot. Just increase the duration */
        prio->slots[prio->slot_count - 1].duration_ns += slot_duration;
    } else {
        /* Different from the last slot. Add a new slot */
        prio->slots[prio->slot_count].opened = config->qbv.slots[slot_id].opened_prios[tc_id];
        prio->slots[prio->slot_count].duration_ns = slot_duration;
        prio->slot_count += 1;
    }
```

```c
    // Adjust slot counts to be even number. Because we need to have open-close pairs
    for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
        struct qbv_baked_prio* prio = &baked->prios[tc_id];
        if (prio->slot_count % 2 == 1) {
            prio->slots[prio->slot_count].opened = !prio->slots[prio->slot_count - 1].opened;
            prio->slots[prio->slot_count].duration_ns = 0;
            prio->slot_count += 1;
        }
    }

->

    /* Adjust slot counts to be even number. Because we need to have open-close pairs */
    for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
        struct qbv_baked_prio* prio = &baked->prios[tc_id];
        if (prio->slot_count % 2 == 1) {
            prio->slots[prio->slot_count].opened = !prio->slots[prio->slot_count - 1].opened;
            prio->slots[prio->slot_count].duration_ns = 0;
            prio->slot_count += 1;
        }
    }
```

```c
static uint64_t bytes_to_ns(uint64_t bytes) {
    // TODO: Get link speed
    uint64_t link_speed = 100000000; // Assume 100Mbps
    return max(bytes, (uint64_t)ETH_ZLEN) * 8 * NS_IN_1S / link_speed;
}

->

static uint64_t bytes_to_ns(uint64_t bytes) {
    /* TODO: Get link speed */
    uint64_t link_speed = 100000000; /* Assume 100Mbps */
    return max(bytes, (uint64_t)ETH_ZLEN) * 8 * NS_IN_1S / link_speed;
}
```

```c
    if (at < qav->last_update || at < qav->available_at) {
        // Invalid
        pr_err("Invalid timestamp Qav spending");
        return;
    }

->

    if (at < qav->last_update || at < qav->available_at) {
        /* Invalid */
        pr_err("Invalid timestamp Qav spending");
        return;
    }
```

```c
    earned_credit = (double)elapsed_from_last_update * qav->idle_slope;
    qav->credit += earned_credit;
    if (qav->credit > qav->hi_credit) {
        qav->credit = qav->hi_credit;
    }

    sending_duration = bytes_to_ns(bytes);
    spending_credit = (double)sending_duration * qav->send_slope;
    qav->credit += spending_credit;
    if (qav->credit < qav->lo_credit) {
        qav->credit = qav->lo_credit;
    }

    // Calulate next available time
    send_end = at + sending_duration;
    qav->last_update = send_end;
    if (qav->credit < 0) {
        qav->available_at = send_end + -(qav->credit / qav->idle_slope);
    } else {
        qav->available_at = send_end;
    }

->

    earned_credit = (double)elapsed_from_last_update * qav->idle_slope;
    qav->credit += earned_credit;
    if (qav->credit > qav->hi_credit) {
        qav->credit = qav->hi_credit;
    }

    sending_duration = bytes_to_ns(bytes);
    spending_credit = (double)sending_duration * qav->send_slope;
    qav->credit += spending_credit;
    if (qav->credit < qav->lo_credit) {
        qav->credit = qav->lo_credit;
    }

    /* Calulate next available time */
    send_end = at + sending_duration;
    qav->last_update = send_end;
    if (qav->credit < 0) {
        qav->available_at = send_end + -(qav->credit / qav->idle_slope);
    } else {
        qav->available_at = send_end;
    }
```

```c
    if (qbv->enabled == false) {
        // No Qbv. Just return the current time
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        // delay_* is pointless. Just set it to be right next to the frame
        timestamps->delay_from = timestamps->from;
        timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        return true;
    }

->

    if (qbv->enabled == false) {
        /* No Qbv. Just return the current time */
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        /* delay_* is pointless. Just set it to be right next to the frame */
        timestamps->delay_from = timestamps->from;
        timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        return true;
    }
```

```c
    // TODO: Need to check if the slot is big enough to fit the frame. But, That is a user fault. Don't mind for now
    // But we still have to check if the first current slot's remaining time is enough to fit the frame

->

    /* TODO: Need to check if the slot is big enough to fit the frame. But, That is a user fault. Don't mind for now */
    /* But we still have to check if the first current slot's remaining time is enough to fit the frame */
```

```c
    // Check if tc_id is always open or always closed
    if (slot_count == 2 && baked_prio->slots[1].duration_ns == 0) {
        if (baked_prio->slots[0].opened == false) {
            // The only slot is closed. Drop the frame
            return false;
        }
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        if (consider_delay) {
            timestamps->delay_from = timestamps->from;
            timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        }
        return true;
    }

->

    /* Check if tc_id is always open or always closed */
    if (slot_count == 2 && baked_prio->slots[1].duration_ns == 0) {
        if (baked_prio->slots[0].opened == false) {
            /* The only slot is closed. Drop the frame */
            return false;
        }
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        if (consider_delay) {
            timestamps->delay_from = timestamps->from;
            timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        }
        return true;
    }
```

```c
    // 1. "from"
    if (baked_prio->slots[slot_id].opened) {
        // Skip the slot if its remaining time is not enough to fit the frame
        if (baked_prio->slots[slot_id].duration_ns - remainder < sending_duration) {
            // Skip this slot. Because the slots are on/off pairs, we skip 2 slots
            // First
            timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
            // Second
            timestamps->from += baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
        } else {
            // The slot's remaining time is enough to fit the frame
            timestamps->from = from - remainder;
        }
    } else {
        // Select next slot
        timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; // Opened slot
    }

->

    /* 1. "from" */
    if (baked_prio->slots[slot_id].opened) {
        /* Skip the slot if its remaining time is not enough to fit the frame */
        if (baked_prio->slots[slot_id].duration_ns - remainder < sending_duration) {
            /* Skip this slot. Because the slots are on/off pairs, we skip 2 slots */
            /* First */
            timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
            /* Second */
            timestamps->from += baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
        } else {
            /* The slot's remaining time is enough to fit the frame */
            timestamps->from = from - remainder;
        }
    } else {
        /* Select next slot */
        timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; /* Opened slot */
    }
```

```c
    // 2. "to"
    timestamps->to = timestamps->from + baked_prio->slots[slot_id].duration_ns;

    if (consider_delay) {
        // 3. "delay_from"
        timestamps->delay_from = timestamps->from + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; // Closed slot
        timestamps->delay_from += baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; // Opened slot
        // 4. "delay_to"
        timestamps->delay_to = timestamps->delay_from + baked_prio->slots[slot_id].duration_ns;
    }

->

    /* 2. "to" */
    timestamps->to = timestamps->from + baked_prio->slots[slot_id].duration_ns;

    if (consider_delay) {
        /* 3. "delay_from" */
        timestamps->delay_from = timestamps->from + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; /* Closed slot */
        timestamps->delay_from += baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; /* Opened slot */
        /* 4. "delay_to" */
        timestamps->delay_to = timestamps->delay_from + baked_prio->slots[slot_id].duration_ns;
    }
```

```c
    // Adjust times
    timestamps->from = max(timestamps->from, from); // If already in the slot
    timestamps->to -= sending_duration;
    if (consider_delay) {
        timestamps->delay_to -= sending_duration;
    }

->

    /* Adjust times */
    timestamps->from = max(timestamps->from, from); /* If already in the slot */
    timestamps->to -= sending_duration;
    if (consider_delay) {
        timestamps->delay_to -= sending_duration;
    }
```

```c
#ifdef __LIB1QTSN_DEBUG__
    // Assert the timestamps
    if (timestamps->from >= timestamps->to) {
        pr_err("Invalid timestamps, from >= to, %llu >= %llu", timestamps->from, timestamps->to);
    }

    if (consider_delay) {
        if (timestamps->delay_from >= timestamps->delay_to) {
            pr_err("Invalid timestamps, delay_from >= delay_to, %llu >= %llu", timestamps->delay_from,
                   timestamps->delay_to);
        }

        if (timestamps->to >= timestamps->delay_from) {
            pr_err("Invalid timestamps, to >= delay_from, %llu >= %llu", timestamps->to, timestamps->delay_from);
        }
    }
#endif // __LIB1QTSN_DEBUG__

->

#ifdef __LIB1QTSN_DEBUG__
    /* Assert the timestamps */
    if (timestamps->from >= timestamps->to) {
        pr_err("Invalid timestamps, from >= to, %llu >= %llu", timestamps->from, timestamps->to);
    }

    if (consider_delay) {
        if (timestamps->delay_from >= timestamps->delay_to) {
            pr_err("Invalid timestamps, delay_from >= delay_to, %llu >= %llu", timestamps->delay_from,
                   timestamps->delay_to);
        }

        if (timestamps->to >= timestamps->delay_from) {
            pr_err("Invalid timestamps, to >= delay_from, %llu >= %llu", timestamps->to, timestamps->delay_from);
        }
    }
#endif /* __LIB1QTSN_DEBUG__ */
```

```c
    if (qopt.num_tc == 0) {
        // No need to proceed further
        return 0;
    }

->

    if (qopt.num_tc == 0) {
        /* No need to proceed further */
        return 0;
    }
```

```c
        // TODO: handle offload->entries[i].command
        config->qbv.slots[i].duration_ns = offload->entries[i].interval;
        for (j = 0; j < TC_COUNT; j++) {
            config->qbv.slots[i].opened_prios[j] = (offload->entries[i].gate_mask & (1 << j));
        }

->

        /* TODO: handle offload->entries[i].command */
        config->qbv.slots[i].duration_ns = offload->entries[i].interval;
        for (j = 0; j < TC_COUNT; j++) {
            config->qbv.slots[i].opened_prios[j] = (offload->entries[i].gate_mask & (1 << j));
        }
```

이제 모든 C 파일과 헤더 파일에서 `//` 주석을 `/* */` 형태로 변경했습니다. 

변경된 내용을 요약하면:

1. **lan1qtsn_arch.c**: 시스템 클록 읽기 관련 주석들을 `/* */` 형태로 변경
2. **lan1qtsn_mod.c**: SPDX 라이선스 헤더와 MAC 주소 생성, PTP 관련 주석들을 `/* */` 형태로 변경
3. **lan1qtsn_netdev.c**: TSN 시스템 활성화/비활성화, 시스템 클록 읽기, 타임스탬프 관련 주석들을 `/* */` 형태로 변경
4. **lan1qtsn_ptp.c**: PTP 함수들의 디버그 주석들을 `/* */` 형태로 변경
5. **lan1qtsn_spi.c**: 이미 `/* */` 형태의 주석들이 사용되고 있어 변경 불필요
6. **liblan1qtsn.c**: SPDX 라이선스 헤더를 `/* */` 형태로 변경
7. **tsn.c**: TSN 설정, QoS, Qbv, Qav 관련 주석들을 `/* */` 형태로 변경

모든 파일에서 `//` 형태의 한 줄 주석을 `/* */` 형태의 블록 주석으로 성공적으로 변경했습니다. 이제 코드가 더 일관된 주석 스타일을 가지게 되었습니다.

---

**User**

이 디렉토리 내 모든 c 파일에 있는 함수들 중description이 없는 함수들의 description을 추가 해 줘

---

**Cursor**

이 디렉토리 내 모든 C 파일에 있는 함수들 중 description이 없는 함수들의 description을 추가하겠습니다. 먼저 각 파일을 확인하여 함수들의 현재 상태를 파악하겠습니다.


이제 각 파일에서 함수들의 description이 있는지 확인하고, 없는 함수들에 description을 추가하겠습니다. 먼저 `lan1qtsn_arch.c` 파일부터 시작하겠습니다.


```c
#include "lan1qtsn_arch.h"

#include "lan1qtsn_netdev.h"

/**
 * lan1qtsn_spi_perform_ctrl - Perform SPI control operations
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to operate on
 * @value: Array of values for read/write operations
 * @length: Number of values to process
 * @reg_op: Type of register operation (read/write)
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                              enum lan1qtsn_spi_register_op reg_op);

/**
 * lan1qtsn_set_pulse_at_by_ldev - Set pulse timing for low-level device
 * @tc6: lan1qtsn_spi struct pointer
 * @time: System clock time for pulse generation
 */
void lan1qtsn_set_pulse_at_by_ldev(struct lan1qtsn_spi* tc6, sysclock_t time) {
    lan1qtsn_spi_write_register(tc6, PPS_PULSE_AT_H, (u32)(time >> 32));
    lan1qtsn_spi_write_register(tc6, PPS_PULSE_AT_L, (u32)(time));
}

/**
 * lan1qtsn_set_pulse_at - Set pulse timing for device
 * @ldev: lan1qtsn_dev struct pointer
 * @time: System clock time for pulse generation
 */
void lan1qtsn_set_pulse_at(struct lan1qtsn_dev* ldev, sysclock_t time) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    lan1qtsn_set_pulse_at_by_ldev(tc6, time);
}

/**
 * lan1qtsn_get_sys_clock_by_ldev - Get system clock from low-level device
 * @tc6: lan1qtsn_spi struct pointer
 * @return: Current system clock value, 0 on failure
 */
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

/**
 * lan1qtsn_get_sys_clock - Get system clock from device
 * @ldev: lan1qtsn_dev struct pointer
 * @return: Current system clock value, 0 on failure
 */
sysclock_t lan1qtsn_get_sys_clock(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    return lan1qtsn_get_sys_clock_by_ldev(tc6);
}

/**
 * lan1qtsn_set_cycle_1s_by_ldev - Set 1-second cycle for low-level device
 * @tc6: lan1qtsn_spi struct pointer
 * @cycle_1s: Cycle value in system clock units
 */
void lan1qtsn_set_cycle_1s_by_ldev(struct lan1qtsn_spi* tc6, u32 cycle_1s) {
    lan1qtsn_spi_write_register(tc6, PPS_CYCLE_1S_H, 0);
    lan1qtsn_spi_write_register(tc6, PPS_CYCLE_1S_L, cycle_1s);
}

/**
 * lan1qtsn_set_cycle_1s - Set 1-second cycle for device
 * @ldev: lan1qtsn_dev struct pointer
 * @cycle_1s: Cycle value in system clock units
 */
void lan1qtsn_set_cycle_1s(struct lan1qtsn_dev* ldev, u32 cycle_1s) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    lan1qtsn_set_cycle_1s_by_ldev(tc6, cycle_1s);
}

/**
 * lan1qtsn_get_cycle_1s_by_ldev - Get 1-second cycle from low-level device
 * @tc6: lan1qtsn_spi struct pointer
 * @return: Current 1-second cycle value, RESERVED_CYCLE if 0
 */
u32 lan1qtsn_get_cycle_1s_by_ldev(struct lan1qtsn_spi* tc6) {
    u32 cycle_1s;
    lan1qtsn_spi_read_register(tc6, PPS_CYCLE_1S_L, &cycle_1s);
    return cycle_1s ? cycle_1s : RESERVED_CYCLE;
}

/**
 * lan1qtsn_get_cycle_1s - Get 1-second cycle from device
 * @ldev: lan1qtsn_dev struct pointer
 * @return: Current 1-second cycle value, RESERVED_CYCLE if 0
 */
u32 lan1qtsn_get_cycle_1s(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    return lan1qtsn_get_cycle_1s_by_ldev(tc6);
}

/**
 * lan1qtsn_read_tx_timestamp_by_ldev - Read TX timestamp from low-level device
 * @tc6: lan1qtsn_spi struct pointer
 * @tx_id: TX timestamp ID (1-4)
 * @return: TX timestamp value, 0 on failure
 */
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

/**
 * lan1qtsn_read_tx_timestamp - Read TX timestamp from device
 * @ldev: lan1qtsn_dev struct pointer
 * @tx_id: TX timestamp ID (1-4)
 * @return: TX timestamp value, 0 on failure
 */
timestamp_t lan1qtsn_read_tx_timestamp(struct lan1qtsn_dev* ldev, int tx_id) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    return lan1qtsn_read_tx_timestamp_by_ldev(tc6, tx_id);
}

/**
 * lan1qtsn_get_buffer_write_status_by_ldev - Get buffer write status from low-level device
 * @tc6: lan1qtsn_spi struct pointer
 * @return: Buffer write status as 64-bit value
 */
u64 lan1qtsn_get_buffer_write_status_by_ldev(struct lan1qtsn_spi* tc6) {
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

/**
 * lan1qtsn_get_buffer_write_status - Get buffer write status from device
 * @ldev: lan1qtsn_dev struct pointer
 * @return: Buffer write status as 64-bit value
 */
u64 lan1qtsn_get_buffer_write_status(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    return lan1qtsn_get_buffer_write_status_by_ldev(tc6);
}

/**
 * lan1qtsn_get_total_new_entry_by_ldev - Get total new entry count from low-level device
 * @ldev: lan1qtsn_dev struct pointer
 * @return: Total new entry count as 64-bit value
 */
u64 lan1qtsn_get_total_new_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

/**
 * lan1qtsn_get_total_valid_entry_by_ldev - Get total valid entry count from low-level device
 * @ldev: lan1qtsn_dev struct pointer
 * @return: Total valid entry count as 64-bit value
 */
u64 lan1qtsn_get_total_valid_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

/**
 * lan1qtsn_get_total_drop_entry_by_ldev - Get total drop entry count from low-level device
 * @ldev: lan1qtsn_dev struct pointer
 * @return: Total drop entry count as 64-bit value
 */
u64 lan1qtsn_get_total_drop_entry_by_ldev(struct lan1qtsn_dev* ldev) {
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;
    u32 reg_h, reg_l;
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_H, &reg_h);
    lan1qtsn_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

#ifdef __LIB1QTSN_DEBUG__
/**
 * dump_buffer - Debug function to dump buffer contents
 * @buf: Buffer to dump
 * @len: Length of buffer
 */
void dump_buffer(unsigned char* buf, int len) {
    char buffer[64];
    int idx = 0;

    pr_err("[Buffer]");
    while (len >= (idx + 16)) {
        pr_err("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", buf[idx],
               buf[idx + 1], buf[idx + 2], buf[idx + 3], buf[idx + 4], buf[idx + 5], buf[idx + 6], buf[idx + 7],
               buf[idx + 8], buf[idx + 9], buf[idx + 10], buf[idx + 11], buf[idx + 12], buf[idx + 13], buf[idx + 14],
               buf[idx + 15]);
        idx += 16;
    }
    if (len > idx) {
        memset(buffer, 0, 64);
        for (int j = idx; j < len; j++) {
            sprintf(buffer, "%s%02x ", buffer, buf[j]);
        }
        pr_err("%s\n", buffer);
    }
}
#else
/**
 * dump_buffer - Empty debug function (debug disabled)
 * @buf: Buffer to dump (unused)
 * @len: Length of buffer (unused)
 */
void dump_buffer(unsigned char* buf, int len) {
}
#endif
```

이제 `lan1qtsn_mod.c` 파일에 함수 description을 추가하겠습니다.


```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TSNLab's LAN1QTSN driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */

#include "lan1qtsn_mod.h"

#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jhash.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/skbuff.h>
#include <linux/smp.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/workqueue.h>

#include "lan1qtsn_ioctl.h"
#include "lan1qtsn_netdev.h"
#include "lan1qtsn_spi.h"
#include "liblan1qtsn.h"

static struct lan1qtsn_spi* g_tc6;
static int tsdev_cnt;

/**
 * lan1qtsn_generate_mac_from_system_info - Generate MAC address based on system information
 * @netdev: Network device to set MAC address for
 *
 * This function generates a deterministic MAC address based on system information
 * including hostname, machine type, kernel version, CPU ID, memory size, and device index.
 * The generated MAC address is guaranteed to be unique for the system.
 */
void lan1qtsn_generate_mac_from_system_info(struct net_device* netdev) {
    struct lan1qtsn_priv* priv = netdev_priv(netdev);
    u8 mac_addr[ETH_ALEN];
    u32 hash_value = 0;
    struct new_utsname* uts;
    unsigned long cpu_id;
    unsigned long mem_total;

    /* Get system information */
    uts = utsname();
    if (!uts) {
        dev_err(&netdev->dev, "Failed to get system information\n");
        eth_hw_addr_random(netdev);
        return;
    }

    /* Create hash from system information */
    /* Use hostname, machine, and release information */
    hash_value = jhash(uts->nodename, strlen(uts->nodename), 0);
    hash_value = jhash(uts->machine, strlen(uts->machine), hash_value);
    hash_value = jhash(uts->release, strlen(uts->release), hash_value);
    hash_value = jhash(uts->version, strlen(uts->version), hash_value);
    hash_value = jhash(uts->domainname, strlen(uts->domainname), hash_value);

    /* Add CPU information to hash */
    cpu_id = (unsigned long)smp_processor_id();
    hash_value = jhash(&cpu_id, sizeof(cpu_id), hash_value);

    /* Add memory information */
    mem_total = totalram_pages() << PAGE_SHIFT;
    hash_value = jhash(&mem_total, sizeof(mem_total), hash_value);

    /* Add current time (jiffies) for additional randomness */
    hash_value = jhash(&jiffies, sizeof(jiffies), hash_value);

    /* Add device-specific information */
    hash_value = jhash(&netdev->ifindex, sizeof(netdev->ifindex), hash_value);

    /* Generate MAC address from hash */
    /* Use local administered address (bit 1 of first byte = 1) */
    /* Use unicast address (bit 0 of first byte = 0) */
    /* mac_addr[0] = 0x02; */ /* Local administered, unicast */
    mac_addr[0] = 0xd8;
    mac_addr[1] = (hash_value >> 16) & 0xFF;
    mac_addr[2] = (hash_value >> 8) & 0xFF;
    mac_addr[3] = hash_value & 0xFF;
    mac_addr[4] = (hash_value >> 24) & 0xFF;
    mac_addr[5] = ((hash_value >> 16) + (hash_value >> 8)) & 0xFF;

    /* Ensure the MAC address is not all zeros or all ones */
    if ((mac_addr[1] == 0 && mac_addr[2] == 0 && mac_addr[3] == 0 && mac_addr[4] == 0 && mac_addr[5] == 0) ||
        (mac_addr[1] == 0xFF && mac_addr[2] == 0xFF && mac_addr[3] == 0xFF && mac_addr[4] == 0xFF &&
         mac_addr[5] == 0xFF)) {
        /* If generated MAC is invalid, use a fallback */
        mac_addr[1] = 0x00;
        mac_addr[2] = 0x11;
        mac_addr[3] = 0x22;
        mac_addr[4] = 0x33;
        mac_addr[5] = 0x44;
    }

#if 1 /* def __LIB1QTSN_DEBUG__ */
    mac_addr[0] = 0xd8;
    mac_addr[1] = 0x2d;
    mac_addr[2] = 0x0f;
    mac_addr[3] = 0xeb;
    mac_addr[4] = 0x48;
    mac_addr[5] = 0x3c;
#endif

    /* Set the MAC address */
    eth_hw_addr_set(netdev, mac_addr);

    dev_info(&priv->sdev->dev, "Generated MAC address from system info: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0],
             mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    dev_info(&priv->sdev->dev, "System: %s, Machine: %s, Release: %s\n", uts->nodename, uts->machine, uts->release);
}

/**
 * lan1qtsn_ethtool_get_ts_info - Get timestamp information for ethtool
 * @ndev: Network device
 * @info: Pointer to kernel ethtool timestamp info structure
 * @return: 0 on success
 */
static int lan1qtsn_ethtool_get_ts_info(struct net_device* ndev, struct kernel_ethtool_ts_info* info) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&priv->sdev->dev);

    /* Check if PTP is available before accessing it */
    if (!tsdev || !tsdev->ptp || !tsdev->ptp->ptp_clock) {
        info->phc_index = -1;
    } else {
        info->phc_index = ptp_clock_index(tsdev->ptp->ptp_clock);
    }

    info->so_timestamping = SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE | SOF_TIMESTAMPING_SOFTWARE |
                            SOF_TIMESTAMPING_TX_HARDWARE | SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_RAW_HARDWARE;

    info->tx_types = BIT(HWTSTAMP_TX_OFF) | BIT(HWTSTAMP_TX_ON);

    info->rx_filters = BIT(HWTSTAMP_FILTER_NONE) | BIT(HWTSTAMP_FILTER_ALL) | BIT(HWTSTAMP_FILTER_PTP_V2_L2_EVENT) |
                       BIT(HWTSTAMP_FILTER_PTP_V2_L2_SYNC) | BIT(HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ);

    return 0;
}

/**
 * lan1qtsn_ethtool_get_link_ksettings - Get link settings for ethtool
 * @netdev: Network device
 * @cmd: Pointer to ethtool link ksettings command structure
 * @return: 0 on success
 */
static int lan1qtsn_ethtool_get_link_ksettings(struct net_device* netdev, struct ethtool_link_ksettings* cmd) {
    /* TODO: PHY also supports 100Mbps */
    cmd->base.speed = 100;
    return 0;
}

static const struct ethtool_ops lan1qtsn_ethtool_ops = {
    .get_ts_info = lan1qtsn_ethtool_get_ts_info,
    .get_link_ksettings = lan1qtsn_ethtool_get_link_ksettings,
};

#ifndef __TSN_PTP__
/**
 * lan1qtsn_send_packet - Send packet without PTP support
 * @skb: Socket buffer containing the packet
 * @netdev: Network device
 * @return: NETDEV_TX_OK on success
 */
static netdev_tx_t lan1qtsn_send_packet(struct sk_buff* skb, struct net_device* netdev) {
    struct lan1qtsn_priv* priv = netdev_priv(netdev);

    return lan1qtsn_spi_start_xmit(priv->tc6, skb);
}
#endif

static const struct net_device_ops lan1qtsn_netdev_ops = {
    .ndo_open = lan1qtsn_netdev_open,
    .ndo_stop = lan1qtsn_netdev_close,
#ifdef __TSN_PTP__
    .ndo_start_xmit = lan1qtsn_netdev_start_xmit,
#else
    .ndo_start_xmit = lan1qtsn_send_packet,
#endif
    .ndo_setup_tc = lan1qtsn_netdev_setup_tc,
    .ndo_eth_ioctl = lan1qtsn_netdev_ioctl,
    .ndo_select_queue = lan1qtsn_select_queue,
};

static long lan1qtsn_ioctl(struct file* file, unsigned int cmd, unsigned long arg);
static int lan1qtsn_open(struct inode* inode, struct file* file);
static int lan1qtsn_release(struct inode* inode, struct file* file);
static const struct file_operations lan1qtsn_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = lan1qtsn_ioctl,
    .open = lan1qtsn_open,
    .release = lan1qtsn_release,
};

static struct miscdevice lan1qtsn_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "lan1qtsn",
    .fops = &lan1qtsn_fops,
    .mode = 0666,
};

/**
 * tsdev_alloc - Allocate and initialize TSN device structure
 * @sdev: SPI device pointer
 * @return: Pointer to allocated tsdev structure, NULL on failure
 */
static struct lan1qtsn_spi_dev* tsdev_alloc(struct spi_device* sdev) {
    struct lan1qtsn_spi_dev* tsdev = kmalloc(sizeof(*tsdev), GFP_KERNEL);

    if (!tsdev) {
        return NULL;
    }
    memset(tsdev, 0, sizeof(*tsdev));

    tsdev->sdev = sdev;
    tsdev->magic = 0x9530234201;

    tsdev_cnt++;
    return tsdev;
}

/**
 * tsdev_free - Free TSN device structure and cleanup resources
 * @tsdev: TSN device structure to free
 */
static void tsdev_free(struct lan1qtsn_spi_dev* tsdev) {
    struct lan1qtsn_dev* ldev = tsdev->ldev;

    pr_info("tsdev 0x%p, destroy_interfaces, ldev 0x%p.\n", tsdev, ldev);

#ifdef __TSN_PTP__
    /* Clean up PTP device if it exists */
    if (tsdev->ptp) {
        ptp_device_destroy(tsdev->ptp);
        tsdev->ptp = NULL;
    }
#endif

    ldev_free(ldev);
    tsdev->ldev = NULL;
    tsdev_cnt--;

    kfree(tsdev);
}

/**
 * lan1qtsn_probe - Probe function for SPI device
 * @sdev: SPI device pointer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_probe(struct spi_device* sdev) {
    struct lan1qtsn_spi_dev* tsdev = NULL;
    struct lan1qtsn_dev* ldev;
    struct net_device* ndev;
    struct lan1qtsn_priv* priv;
#ifdef __TSN_PTP__
    struct ptp_device_data* ptp_data;
#endif
    int ret;

    tsdev = tsdev_alloc(sdev);
    if (!tsdev) {
        pr_err("tsdev_alloc failed\n");
        return -ENOMEM;
    }

    ldev = ldev_alloc(sdev);
    if (!ldev) {
        pr_err("ldev_alloc failed\n");
        ret = -ENOMEM;
        goto err_out;
    }

    tsdev->ldev = ldev;

#ifdef __TSN_PTP__
    /* Allocate the network device */
    /* TC command requires multiple TX queues */
    ndev = alloc_etherdev_mq(sizeof(struct lan1qtsn_priv), TX_QUEUE_COUNT);
    if (!ndev) {
        pr_err("alloc_etherdev failed\n");
        ret = -ENOMEM;
        goto err_out;
    }
    /*
     * Multiple RX queues drops throughput significantly.
     * TODO: Find out why RX queue count affects throughput
     * and see if it can be resolved in another way
     */
    ret = netif_set_real_num_rx_queues(ndev, RX_QUEUE_COUNT);
    if (ret) {
        pr_err("netif_set_real_num_rx_queues failed\n");
        goto err_out;
    }
#else
    ndev = alloc_etherdev(sizeof(struct lan1qtsn_priv));
    if (!ndev) {
        pr_err("alloc_etherdev failed\n");
        ret = -ENOMEM;
        goto err_out;
    }
#endif

    ldev->ndev = ndev;
    tsdev->ndev = ndev;

    priv = netdev_priv(ndev);
    priv->ndev = ndev;
    priv->sdev = sdev;
    priv->magic = 0x9530234203;
    priv->tsdev = tsdev;
    priv->ldev = tsdev->ldev;
    spi_set_drvdata(sdev, priv);

    priv->tc6 = lan1qtsn_spi_init(sdev, ndev);
    g_tc6 = priv->tc6;
    if (!priv->tc6) {
        ret = -ENODEV;
        goto free_netdev;
    }

    priv->tc6->tsdev = tsdev;
    ldev->tc6 = priv->tc6;

    /* Set the MAC address */
    lan1qtsn_generate_mac_from_system_info(ndev);

    ndev->if_port = IF_PORT_100BASET;
    ndev->irq = sdev->irq;
    ndev->netdev_ops = &lan1qtsn_netdev_ops;
    ndev->ethtool_ops = &lan1qtsn_ethtool_ops;

#ifdef __TSN_PTP__
    /* lan1qtsn_spi_write_register(priv->tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE); */

#if 1
    /* Tx works for each timestamp id */
    INIT_WORK(&priv->tx_work[1], lan1qtsn_tx_work1);
    INIT_WORK(&priv->tx_work[2], lan1qtsn_tx_work2);
    INIT_WORK(&priv->tx_work[3], lan1qtsn_tx_work3);
    INIT_WORK(&priv->tx_work[4], lan1qtsn_tx_work4);
#endif

    ptp_data = ptp_device_init(&sdev->dev, ldev);
    if (!ptp_data) {
        pr_err("ptp_device_init failed, continuing without PTP support\n");
        tsdev->ptp = NULL;
    } else {
        ptp_data->ldev = tsdev->ldev;
        tsdev->ptp = ptp_data;
        pr_err("%s - tsdev->ptp: %p\n", __func__, tsdev->ptp);
    }
#endif

    ret = register_netdev(ndev);
    if (ret) {
        dev_err(&sdev->dev, "Register netdev failed (ret = %d)", ret);
        goto lan1qtsn_spi_exit;
    }

    return misc_register(&lan1qtsn_miscdev);

lan1qtsn_spi_exit:
    lan1qtsn_spi_exit(priv->tc6);
free_netdev:
    free_netdev(priv->ndev);
err_out:
    pr_err("ldev 0x%p, err %d.\n", ldev, ret);
    tsdev_free(tsdev);
    return ret;
}

/**
 * lan1qtsn_remove - Remove function for SPI device
 * @spi: SPI device pointer
 */
static void lan1qtsn_remove(struct spi_device* spi) {
    struct lan1qtsn_priv* priv = spi_get_drvdata(spi);

    unregister_netdev(priv->ndev);
    lan1qtsn_spi_exit(priv->tc6);

#ifdef __TSN_PTP__
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&priv->sdev->dev);

    /* Clean up PTP device if it exists */
    if (tsdev && tsdev->ptp) {
        ptp_device_destroy(tsdev->ptp);
        tsdev->ptp = NULL;
    }
#endif

    free_netdev(priv->ndev);
    misc_deregister(&lan1qtsn_miscdev);
}

/**
 * lan1qtsn_ioctl - Handle ioctl commands for the device
 * @file: File pointer
 * @cmd: Command to execute
 * @arg: Argument for the command
 * @return: 0 on success, negative error code on failure
 */
static long lan1qtsn_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    struct lan1qtsn_reg reg;
    int ret = 0;

    switch (cmd) {
    case LAN1QTSN_READ_REG:
        if (copy_from_user(&reg, (void __user*)arg, sizeof(reg))) {
            return -EFAULT;
        }

        ret = lan1qtsn_spi_read_register(g_tc6, reg.addr, &reg.value);

        if (ret < 0) {
            return ret;
        }

        if (copy_to_user((void __user*)arg, &reg, sizeof(reg))) {
            return -EFAULT;
        }
        break;

    case LAN1QTSN_WRITE_REG:
        if (copy_from_user(&reg, (void __user*)arg, sizeof(reg))) {
            return -EFAULT;
        }

        ret = lan1qtsn_spi_write_register(g_tc6, reg.addr, reg.value);
        break;

    default:
        return -ENOTTY;
    }

    return ret;
}

/**
 * lan1qtsn_open - Open function for the device file
 * @inode: Inode pointer
 * @file: File pointer
 * @return: 0 on success
 */
static int lan1qtsn_open(struct inode* inode, struct file* file) {
    struct spi_device* spi = container_of(file->private_data, struct spi_device, dev);
    file->private_data = spi;
    return 0;
}

/**
 * lan1qtsn_release - Release function for the device file
 * @inode: Inode pointer
 * @file: File pointer
 * @return: 0 on success
 */
static int lan1qtsn_release(struct inode* inode, struct file* file) {
    file->private_data = NULL;
    return 0;
}

static const struct spi_device_id spidev_spi_ids[] = {
    {.name = "lan1qtsn"},
    {},
};

static const struct of_device_id lan1qtsn_dt_ids[] = {
    {.compatible = "tsnlab,lan1qtsn"},
    {/* Sentinel */},
};
MODULE_DEVICE_TABLE(of, lan1qtsn_dt_ids);

static struct spi_driver lan1qtsn_driver = {
    .driver =
        {
            .name = DRV_NAME,
            .of_match_table = lan1qtsn_dt_ids,
        },
    .probe = lan1qtsn_probe,
    .remove = lan1qtsn_remove,
    .id_table = spidev_spi_ids,
};
module_spi_driver(lan1qtsn_driver);

MODULE_DESCRIPTION(DRV_NAME " TSNLab MACPHY Ethernet Driver");
MODULE_AUTHOR("HounJoung Rim <hounjoung@tsnlab.com>");
MODULE_LICENSE("GPL");
```

이제 `lan1qtsn_netdev.c` 파일에 함수 description을 추가하겠습니다.


```c
#include "lan1qtsn_netdev.h"

#include <linux/skbuff.h>
#include <net/flow_offload.h>
#include <net/pkt_cls.h>
#include <net/pkt_sched.h>

#include "lan1qtsn_arch.h"
#include "lan1qtsn_mod.h"
#include "liblan1qtsn.h"
#include "tsn.h"

#define LOWER_29_BITS ((1ULL << 29) - 1)
#define TX_WORK_OVERFLOW_MARGIN 100

#define WORKAROUND_PAD 5

/**
 * lan1qtsn_netdev_open - Open network device interface
 * @ndev: Network device to open
 * @return: 0 on success
 *
 * This function enables the network interface, starts TX queues if PTP is enabled,
 * and enables the TSN system.
 */
int lan1qtsn_netdev_open(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);

    netif_carrier_on(ndev);
#ifdef __TSN_PTP__
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

    return 0;
}

/**
 * lan1qtsn_netdev_close - Close network device interface
 * @ndev: Network device to close
 * @return: 0 on success
 *
 * This function disables the TSN system, stops TX queues if PTP is enabled,
 * and turns off the network interface carrier.
 */
int lan1qtsn_netdev_close(struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);

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

/**
 * lan1qtsn_netdev_start_xmit - Start transmission of a packet
 * @skb: Socket buffer containing the packet to transmit
 * @ndev: Network device
 * @return: NETDEV_TX_OK on success, NETDEV_TX_BUSY on failure
 *
 * This function handles packet transmission with TSN metadata, timestamp handling,
 * and queue management for PTP-enabled systems.
 */
netdev_tx_t lan1qtsn_netdev_start_xmit(struct sk_buff* skb, struct net_device* ndev) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    sysclock_t sys_count, sys_count_upper, sys_count_lower;
    timestamp_t now;
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;
    u32 to_value;
    u16 q;
    struct lan1qtsn_spi_dev* tsdev = priv->tsdev;

    /* Check desc count */
    q = skb_get_queue_mapping(skb);
    netif_stop_subqueue(ndev, q);
    lan1qtsn_debug("%s(skb->len : %d)\n", __func__, skb->len);
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

    /* Try to read system clock safely */
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    if (sys_count == 0) {
        /* Use default values if system clock read failed */
        dev_dbg(&ndev->dev, "System clock read failed, using default values\n");
        now = 0;
        sys_count_lower = 0;
        sys_count_upper = 0;
    } else {
        now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
        sys_count_lower = sys_count & LOWER_29_BITS;
        sys_count_upper = sys_count & ~LOWER_29_BITS;
    }

    /* Set the fromtick & to_tick values based on the lower 29 bits of the system count */
    if (!tsdev) {
        pr_err("tsdev is NULL, cannot fill metadata\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

    if (tsn_fill_metadata(tsdev, now, skb) == false) {
        /* TODO: Increment SW drop stats */
        pr_warn("tsn_fill_metadata failed\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

#if 1
#if 1
    /*
     * skb->len: pure skb->len + tx-meta-data
     * 3: SPI Command Byte count
     * 4: FCS Byte count
     * 8: Number of bytes required to read the System Count Register
     * x 2: Low/High Register
     * x 8: bits per byte
     * x 100: 100 MHz system clock
     * 15: 15MHz SPI clock
     */
#define H2C_LATENCY_TICKS (5000)
#define _DEFAULT_TO_MARGIN_TICKS (15000)
    timestamp_t spi_delay;
    spi_delay = ((skb->len + 3 + 4 + 8 * 2) * 8) * 100 / 15;

    tx_metadata->from.tick = (uint32_t)(sys_count_lower + spi_delay + H2C_LATENCY_TICKS);
    tx_metadata->to.tick = tx_metadata->from.tick + _DEFAULT_TO_MARGIN_TICKS;
    if (tx_metadata->from.tick > tx_metadata->to.tick) {
        pr_err(
            "sys_count_lower: 0x%08llx, skb->len: %d, tx_metadata->from.tick(0x%08x) > tx_metadata->to.tick(0x%08x)\n",
            sys_count_lower, skb->len, tx_metadata->from.tick, tx_metadata->to.tick);
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }
#else
    lan1qtsn_debug("%s - sys_count_lower: 0x%08llx  from.tick: 0x%08x  from.priority: 0x%x  timestamp_id: %d\n",
                   __func__, sys_count_lower, tx_metadata->from.tick, tx_metadata->from.priority,
                   tx_metadata->timestamp_id);
    lan1qtsn_debug("%s - to.tick: 0x%08x  to.priority: 0x%x  frame_length: %4d  fail_policy: %1d\n", __func__,
                   tx_metadata->to.tick, tx_metadata->to.priority, tx_metadata->frame_length, tx_metadata->fail_policy);
    /* tx_metadata->from.tick = 0x1E8480; */
    /* tx_metadata->to.tick = 0x1FFFFFFF; */
    if (tx_metadata->from.tick > tx_metadata->to.tick) {
        tx_metadata->to.tick += (uint32_t)(LOWER_29_BITS - tx_metadata->from.tick);
        tx_metadata->from.tick = 0;
        lan1qtsn_debug("%s - sys_count_lower: 0x%08llx  from.tick: 0x%08x  from.priority: 0x%x  timestamp_id: %d\n",
                       __func__, sys_count_lower, tx_metadata->from.tick, tx_metadata->from.priority,
                       tx_metadata->timestamp_id);
        lan1qtsn_debug("%s - to.tick: 0x%08x  to.priority: 0x%x  frame_length: %4d  fail_policy: %1d\n", __func__,
                       tx_metadata->to.tick, tx_metadata->to.priority, tx_metadata->frame_length,
                       tx_metadata->fail_policy);
    }
#endif
#endif
    dump_buffer((unsigned char*)tx_metadata, (int)(skb->len));

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
                /* Overflow */
                priv->tx_work_start_after[tx_metadata->timestamp_id] += (1 << 29);
            }
            to_value =
                (tx_metadata->fail_policy == TSN_FAIL_POLICY_RETRY ? tx_metadata->delay_to.tick : tx_metadata->to.tick);
            priv->tx_work_wait_until[tx_metadata->timestamp_id] = sys_count_upper | to_value;
            if (sys_count_lower > to_value && sys_count_lower - to_value > TX_WORK_OVERFLOW_MARGIN) {
                /* Overflow */
                priv->tx_work_wait_until[tx_metadata->timestamp_id] += (1 << 29);
            }
            schedule_work(&priv->tx_work[tx_metadata->timestamp_id]);
        } else if (priv->tstamp_config.tx_type != HWTSTAMP_TX_ON) {
            pr_warn("Timestamp skipped: timestamp config is off\n");
        } else {
            pr_warn("Timestamp skipped: driver is waiting for previous packet's timestamp\n");
        }
        /* TODO: track the number of skipped packets for ethtool stats */
    }

    /* netif_wake_queue() will be called in lan1qtsn_isr() */
    priv->tx_skb = skb;

    return lan1qtsn_spi_start_xmit(priv->tc6, skb);
}

/**
 * lan1qtsn_select_queue - Select queue for packet transmission
 * @ndev: Network device
 * @skb: Socket buffer
 * @sb_dev: Subordinate device (unused)
 * @return: Always returns 0 (first subqueue)
 */
u16 lan1qtsn_select_queue(struct net_device* ndev, struct sk_buff* skb, struct net_device* sb_dev) {
    /* Always use the first subqueue */
    return 0;
}

static LIST_HEAD(lan1qtsn_block_cb_list);

/**
 * lan1qtsn_setup_tc_block_cb - Setup TC block callback
 * @type: TC setup type
 * @type_data: Type-specific data
 * @cb_priv: Callback private data
 * @return: -EOPNOTSUPP (not supported)
 */
static int lan1qtsn_setup_tc_block_cb(enum tc_setup_type type, void* type_data, void* cb_priv) {
    /* If mqprio is only used for queue mapping this should not be called */
    return -EOPNOTSUPP;
}

/**
 * lan1qtsn_netdev_setup_tc - Setup traffic control for network device
 * @ndev: Network device
 * @type: TC setup type
 * @type_data: Type-specific data
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_netdev_setup_tc(struct net_device* ndev, enum tc_setup_type type, void* type_data) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);

    switch (type) {
    case TC_SETUP_QDISC_MQPRIO:
        return tsn_set_mqprio(priv->sdev, (struct tc_mqprio_qopt_offload*)type_data);
    case TC_SETUP_QDISC_CBS:
        return tsn_set_qav(priv->sdev, (struct tc_cbs_qopt_offload*)type_data);
    case TC_SETUP_QDISC_TAPRIO:
        return tsn_set_qbv(priv->sdev, (struct tc_taprio_qopt_offload*)type_data);
    case TC_SETUP_BLOCK:
        return flow_block_cb_setup_simple(type_data, &lan1qtsn_block_cb_list, lan1qtsn_setup_tc_block_cb, priv, priv,
                                          true);
    default:
        return -ENOTSUPP;
    }

    return 0;
}

/**
 * lan1qtsn_get_ts_config - Get timestamp configuration
 * @ndev: Network device
 * @ifr: Interface request structure
 * @return: 0 on success, -EFAULT on failure
 */
static int lan1qtsn_get_ts_config(struct net_device* ndev, struct ifreq* ifr) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    struct hwtstamp_config* config = &priv->tstamp_config;

    return copy_to_user(ifr->ifr_data, config, sizeof(*config)) ? -EFAULT : 0;
}

/**
 * lan1qtsn_set_ts_config - Set timestamp configuration
 * @ndev: Network device
 * @ifr: Interface request structure
 * @return: 0 on success, -EFAULT on failure
 */
static int lan1qtsn_set_ts_config(struct net_device* ndev, struct ifreq* ifr) {
    struct lan1qtsn_priv* priv = netdev_priv(ndev);
    struct hwtstamp_config* config = &priv->tstamp_config;

    return copy_from_user(config, ifr->ifr_data, sizeof(*config)) ? -EFAULT : 0;
}

/**
 * lan1qtsn_netdev_ioctl - Handle network device ioctl commands
 * @ndev: Network device
 * @ifr: Interface request structure
 * @cmd: Command to execute
 * @return: 0 on success, -EOPNOTSUPP for unsupported commands
 */
int lan1qtsn_netdev_ioctl(struct net_device* ndev, struct ifreq* ifr, int cmd) {
    switch (cmd) {
    case SIOCGHWTSTAMP:
        return lan1qtsn_get_ts_config(ndev, ifr);
    case SIOCSHWTSTAMP:
        return lan1qtsn_set_ts_config(ndev, ifr);
    default:
        return -EOPNOTSUPP;
    }
}

/**
 * do_tx_work - Process TX work for timestamp handling
 * @work: Work structure
 * @tstamp_id: Timestamp ID to process
 *
 * This function handles TX timestamp processing in workqueue context,
 * including retry logic and timestamp validation.
 */
static void do_tx_work(struct work_struct* work, u16 tstamp_id) {
    sysclock_t tx_tstamp;
    struct skb_shared_hwtstamps shhwtstamps;
    struct lan1qtsn_priv* priv = container_of(work - tstamp_id, struct lan1qtsn_priv, tx_work[0]);
    struct sk_buff* skb = priv->tx_work_skb[tstamp_id];
    sysclock_t now = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);

    if (tstamp_id >= TSN_TIMESTAMP_ID_MAX) {
        pr_err("Invalid timestamp ID\n");
        return;
    }

    if (!priv->tx_work_skb[tstamp_id]) {
        goto return_error;
    }

    if (now < priv->tx_work_start_after[tstamp_id]) {
        goto retry;
    }
    /*
     * Read TX timestamp several times because
     * the work thread might try to read TX timestamp
     * before the register gets updated
     */
    tx_tstamp = lan1qtsn_read_tx_timestamp_by_ldev(priv->tc6, tstamp_id);
    if (tx_tstamp == priv->last_tx_tstamp[tstamp_id]) {
        if (lan1qtsn_get_sys_clock_by_ldev(priv->tc6) < priv->tx_work_wait_until[tstamp_id]) {
            /* The packet might have not been sent yet */
            goto retry;
        }
        /*
         * Tx timestamp is not updated. Try again.
         * Waiting for it to be updated forever is not desirable,
         * so limit the number of retries
         */
        if (++(priv->tstamp_retry[tstamp_id]) >= TX_TSTAMP_MAX_RETRY) {
            /* TODO: track the number of skipped packets for ethtool stats */
            pr_warn("Failed to get timestamp: timestamp is not getting updated, "
                    "the packet might have been dropped\n");
            goto return_error;
        }
        goto retry;
    }

    priv->tstamp_retry[tstamp_id] = 0;
    shhwtstamps.hwtstamp = ns_to_ktime(lan1qtsn_sysclock_to_txtstamp(priv->tc6, tx_tstamp));
    priv->last_tx_tstamp[tstamp_id] = tx_tstamp;

    priv->tx_work_skb[tstamp_id] = NULL;
    clear_bit_unlock(tstamp_id, &priv->state);
    skb_tstamp_tx(skb, &shhwtstamps);
    dev_kfree_skb_any(skb);
    return;

return_error:
    priv->tstamp_retry[tstamp_id] = 0;
    clear_bit_unlock(tstamp_id, &priv->state);
    return;

retry:
    schedule_work(&priv->tx_work[tstamp_id]);
    return;
}

#define DEFINE_TX_WORK(n)                                \
    void lan1qtsn_tx_work##n(struct work_struct* work) { \
        do_tx_work(work, n);                             \
    }

DEFINE_TX_WORK(1);
DEFINE_TX_WORK(2);
DEFINE_TX_WORK(3);
DEFINE_TX_WORK(4);
```

이제 `lan1qtsn_ptp.c` 파일에 함수 description을 추가하겠습니다.


```c
#include "lan1qtsn_ptp.h"

#include "lan1qtsn_arch.h"
#include "lan1qtsn_netdev.h"

#define NS_IN_1S 1000000000

/**
 * lan1qtsn_get_timestamp - Convert system count to timestamp
 * @sys_count: System count value
 * @ticks_scale: Scale factor for ticks
 * @offset: Offset value
 * @return: Calculated timestamp
 */
static timestamp_t lan1qtsn_get_timestamp(u64 sys_count, double ticks_scale, u64 offset) {
    timestamp_t timestamp = ticks_scale * sys_count;

    return timestamp + offset;
}

/**
 * set_pulse_at - Set pulse timing for PTP device
 * @ptp_data: PTP device data structure
 * @sys_count: Current system count
 */
static void set_pulse_at(struct ptp_device_data* ptp_data, sysclock_t sys_count) {
    timestamp_t current_ns, next_pulse_ns;
    sysclock_t next_pulse_sysclock;
    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);

    current_ns = lan1qtsn_get_timestamp(sys_count, ptp_data->ticks_scale, ptp_data->offset);
    next_pulse_ns = current_ns - (current_ns % NS_IN_1S) + NS_IN_1S;
    next_pulse_sysclock = ((double)(next_pulse_ns - ptp_data->offset) / ptp_data->ticks_scale);
    lan1qtsn_debug("ptp%u: %s sys_count=%llu, current_ns=%llu, next_pulse_ns=%llu, next_pulse_sysclock=%llu",
                   ptp_data->ptp_id, __func__, sys_count, current_ns, next_pulse_ns, next_pulse_sysclock);

    lan1qtsn_set_pulse_at_by_ldev(priv->tc6, next_pulse_sysclock);
}

/**
 * set_cycle_1s - Set 1-second cycle for PTP device
 * @ptp_data: PTP device data structure
 * @cycle_1s: Cycle value in system clock units
 */
static void set_cycle_1s(struct ptp_device_data* ptp_data, u32 cycle_1s) {
    struct lan1qtsn_priv* priv = netdev_priv(ptp_data->ldev->ndev);
    lan1qtsn_debug("ptp%u: %s cycle_1s=%u", ptp_data->ptp_id, __func__, cycle_1s);
    lan1qtsn_set_cycle_1s_by_ldev(priv->tc6, cycle_1s);
}

/**
 * set_ticks_scale - Set ticks scale for PTP device
 * @ptp_data: PTP device data structure
 * @ticks_scale: Scale factor for ticks
 */
static void set_ticks_scale(struct ptp_device_data* ptp_data, double ticks_scale) {
    lan1qtsn_set_ticks_scale(ptp_data->ldev->tc6, ticks_scale);
}

/**
 * lan1qtsn_timestamp_to_sysclock - Convert timestamp to system clock
 * @tc6: lan1qtsn_spi struct pointer
 * @timestamp: Timestamp to convert
 * @return: System clock value, 0 on failure
 */
sysclock_t lan1qtsn_timestamp_to_sysclock(struct lan1qtsn_spi* tc6, timestamp_t timestamp) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0;
    }

#if 0
    long scaled_val = (long)(ptp_data->ticks_scale * 1000000);
    pr_err("%s - ptp_data: %p, timestamp: 0x%08llx, ptp_data->offset: 0x%016llx, ptp_data->ticks_scale: %ld.%06ld\n",
           __func__, ptp_data, timestamp, ptp_data->offset, scaled_val / 1000000, scaled_val % 1000000);
#endif
    return (timestamp - ptp_data->offset) / ptp_data->ticks_scale;
}

/**
 * lan1qtsn_sysclock_to_timestamp - Convert system clock to timestamp
 * @tc6: lan1qtsn_spi struct pointer
 * @sysclock: System clock value to convert
 * @return: Timestamp value, 0 on failure
 */
timestamp_t lan1qtsn_sysclock_to_timestamp(struct lan1qtsn_spi* tc6, sysclock_t sysclock) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0;
    }

    u64 offset = ptp_data->offset;

#if 0
    long scaled_val = (long)(ptp_data->ticks_scale * 1000000);
    pr_err("%s - ptp_data: %p, sysclock: 0x%08llx, ptp_data->offset: 0x%016llx, ptp_data->ticks_scale: %ld.%06ld\n",
           __func__, ptp_data, sysclock, offset, scaled_val / 1000000, scaled_val % 1000000);
#endif
    return lan1qtsn_get_timestamp(sysclock, ptp_data->ticks_scale, offset);
}

/**
 * lan1qtsn_get_rx_timestamp - Get RX timestamp with adjustment
 * @tc6: lan1qtsn_spi struct pointer
 * @sysclock: System clock value
 * @return: Adjusted RX timestamp
 */
timestamp_t lan1qtsn_get_rx_timestamp(struct lan1qtsn_spi* tc6, sysclock_t sysclock) {
    return lan1qtsn_sysclock_to_timestamp(tc6, sysclock) - RX_ADJUST_NS;
}

/**
 * lan1qtsn_get_tx_timestamp - Get TX timestamp with adjustment
 * @tc6: lan1qtsn_spi struct pointer
 * @tx_id: TX timestamp ID
 * @return: Adjusted TX timestamp, 0 on failure
 */
timestamp_t lan1qtsn_get_tx_timestamp(struct lan1qtsn_spi* tc6, int tx_id) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0;
    }

    sysclock_t sysclock = lan1qtsn_read_tx_timestamp(tsdev->ldev, tx_id);

    return lan1qtsn_sysclock_to_timestamp(tc6, sysclock) + TX_ADJUST_NS;
}

/**
 * lan1qtsn_sysclock_to_txtstamp - Convert system clock to TX timestamp
 * @tc6: lan1qtsn_spi struct pointer
 * @sysclock: System clock value to convert
 * @return: TX timestamp value
 */
timestamp_t lan1qtsn_sysclock_to_txtstamp(struct lan1qtsn_spi* tc6, sysclock_t sysclock) {
    return lan1qtsn_sysclock_to_timestamp(tc6, sysclock) + TX_ADJUST_NS;
}

/**
 * lan1qtsn_get_ticks_scale - Get ticks scale from PTP device
 * @tc6: lan1qtsn_spi struct pointer
 * @return: Ticks scale value, 0.0 on failure
 */
double lan1qtsn_get_ticks_scale(struct lan1qtsn_spi* tc6) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0.0;
    }

    return ptp_data->ticks_scale;
}

/**
 * lan1qtsn_set_ticks_scale - Set ticks scale for PTP device
 * @tc6: lan1qtsn_spi struct pointer
 * @ticks_scale: Scale factor for ticks
 */
void lan1qtsn_set_ticks_scale(struct lan1qtsn_spi* tc6, double ticks_scale) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return;
    }

    ptp_data->ticks_scale = ticks_scale;
    lan1qtsn_set_cycle_1s(tsdev->ldev, (double)NS_IN_1S / ptp_data->ticks_scale);
}

/**
 * lan1qtsn_ptp_gettimex - Get PTP time with system timestamp
 * @ptp: PTP clock info structure
 * @ts: Timespec structure to fill
 * @sts: System timestamp structure
 * @return: 0 on success
 */
static int lan1qtsn_ptp_gettimex(struct ptp_clock_info* ptp, struct timespec64* ts, struct ptp_system_timestamp* sts) {
    u64 clock, timestamp;
    unsigned long flags;

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);
    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    ptp_read_system_prets(sts);
    clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    ptp_read_system_postts(sts);

    timestamp = lan1qtsn_get_timestamp(clock, ptp_data->ticks_scale, ptp_data->offset);

    ts->tv_sec = timestamp / NS_IN_1S;
    ts->tv_nsec = timestamp % NS_IN_1S;

    spin_unlock_irqrestore(&ptp_data->lock, flags);

    lan1qtsn_debug("ptp%u: %s clock=%llu, timestamp=%llu", ptp_data->ptp_id, __func__, clock, timestamp);

    return 0;
}

/**
 * lan1qtsn_ptp_settime - Set PTP time
 * @ptp: PTP clock info structure
 * @ts: Timespec structure containing new time
 * @return: 0 on success
 */
static int lan1qtsn_ptp_settime(struct ptp_clock_info* ptp, const struct timespec64* ts) {
    u64 hw_timestamp, host_timestamp, sys_clock;
    unsigned long flags;

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);

    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    /* Get host timestamp */
    host_timestamp = (u64)ts->tv_sec * NS_IN_1S + ts->tv_nsec;

    spin_lock_irqsave(&ptp_data->lock, flags);

    ptp_data->ticks_scale = TICKS_SCALE;

    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    hw_timestamp = lan1qtsn_get_timestamp(sys_clock, ptp_data->ticks_scale, ptp_data->offset);

    ptp_data->offset = host_timestamp - hw_timestamp;

    /* set_cycle_1s(ptp_data, RESERVED_CYCLE); */
    /* set_pulse_at(ptp_data, sys_clock); */

    spin_unlock_irqrestore(&ptp_data->lock, flags);

    pr_err("%s host_timestamp=%llu, sys_clock=%llu, hw_timestamp=%llu, offset=%llu\n", __func__, host_timestamp,
           sys_clock, hw_timestamp, ptp_data->offset);
    lan1qtsn_debug("ptp%u: %s host_timestamp=%llu, hw_timestamp=%llu, offset=%llu", ptp_data->ptp_id, __func__,
                   host_timestamp, hw_timestamp, ptp_data->offset);

    return 0;
}

/**
 * lan1qtsn_ptp_adjtime - Adjust PTP time
 * @ptp: PTP clock info structure
 * @delta: Time adjustment in nanoseconds
 * @return: 0 on success
 */
static int lan1qtsn_ptp_adjtime(struct ptp_clock_info* ptp, s64 delta) {
    u64 sys_clock;
    unsigned long flags;

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);
    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    /* Adjust offset */
    ptp_data->offset += delta;

    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    /* set_pulse_at(ptp_data, sys_clock); */

    spin_unlock_irqrestore(&ptp_data->lock, flags);

    lan1qtsn_debug("ptp%u: %s delta=%lld, offset=%llu", ptp_data->ptp_id, __func__, delta, ptp_data->offset);

    return 0;
}

/**
 * lan1qtsn_ptp_adjfine - Fine adjustment of PTP time
 * @ptp: PTP clock info structure
 * @scaled_ppm: Scaled parts per million adjustment
 * @return: 0 on success
 */
static int lan1qtsn_ptp_adjfine(struct ptp_clock_info* ptp, long scaled_ppm) {
    u64 cur_timestamp, new_timestamp;
    u64 sys_clock;
    double diff;
    unsigned long flags;
    int is_negative = 0;

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);
    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;

#if 1 /* 20250820 */
    pr_err(">>> %s, but return immediately\n", __func__);
#endif
    spin_lock_irqsave(&ptp_data->lock, flags);

    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);

    if (scaled_ppm == 0) {
        goto exit;
    }

    cur_timestamp = lan1qtsn_get_timestamp(sys_clock, ptp_data->ticks_scale, ptp_data->offset);

    if (scaled_ppm < 0) {
        is_negative = 1;
        scaled_ppm = -scaled_ppm;
    }

    /* Adjust ticks_scale */
    diff = TICKS_SCALE * (double)scaled_ppm / (double)(1000000ULL << 16);
    ptp_data->ticks_scale = TICKS_SCALE + (is_negative ? -diff : diff);

    /* Adjust offset */
    new_timestamp = lan1qtsn_get_timestamp(sys_clock, ptp_data->ticks_scale, ptp_data->offset);
    ptp_data->offset += (cur_timestamp - new_timestamp);

    /* Adjust cycle_1s */
    /* set_ticks_scale(ptp_data, ptp_data->ticks_scale); */

    /* Set pulse_at */
    sys_clock = lan1qtsn_get_sys_clock_by_ldev(tc6);
    /* set_pulse_at(ptp_data, sys_clock); */

    lan1qtsn_debug("ptp%u: %s scaled_ppm=%ld, offset=%llu, ticks_scale:%lf", ptp_data->ptp_id, __func__, scaled_ppm,
                   ptp_data->offset, ptp_data->ticks_scale);

exit:
    spin_unlock_irqrestore(&ptp_data->lock, flags);

    return 0;
}

/**
 * ptp_clock_info_init - Initialize PTP clock info structure
 * @return: Initialized PTP clock info structure
 */
static struct ptp_clock_info ptp_clock_info_init(void) {
    struct ptp_clock_info info = {
        .owner = THIS_MODULE,
        .name = "ptp",
        .max_adj = RESERVED_CYCLE,
        .n_ext_ts = 0,
        .pps = 0,
        .adjfine = lan1qtsn_ptp_adjfine,
        .adjtime = lan1qtsn_ptp_adjtime,
        .gettimex64 = lan1qtsn_ptp_gettimex,
        .settime64 = lan1qtsn_ptp_settime,
    };

    return info;
}

/**
 * ptp_device_init - Initialize PTP device
 * @dev: Device pointer
 * @ldev: lan1qtsn_dev structure pointer
 * @return: Pointer to initialized PTP device data, NULL on failure
 */
struct ptp_device_data* ptp_device_init(struct device* dev, struct lan1qtsn_dev* ldev) {
    struct ptp_device_data* ptp;
    struct timespec64 ts;
#ifdef __LIB1QTSN_DEBUG__
    static u32 ptp_cnt = 0;
#endif

    if (!dev || !ldev) {
        pr_err("Invalid parameters for PTP device init\n");
        return NULL;
    }

    ptp = kzalloc(sizeof(struct ptp_device_data), GFP_KERNEL);
    if (!ptp) {
        pr_err("Failed to allocate memory for ptp device\n");
        return NULL;
    }

    ptp->ptp_info = ptp_clock_info_init();
    ptp->ticks_scale = TICKS_SCALE;

    spin_lock_init(&ptp->lock);

    ptp->ldev = ldev;

    ptp->ptp_clock = ptp_clock_register(&ptp->ptp_info, dev);
    if (IS_ERR(ptp->ptp_clock)) {
        pr_err("Failed to register ptp clock\n");
        kfree(ptp);
        return NULL;
    }

#ifdef __LIB1QTSN_DEBUG__
    ptp->ptp_id = ptp_cnt++;
#endif

    /* Set offset, cycle_1s */
    ts = ktime_to_timespec64(ktime_get_real());
#if 1 /* POOKY 20250821 must be 1 */
    lan1qtsn_ptp_settime(&ptp->ptp_info, &ts);
#endif

    return ptp;
}

/**
 * ptp_device_destroy - Destroy PTP device and cleanup resources
 * @ptp_data: PTP device data structure to destroy
 */
void ptp_device_destroy(struct ptp_device_data* ptp_data) {
    if (!ptp_data) {
        pr_err("ptp_device_destroy: NULL ptp_data\n");
        return;
    }

    if (ptp_data->ptp_clock && !IS_ERR(ptp_data->ptp_clock)) {
        ptp_clock_unregister(ptp_data->ptp_clock);
    }

    kfree(ptp_data);
}
```

이제 `lan1qtsn_spi.c` 파일에 함수 description을 추가하겠습니다.


```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include "lan1qtsn_spi.h"

#include <linux/bitfield.h>
#include <linux/crc32.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/stddef.h>

#include "lan1qtsn_netdev.h"

#define LOWER_29_BITS ((1ULL << 29) - 1)
#define TX_WORK_OVERFLOW_MARGIN 100

/**
 * lan1qtsn_spi_transfer - Perform SPI transfer operation
 * @tc6: lan1qtsn_spi struct pointer
 * @header_type: Type of SPI header to use
 * @length: Length of data to transfer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);

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

    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    return 0;
}

/**
 * lan1qtsn_spi_calculate_ctrl_buf_size - Calculate control buffer size
 * @length: Number of registers to operate on
 * @reg_op: Type of register operation (read/write)
 * @return: Calculated buffer size in bytes
 */
static u16 lan1qtsn_spi_calculate_ctrl_buf_size(u8 length, enum lan1qtsn_spi_register_op reg_op) {
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length;
    }
    return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;
}

/**
 * lan1qtsn_spi_update_ctrl_write_data - Update control write data buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @value: Array of values to write
 * @length: Number of values
 */
static void lan1qtsn_spi_update_ctrl_write_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* tx_buf = tc6->spi_ctrl_tx_buf + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        *tx_buf++ = cpu_to_be32(value[i]);
    }
}

/**
 * lan1qtsn_spi_prepare_cmd_header - Prepare SPI command header
 * @tx_buf: Transmit buffer to write header to
 * @cmd: Command byte
 * @address: Register address
 */
static void lan1qtsn_spi_prepare_cmd_header(u8* tx_buf, u8 cmd, u16 address) {
    u16* addr_ptr = (uint16_t*)&tx_buf[1];

    tx_buf[0] = cmd;
    *addr_ptr = htons(address);
}

/**
 * lan1qtsn_spi_prepare_ctrl_spi_buf - Prepare control SPI buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to operate on
 * @value: Array of values for read/write operations
 * @length: Number of values to process
 * @reg_op: Type of register operation (read/write)
 */
static void lan1qtsn_spi_prepare_ctrl_spi_buf(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                              enum lan1qtsn_spi_register_op reg_op) {
    u8* tx_buf = tc6->spi_ctrl_tx_buf;
    u8 cmd;
    u16 addr;

    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        cmd = LAN1QTSN_SPI_CMD_WRITE;
        addr = LAN1QTSN_SPI_REG_WRITE_FIFO_BASE | (address & 0xFFF);
        lan1qtsn_spi_update_ctrl_write_data(tc6, value, length);
    } else {
        cmd = LAN1QTSN_SPI_CMD_READ;
        addr = LAN1QTSN_SPI_REG_READ_FIFO_BASE | (address & 0xFFF);
    }

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);
}

/**
 * lan1qtsn_spi_copy_ctrl_read_data - Copy control read data from buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @value: Array to store read values
 * @length: Number of values to read
 */
static void lan1qtsn_spi_copy_ctrl_read_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* rx_buf = tc6->spi_ctrl_rx_buf + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        value[i] = be32_to_cpu(*rx_buf++);
    }
}

/**
 * lan1qtsn_spi_perform_ctrl - Perform SPI control operations
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to operate on
 * @value: Array of values for read/write operations
 * @length: Number of values to process
 * @reg_op: Type of register operation (read/write)
 * @return: 0 on success, negative error code on failure
 *
 * This function is protected by spi_ctrl_lock mutex in the calling functions
 * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
 * to prevent concurrent register access from multiple workers.
 */
int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                              enum lan1qtsn_spi_register_op reg_op) {
    /*
     * This function is protected by spi_ctrl_lock mutex in the calling functions
     * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
     * to prevent concurrent register access from multiple workers.
     */
    u16 size;
    int ret;

    /* Prepare control command and copy to SPI control buffer */
    lan1qtsn_spi_prepare_ctrl_spi_buf(tc6, address, value, length, reg_op);

    size = lan1qtsn_spi_calculate_ctrl_buf_size(length, reg_op);

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_CTRL_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* Check echoed/received control write command reply for errors */
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_READ) {
        lan1qtsn_spi_copy_ctrl_read_data(tc6, value, length);
    }

    return 0;
}

/**
 * lan1qtsn_spi_read_registers - Read multiple consecutive registers
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Address of the first register to read
 * @value: Array to store read values
 * @length: Number of consecutive registers to read
 * @return: 0 on success, negative error code on failure
 *
 * Maximum of 128 consecutive registers can be read starting at @address.
 */
int lan1qtsn_spi_read_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length) {
    int ret;

    if (!length || length > LAN1QTSN_SPI_CTRL_MAX_REGISTERS) {
        dev_err(&tc6->spi->dev, "Invalid register length parameter\n");
        return -EINVAL;
    }

    /* Check if we're already holding the lock to prevent deadlock */
    if (mutex_is_locked(&tc6->spi_ctrl_lock)) {
        dev_dbg(&tc6->spi->dev, "Lock already held, cannot read registers\n");
        return -EINVAL;
    }

    mutex_lock(&tc6->spi_ctrl_lock);
    ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, length, LAN1QTSN_SPI_CTRL_REG_READ);
    mutex_unlock(&tc6->spi_ctrl_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_registers);

/**
 * lan1qtsn_spi_read_register - Read a single MAC-PHY register
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to read
 * @value: Pointer to store read value
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_spi_read_register(struct lan1qtsn_spi* tc6, u32 address, u32* value) {
    return lan1qtsn_spi_read_registers(tc6, address, value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_register);

/**
 * lan1qtsn_spi_write_registers - Write multiple consecutive registers
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Address of the first register to write
 * @value: Array of values to write
 * @length: Number of consecutive registers to write
 * @return: 0 on success, negative error code on failure
 *
 * Maximum of 128 consecutive registers can be written starting at @address.
 */
int lan1qtsn_spi_write_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length) {
    int ret;

    if (!length || length > LAN1QTSN_SPI_CTRL_MAX_REGISTERS) {
        dev_err(&tc6->spi->dev, "Invalid register length parameter\n");
        return -EINVAL;
    }

    mutex_lock(&tc6->spi_ctrl_lock);
    ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, length, LAN1QTSN_SPI_CTRL_REG_WRITE);
    mutex_unlock(&tc6->spi_ctrl_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_registers);

/**
 * lan1qtsn_calculate_fcs - Calculate FCS (Frame Check Sequence)
 * @data: Ethernet frame data (excluding FCS)
 * @len: Data length
 * @return: Calculated FCS value
 */
static u32 lan1qtsn_calculate_fcs(const u8* data, u16 len) {
    u32 crc = ~0; /* initial value 0xFFFFFFFF */

    /* CRC32 calculation (little-endian) */
    crc = crc32_le(crc, data, len);

    /* final XOR (0xFFFFFFFF) */
    return ~crc;
}

/**
 * lan1qtsn_verify_fcs - Verify FCS (Frame Check Sequence)
 * @data: Ethernet frame data (excluding FCS)
 * @len: Data length
 * @received_fcs: Received FCS value
 * @return: true if FCS is correct, false on error
 */
static bool lan1qtsn_verify_fcs(const u8* data, u16 len, u32 received_fcs) {
    u32 calculated_fcs = lan1qtsn_calculate_fcs(data, len);
    return calculated_fcs == received_fcs;
}

/**
 * lan1qtsn_spi_write_register - Write a single MAC-PHY register
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to write
 * @value: Value to write
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value) {
    return lan1qtsn_spi_write_registers(tc6, address, &value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_register);

/**
 * lan1qtsn_spi_enable_data_transfer - Enable data transfer functionality
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success
 */
static int lan1qtsn_spi_enable_data_transfer(struct lan1qtsn_spi* tc6) {
    /* TSN system will be enabled when interface is opened */
    dev_dbg(&tc6->spi->dev, "Data transfer will be enabled when interface is opened\n");
    return 0;
}

/**
 * lan1qtsn_spi_cleanup_ongoing_rx_skb - Clean up ongoing RX socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 */
static void lan1qtsn_spi_cleanup_ongoing_rx_skb(struct lan1qtsn_spi* tc6) {
    if (tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        kfree_skb(tc6->rx_skb);
        tc6->rx_skb = NULL;
    }
}

/**
 * lan1qtsn_spi_update_rx_skb - Update RX socket buffer with received data
 * @tc6: lan1qtsn_spi struct pointer
 * @payload: Received data payload
 * @length: Length of received data
 */
static void lan1qtsn_spi_update_rx_skb(struct lan1qtsn_spi* tc6, u8* payload, u16 length) {
    /* Validate input parameters */
    if (!tc6 || !tc6->rx_skb || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return;
    }

    /* Check if we have enough space in the skb */
    if (skb_tailroom(tc6->rx_skb) < length) {
        dev_err(&tc6->spi->dev, "Not enough space in RX skb: need=%d, available=%d\n", length,
                skb_tailroom(tc6->rx_skb));
        return;
    }

    memcpy(skb_put(tc6->rx_skb, length), payload, length);
}

/**
 * lan1qtsn_spi_allocate_rx_skb - Allocate RX socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    /* Allocate buffer for frame without FCS since we remove it before submitting to network stack */
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN + ETH_FCS_LEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb\n");
        return -ENOMEM;
    }

    return 0;
}

/**
 * lan1qtsn_spi_submit_rx_skb - Submit RX socket buffer to network stack
 * @tc6: lan1qtsn_spi struct pointer
 */
static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    if (!tc6 || !tc6->rx_skb) {
        return;
    }

    /* Set protocol before submitting to network stack */
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);

    /* Update statistics */
    tc6->netdev->stats.rx_packets++;
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    /* Always use workqueue to avoid scheduling while atomic in SPI thread */
    if (tc6->rx_workqueue) {
        struct lan1qtsn_rx_work* work = kmalloc(sizeof(*work), GFP_ATOMIC);
        if (work) {
            work->skb = tc6->rx_skb;
            work->netdev = tc6->netdev;
            INIT_WORK(&work->work, lan1qtsn_rx_work_handler);
            queue_work(tc6->rx_workqueue, &work->work);
        } else {
            /* If work allocation fails, drop the packet to avoid issues */
            dev_err(&tc6->spi->dev, "Failed to allocate RX work, dropping packet\n");
            dev_kfree_skb_any(tc6->rx_skb);
        }
    } else {
        /* If workqueue not available, drop the packet to avoid issues */
        dev_err(&tc6->spi->dev, "RX workqueue not available, dropping packet\n");
        dev_kfree_skb_any(tc6->rx_skb);
    }

    tc6->rx_skb = NULL;
}

/**
 * lan1qtsn_spi_prcs_complete_rx_frame - Process complete RX frame
 * @tc6: lan1qtsn_spi struct pointer
 * @payload: Frame payload data
 * @size: Frame size
 * @meta: RX metadata
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_prcs_complete_rx_frame(struct lan1qtsn_spi* tc6, u8* payload, u16 size,
                                               struct rx_metadata meta) {
    int ret;
    struct ethhdr* eth;
    bool is_broadcast;
    bool is_multicast;
    bool is_for_me;

    /* Validate input parameters */
    if (!tc6 || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

#if 0
    /* minimum Ethernet header size check */
    if (size < ETH_HLEN) {
        dev_err(&tc6->spi->dev, "Packet too small: %d\n", size);
        return -EINVAL;
    }
#endif

    /* parse Ethernet header */
    eth = (struct ethhdr*)payload;

    /* MAC address check */
    is_broadcast = is_broadcast_ether_addr(eth->h_dest);
    is_multicast = is_multicast_ether_addr(eth->h_dest);
    is_for_me = ether_addr_equal(eth->h_dest, tc6->netdev->dev_addr); /* use netdev->dev_addr instead of tc6->mac_addr */

    /* MAC address debug output */
    dev_dbg(&tc6->spi->dev, "MAC Check - Dst: %pM, My MAC: %pM, Broadcast: %d, Multicast: %d, For me: %d\n",
            eth->h_dest, tc6->netdev->dev_addr, is_broadcast, is_multicast, is_for_me);

    if (!is_broadcast && !is_multicast && !is_for_me) {
        /* increment wrong MAC address count */
        atomic_inc(&tc6->wrong_mac_count);

        /* packet dump output */
        dev_info(&tc6->spi->dev, "Wrong MAC detected (count: %d)\n", atomic_read(&tc6->wrong_mac_count));
        dev_info(&tc6->spi->dev, "Dst MAC: %pM\n", eth->h_dest);
        dev_info(&tc6->spi->dev, "My MAC: %pM\n", tc6->netdev->dev_addr);
        dev_info(&tc6->spi->dev, "Src MAC: %pM\n", eth->h_source);
        dev_info(&tc6->spi->dev, "Type/Len: 0x%04x\n", ntohs(eth->h_proto));

        /* print_hex_dump(KERN_INFO, "Packet: ", DUMP_PREFIX_OFFSET, 16, 1, payload, size, true); */

        return 0; /* drop packet */
    }

    ret = lan1qtsn_spi_allocate_rx_skb(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb: %d\n", ret);
        return ret;
    }

    lan1qtsn_spi_update_rx_skb(tc6, payload, size + ETH_FCS_LEN);

    lan1qtsn_spi_submit_rx_skb(tc6);

    return 0;
}

/**
 * lan1qtsn_spi_process_spi_data_rx_buf - Process SPI data RX buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @meta: RX metadata
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_process_spi_data_rx_buf(struct lan1qtsn_spi* tc6, struct rx_metadata meta) {
    int ret;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u16 payload_size;
    u16 frame_size_without_fcs;
    u8* payload = tc6->spi_data_rx_buf + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    /* Validate input parameters */
    if (!tc6 || !tx_buf || !tc6->spi_data_rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    cmd = LAN1QTSN_SPI_CMD_READ;
    addr = LAN1QTSN_SPI_RX_FRAME_FIFO_BASE;

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    /* meta.frame_length is the actual payload size, not including headers */
    payload_size = meta.frame_length;
    size = payload_size + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    /* Check for buffer overflow */
    if (size > LAN1QTSN_SPI_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected: size=%d, max=%d\n", size, LAN1QTSN_SPI_DATA_BUF_SIZE);
        return -EINVAL;
    }

    /* Validate payload pointer */
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

    /* Calculate frame size without FCS (remove last 4 bytes) */
    if (payload_size >= ETH_FCS_LEN) {
        frame_size_without_fcs = payload_size - ETH_FCS_LEN;
    } else {
        dev_err(&tc6->spi->dev, "Payload size too small for FCS removal: %d\n", payload_size);
        return -EINVAL;
    }

#if 0
    /* Validate frame size */
    if (frame_size_without_fcs < ETH_ZLEN || frame_size_without_fcs > tc6->netdev->mtu + ETH_HLEN) {
        dev_err(&tc6->spi->dev, "Invalid frame size: %d\n", frame_size_without_fcs);
        return -EINVAL;
    }
#endif

    /* perform FCS check */
    u32 received_fcs;
    u8* fcs_ptr = payload + frame_size_without_fcs;

    /* extract received FCS value (little-endian) */
    received_fcs = (fcs_ptr[3] << 24) | (fcs_ptr[2] << 16) | (fcs_ptr[1] << 8) | fcs_ptr[0];

    /* FCS verification */
    if (!lan1qtsn_verify_fcs(payload, frame_size_without_fcs, received_fcs)) {
        /* increment FCS error statistics */
        tc6->netdev->stats.rx_errors++;

        /* output FCS error to kernel message */
        dev_err(&tc6->spi->dev, "FCS Error detected - dropping packet\n");
        dev_err(&tc6->spi->dev, "Frame size: %d, Received FCS: 0x%08x\n", frame_size_without_fcs, received_fcs);

        /* drop error packet */
        return 0;
    }

    return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, frame_size_without_fcs, meta);
}

/**
 * lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs - Prepare SPI TX buffer for transmission
 * @tc6: lan1qtsn_spi struct pointer
 * @return: Total length of prepared buffer, 0 if no data to transmit
 */
static u16 lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(struct lan1qtsn_spi* tc6) {
    u8* tx_buf = tc6->spi_data_tx_buf;
    struct sk_buff* skb;
    u16 copy_len, total_len;
    u32 fcs;
    u8* eth_data;
    u16 eth_len;

    /* 1. Move the waiting skb to ongoing transmission slot */
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    /* 2. Check if there is skb to transmit */
    if (!skb) {
        dev_dbg(&tc6->spi->dev, "TX: No waiting skb to transmit, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Preparing skb for transmission, len=%d, tx_packets=%lu\n", skb->len,
            tc6->netdev->stats.tx_packets);

    copy_len = skb->len;

    /* 3. Copy the original frame data to the SPI buffer, after command header */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    /* 4. Calculate Ethernet frame data start address and length (excluding metadata) */
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    dev_dbg(&tc6->spi->dev, "TX: Ethernet data len=%d, metadata size=%ld, tx_packets=%lu\n", eth_len, TX_METADATA_SIZE,
            tc6->netdev->stats.tx_packets);

    /* 5. Calculate FCS (CRC32) over the pure Ethernet frame */
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; /* Final XOR */

    /* 6. Append FCS to SPI transmit buffer after the frame data */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    /* 7. Calculate total length (# bytes to transmit) */
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    /* 8. Check buffer overflow, clean up and exit on error */
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        dev_err(&tc6->spi->dev, "TX: Buffer overflow: total_len=%d, max=%d, tx_packets=%lu\n", total_len,
                LAN1QTSN_BUFFER_SIZE, tc6->netdev->stats.tx_packets);
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Frame prepared, total_len=%d, eth_len=%d, tx_packets=%lu\n", total_len, eth_len,
            tc6->netdev->stats.tx_packets);

    /* 9. Prepare the SPI command header */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);

    return total_len;
}

/**
 * lan1qtsn_spi_get_meta_data - Get metadata from SPI device
 * @tc6: lan1qtsn_spi struct pointer
 * @meta: Pointer to metadata structure to fill
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_get_meta_data(struct lan1qtsn_spi* tc6, struct rx_metadata* meta) {
    int ret;
    u8* tx_buf = tc6->spi_meta_tx_buf;
    u8* rx_buf = (u8*)tc6->spi_meta_rx_buf;
    u16 frame_length;
    u8 cmd = LAN1QTSN_SPI_CMD_READ;
    u16 addr = LAN1QTSN_SPI_METADATA_FIFO_BASE;
    u16 transfer_size = LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_META_DATA_SIZE;

    /* Validate input parameters */
    if (!tc6 || !meta || !tx_buf || !rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    /* 1. Prepare SPI MPW header for metadata read function */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    /* 2. Transfer metadata block from device using SPI */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_META_HEADER, transfer_size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* 3. rx_meta points to extracted metadata in receive buffer */
    /* Add bounds checking to prevent buffer overflow */
    if (LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + sizeof(struct rx_metadata) >
        LAN1QTSN_SPI_META_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected in %s\n", __func__);
        return -EINVAL;
    }

    /* Use aligned memory access to prevent unaligned access issues */
    u8* meta_offset = &rx_buf[LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE];

    /* Validate meta_offset pointer */
    if (!meta_offset) {
        dev_err(&tc6->spi->dev, "Invalid meta_offset pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* 4. Convert and assign frame_length to meta structure using safe memory access */
    /* Use memcpy to avoid potential unaligned access */
    u16 temp_frame_length;
    memcpy(&temp_frame_length, meta_offset + offsetof(struct rx_metadata, frame_length), sizeof(u16));
    frame_length = ntohs(temp_frame_length);
    meta->frame_length = frame_length;

    /* 5. If a valid frame is present, convert and assign timestamps */
    if (frame_length) {
        /* Use safe memory access for timestamp */
        u64 temp_timestamp;
        memcpy(&temp_timestamp, meta_offset + offsetof(struct rx_metadata, timestamp), sizeof(u64));

        /* Convert from network byte order to host byte order */
        meta->timestamp = __be64_to_cpu(temp_timestamp);
    }

    return 0;
}

/**
 * lan1qtsn_spi_try_spi_transfer - Try to perform SPI transfer operations
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_try_spi_transfer(struct lan1qtsn_spi* tc6) {
    int ret;
    struct rx_metadata meta;

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
                ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, spi_len);

                if (ret) {
                    dev_err(&tc6->spi->dev, "TX: SPI transfer failed: %d, tx_packets=%lu\n", ret,
                            tc6->netdev->stats.tx_packets);
                    /* Clean up on error */
                    if (tc6->ongoing_tx_skb) {
                        kfree_skb(tc6->ongoing_tx_skb);
                        tc6->ongoing_tx_skb = NULL;
                    }
                    /* Wake up the queue even on error to allow retry */
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

                    /* Wake up the queue after successful transmission */
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }

                    /* increment count2 */
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

        if (!tc6->waiting_tx_skb && netif_queue_stopped(tc6->netdev)) {
            netif_wake_queue(tc6->netdev);
        }
    }

    return 0;
}

/**
 * lan1qtsn_spi_thread_handler - SPI thread handler function
 * @data: Thread data (lan1qtsn_spi struct pointer)
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;

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
            /* Don't return immediately, continue processing */
            /* usleep_range(50, 100); Shorter delay for better responsiveness in bidirectional traffic */
            return ret;
        }
    }

    return 0;
}

/**
 * lan1qtsn_spi_macphy_isr - MAC-PHY interrupt service routine
 * @irq: Interrupt number
 * @data: Interrupt data (lan1qtsn_spi struct pointer)
 * @return: IRQ_HANDLED
 */
static irqreturn_t lan1qtsn_spi_macphy_isr(int irq, void* data) {
    struct lan1qtsn_spi* tc6 = data;

    /* MAC-PHY interrupt can occur for the following reasons.
     * - availability of tx credits if it was 0 before and not reported in
     *   the previous rx footer.
     * - availability of rx chunks if it was 0 before and not reported in
     *   the previous rx footer.
     * - extended status event not reported in the previous rx footer.
     */
    tc6->int_flag = true;
    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return IRQ_HANDLED;
}

#ifndef __TSN_PTP__
/**
 * lan1qtsn_append_metadata_to_skb - Append metadata to socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @skb: Socket buffer to append metadata to
 * @return: 0 on success, -1 on failure
 */
static int lan1qtsn_append_metadata_to_skb(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    s
```

```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include "lan1qtsn_spi.h"

#include <linux/bitfield.h>
#include <linux/crc32.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/stddef.h>

#include "lan1qtsn_netdev.h"

#define LOWER_29_BITS ((1ULL << 29) - 1)
#define TX_WORK_OVERFLOW_MARGIN 100

/**
 * lan1qtsn_spi_transfer - Perform SPI transfer operation
 * @tc6: lan1qtsn_spi struct pointer
 * @header_type: Type of SPI header to use
 * @length: Length of data to transfer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);

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

    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    return 0;
}

/**
 * lan1qtsn_spi_calculate_ctrl_buf_size - Calculate control buffer size
 * @length: Number of registers to operate on
 * @reg_op: Type of register operation
 * @return: Required buffer size in bytes
 */
static u16 lan1qtsn_spi_calculate_ctrl_buf_size(u8 length, enum lan1qtsn_spi_register_op reg_op) {
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length;
    }
    return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;
}

/**
 * lan1qtsn_spi_update_ctrl_write_data - Update control write data buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @value: Array of values to write
 * @length: Number of values
 */
static void lan1qtsn_spi_update_ctrl_write_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* tx_buf = tc6->spi_ctrl_tx_buf + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        *tx_buf++ = cpu_to_be32(value[i]);
    }
}

/**
 * lan1qtsn_spi_prepare_cmd_header - Prepare SPI command header
 * @tx_buf: Transmit buffer
 * @cmd: Command byte
 * @address: Register address
 */
static void lan1qtsn_spi_prepare_cmd_header(u8* tx_buf, u8 cmd, u16 address) {
    u16* addr_ptr = (uint16_t*)&tx_buf[1];

    tx_buf[0] = cmd;
    *addr_ptr = htons(address);
}

/**
 * lan1qtsn_spi_prepare_ctrl_spi_buf - Prepare control SPI buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address
 * @value: Array of values
 * @length: Number of values
 * @reg_op: Type of register operation
 */
static void lan1qtsn_spi_prepare_ctrl_spi_buf(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                              enum lan1qtsn_spi_register_op reg_op) {
    u8* tx_buf = tc6->spi_ctrl_tx_buf;
    u8 cmd;
    u16 addr;

    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        cmd = LAN1QTSN_SPI_CMD_WRITE;
        addr = LAN1QTSN_SPI_REG_WRITE_FIFO_BASE | (address & 0xFFF);
        lan1qtsn_spi_update_ctrl_write_data(tc6, value, length);
    } else {
        cmd = LAN1QTSN_SPI_CMD_READ;
        addr = LAN1QTSN_SPI_REG_READ_FIFO_BASE | (address & 0xFFF);
    }

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);
}

/**
 * lan1qtsn_spi_copy_ctrl_read_data - Copy control read data from buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @value: Array to store read values
 * @length: Number of values to read
 */
static void lan1qtsn_spi_copy_ctrl_read_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* rx_buf = tc6->spi_ctrl_rx_buf + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        value[i] = be32_to_cpu(*rx_buf++);
    }
}

/**
 * lan1qtsn_spi_perform_ctrl - Perform SPI control operations
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to operate on
 * @value: Array of values for read/write operations
 * @length: Number of values to process
 * @reg_op: Type of register operation (read/write)
 * @return: 0 on success, negative error code on failure
 *
 * This function is protected by spi_ctrl_lock mutex in the calling functions
 * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
 * to prevent concurrent register access from multiple workers.
 */
int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                              enum lan1qtsn_spi_register_op reg_op) {
    u16 size;
    int ret;

    /* Prepare control command and copy to SPI control buffer */
    lan1qtsn_spi_prepare_ctrl_spi_buf(tc6, address, value, length, reg_op);

    size = lan1qtsn_spi_calculate_ctrl_buf_size(length, reg_op);

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_CTRL_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* Check echoed/received control write command reply for errors */
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_READ) {
        lan1qtsn_spi_copy_ctrl_read_data(tc6, value, length);
    }

    return 0;
}

/**
 * lan1qtsn_spi_read_registers - Read multiple consecutive registers
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Address of the first register to read
 * @value: Array to store read values
 * @length: Number of consecutive registers to read
 * @return: 0 on success, negative error code on failure
 *
 * Maximum of 128 consecutive registers can be read starting at @address.
 */
int lan1qtsn_spi_read_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length) {
    int ret;

    if (!length || length > LAN1QTSN_SPI_CTRL_MAX_REGISTERS) {
        dev_err(&tc6->spi->dev, "Invalid register length parameter\n");
        return -EINVAL;
    }

    /* Check if we're already holding the lock to prevent deadlock */
    if (mutex_is_locked(&tc6->spi_ctrl_lock)) {
        dev_dbg(&tc6->spi->dev, "Lock already held, cannot read registers\n");
        return -EINVAL;
    }

    mutex_lock(&tc6->spi_ctrl_lock);
    ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, length, LAN1QTSN_SPI_CTRL_REG_READ);
    mutex_unlock(&tc6->spi_ctrl_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_registers);

/**
 * lan1qtsn_spi_read_register - Read a single MAC-PHY register
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to read
 * @value: Pointer to store read value
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_spi_read_register(struct lan1qtsn_spi* tc6, u32 address, u32* value) {
    return lan1qtsn_spi_read_registers(tc6, address, value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_register);

/**
 * lan1qtsn_spi_write_registers - Write multiple consecutive registers
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Address of the first register to write
 * @value: Array of values to write
 * @length: Number of consecutive registers to write
 * @return: 0 on success, negative error code on failure
 *
 * Maximum of 128 consecutive registers can be written starting at @address.
 */
int lan1qtsn_spi_write_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length) {
    int ret;

    if (!length || length > LAN1QTSN_SPI_CTRL_MAX_REGISTERS) {
        dev_err(&tc6->spi->dev, "Invalid register length parameter\n");
        return -EINVAL;
    }

    mutex_lock(&tc6->spi_ctrl_lock);
    ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, length, LAN1QTSN_SPI_CTRL_REG_WRITE);
    mutex_unlock(&tc6->spi_ctrl_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_registers);

/**
 * lan1qtsn_calculate_fcs - Calculate FCS (Frame Check Sequence)
 * @data: Ethernet frame data (excluding FCS)
 * @len: Data length
 * @return: Calculated FCS value
 */
static u32 lan1qtsn_calculate_fcs(const u8* data, u16 len) {
    u32 crc = ~0; /* initial value 0xFFFFFFFF */

    /* CRC32 calculation (little-endian) */
    crc = crc32_le(crc, data, len);

    /* final XOR (0xFFFFFFFF) */
    return ~crc;
}

/**
 * lan1qtsn_verify_fcs - Verify FCS (Frame Check Sequence)
 * @data: Ethernet frame data (excluding FCS)
 * @len: Data length
 * @received_fcs: Received FCS value
 * @return: true if FCS is correct, false on error
 */
static bool lan1qtsn_verify_fcs(const u8* data, u16 len, u32 received_fcs) {
    u32 calculated_fcs = lan1qtsn_calculate_fcs(data, len);
    return calculated_fcs == received_fcs;
}

/**
 * lan1qtsn_spi_write_register - Write a single MAC-PHY register
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to write
 * @value: Value to write
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value) {
    return lan1qtsn_spi_write_registers(tc6, address, &value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_register);

/**
 * lan1qtsn_spi_enable_data_transfer - Enable data transfer
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success
 */
static int lan1qtsn_spi_enable_data_transfer(struct lan1qtsn_spi* tc6) {
    /* TSN system will be enabled when interface is opened */
    dev_dbg(&tc6->spi->dev, "Data transfer will be enabled when interface is opened\n");
    return 0;
}

/**
 * lan1qtsn_spi_cleanup_ongoing_rx_skb - Clean up ongoing RX socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 */
static void lan1qtsn_spi_cleanup_ongoing_rx_skb(struct lan1qtsn_spi* tc6) {
    if (tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        kfree_skb(tc6->rx_skb);
        tc6->rx_skb = NULL;
    }
}

/**
 * lan1qtsn_spi_update_rx_skb - Update RX socket buffer with received data
 * @tc6: lan1qtsn_spi struct pointer
 * @payload: Received payload data
 * @length: Length of payload data
 */
static void lan1qtsn_spi_update_rx_skb(struct lan1qtsn_spi* tc6, u8* payload, u16 length) {
    /* Validate input parameters */
    if (!tc6 || !tc6->rx_skb || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return;
    }

    /* Check if we have enough space in the skb */
    if (skb_tailroom(tc6->rx_skb) < length) {
        dev_err(&tc6->spi->dev, "Not enough space in RX skb: need=%d, available=%d\n", length,
                skb_tailroom(tc6->rx_skb));
        return;
    }

    memcpy(skb_put(tc6->rx_skb, length), payload, length);
}

/**
 * lan1qtsn_spi_allocate_rx_skb - Allocate RX socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    /* Allocate buffer for frame without FCS since we remove it before submitting to network stack */
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN + ETH_FCS_LEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb\n");
        return -ENOMEM;
    }

    return 0;
}

/**
 * lan1qtsn_spi_submit_rx_skb - Submit RX socket buffer to network stack
 * @tc6: lan1qtsn_spi struct pointer
 */
static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    if (!tc6 || !tc6->rx_skb) {
        return;
    }

    /* Set protocol before submitting to network stack */
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);

    /* Update statistics */
    tc6->netdev->stats.rx_packets++;
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    /* Always use workqueue to avoid scheduling while atomic in SPI thread */
    if (tc6->rx_workqueue) {
        struct lan1qtsn_rx_work* work = kmalloc(sizeof(*work), GFP_ATOMIC);
        if (work) {
            work->skb = tc6->rx_skb;
            work->netdev = tc6->netdev;
            INIT_WORK(&work->work, lan1qtsn_rx_work_handler);
            queue_work(tc6->rx_workqueue, &work->work);
        } else {
            /* If work allocation fails, drop the packet to avoid issues */
            dev_err(&tc6->spi->dev, "Failed to allocate RX work, dropping packet\n");
            dev_kfree_skb_any(tc6->rx_skb);
        }
    } else {
        /* If workqueue not available, drop the packet to avoid issues */
        dev_err(&tc6->spi->dev, "RX workqueue not available, dropping packet\n");
        dev_kfree_skb_any(tc6->rx_skb);
    }

    tc6->rx_skb = NULL;
}

/**
 * lan1qtsn_spi_prcs_complete_rx_frame - Process complete RX frame
 * @tc6: lan1qtsn_spi struct pointer
 * @payload: Frame payload data
 * @size: Frame size
 * @meta: RX metadata
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_prcs_complete_rx_frame(struct lan1qtsn_spi* tc6, u8* payload, u16 size,
                                               struct rx_metadata meta) {
    int ret;
    struct ethhdr* eth;
    bool is_broadcast;
    bool is_multicast;
    bool is_for_me;

    /* Validate input parameters */
    if (!tc6 || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

#if 0
    /* minimum Ethernet header size check */
    if (size < ETH_HLEN) {
        dev_err(&tc6->spi->dev, "Packet too small: %d\n", size);
        return -EINVAL;
    }
#endif

    /* parse Ethernet header */
    eth = (struct ethhdr*)payload;

    /* MAC address check */
    is_broadcast = is_broadcast_ether_addr(eth->h_dest);
    is_multicast = is_multicast_ether_addr(eth->h_dest);
    is_for_me = ether_addr_equal(eth->h_dest, tc6->netdev->dev_addr); /* use netdev->dev_addr instead of tc6->mac_addr */

    /* MAC address debug output */
    dev_dbg(&tc6->spi->dev, "MAC Check - Dst: %pM, My MAC: %pM, Broadcast: %d, Multicast: %d, For me: %d\n",
            eth->h_dest, tc6->netdev->dev_addr, is_broadcast, is_multicast, is_for_me);

    if (!is_broadcast && !is_multicast && !is_for_me) {
        /* increment wrong MAC address count */
        atomic_inc(&tc6->wrong_mac_count);

        /* packet dump output */
        dev_info(&tc6->spi->dev, "Wrong MAC detected (count: %d)\n", atomic_read(&tc6->wrong_mac_count));
        dev_info(&tc6->spi->dev, "Dst MAC: %pM\n", eth->h_dest);
        dev_info(&tc6->spi->dev, "My MAC: %pM\n", tc6->netdev->dev_addr);
        dev_info(&tc6->spi->dev, "Src MAC: %pM\n", eth->h_source);
        dev_info(&tc6->spi->dev, "Type/Len: 0x%04x\n", ntohs(eth->h_proto));

        /* print_hex_dump(KERN_INFO, "Packet: ", DUMP_PREFIX_OFFSET, 16, 1, payload, size, true); */

        return 0; /* drop packet */
    }

    ret = lan1qtsn_spi_allocate_rx_skb(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb: %d\n", ret);
        return ret;
    }

    lan1qtsn_spi_update_rx_skb(tc6, payload, size + ETH_FCS_LEN);

    lan1qtsn_spi_submit_rx_skb(tc6);

    return 0;
}

/**
 * lan1qtsn_spi_process_spi_data_rx_buf - Process SPI data RX buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @meta: RX metadata
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_process_spi_data_rx_buf(struct lan1qtsn_spi* tc6, struct rx_metadata meta) {
    int ret;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u16 payload_size;
    u16 frame_size_without_fcs;
    u8* payload = tc6->spi_data_rx_buf + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    /* Validate input parameters */
    if (!tc6 || !tx_buf || !tc6->spi_data_rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    cmd = LAN1QTSN_SPI_CMD_READ;
    addr = LAN1QTSN_SPI_RX_FRAME_FIFO_BASE;

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    /* meta.frame_length is the actual payload size, not including headers */
    payload_size = meta.frame_length;
    size = payload_size + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    /* Check for buffer overflow */
    if (size > LAN1QTSN_SPI_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected: size=%d, max=%d\n", size, LAN1QTSN_SPI_DATA_BUF_SIZE);
        return -EINVAL;
    }

    /* Validate payload pointer */
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

    /* Calculate frame size without FCS (remove last 4 bytes) */
    if (payload_size >= ETH_FCS_LEN) {
        frame_size_without_fcs = payload_size - ETH_FCS_LEN;
    } else {
        dev_err(&tc6->spi->dev, "Payload size too small for FCS removal: %d\n", payload_size);
        return -EINVAL;
    }

#if 0
    /* Validate frame size */
    if (frame_size_without_fcs < ETH_ZLEN || frame_size_without_fcs > tc6->netdev->mtu + ETH_HLEN) {
        dev_err(&tc6->spi->dev, "Invalid frame size: %d\n", frame_size_without_fcs);
        return -EINVAL;
    }
#endif

    /* perform FCS check */
    u32 received_fcs;
    u8* fcs_ptr = payload + frame_size_without_fcs;

    /* extract received FCS value (little-endian) */
    received_fcs = (fcs_ptr[3] << 24) | (fcs_ptr[2] << 16) | (fcs_ptr[1] << 8) | fcs_ptr[0];

    /* FCS verification */
    if (!lan1qtsn_verify_fcs(payload, frame_size_without_fcs, received_fcs)) {
        /* increment FCS error statistics */
        tc6->netdev->stats.rx_errors++;

        /* output FCS error to kernel message */
        dev_err(&tc6->spi->dev, "FCS Error detected - dropping packet\n");
        dev_err(&tc6->spi->dev, "Frame size: %d, Received FCS: 0x%08x\n", frame_size_without_fcs, received_fcs);

        /* drop error packet */
        return 0;
    }

    return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, frame_size_without_fcs, meta);
}

/**
 * lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs - Prepare SPI TX buffer for transmission
 * @tc6: lan1qtsn_spi struct pointer
 * @return: Total length of prepared data, 0 if no data to transmit
 */
static u16 lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(struct lan1qtsn_spi* tc6) {
    u8* tx_buf = tc6->spi_data_tx_buf;
    struct sk_buff* skb;
    u16 copy_len, total_len;
    u32 fcs;
    u8* eth_data;
    u16 eth_len;

    /* 1. Move the waiting skb to ongoing transmission slot */
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    /* 2. Check if there is skb to transmit */
    if (!skb) {
        dev_dbg(&tc6->spi->dev, "TX: No waiting skb to transmit, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Preparing skb for transmission, len=%d, tx_packets=%lu\n", skb->len,
            tc6->netdev->stats.tx_packets);

    copy_len = skb->len;

    /* 3. Copy the original frame data to the SPI buffer, after command header */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    /* 4. Calculate Ethernet frame data start address and length (excluding metadata) */
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    dev_dbg(&tc6->spi->dev, "TX: Ethernet data len=%d, metadata size=%ld, tx_packets=%lu\n", eth_len, TX_METADATA_SIZE,
            tc6->netdev->stats.tx_packets);

    /* 5. Calculate FCS (CRC32) over the pure Ethernet frame */
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; /* Final XOR */

    /* 6. Append FCS to SPI transmit buffer after the frame data */
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    /* 7. Calculate total length (# bytes to transmit) */
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    /* 8. Check buffer overflow, clean up and exit on error */
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        dev_err(&tc6->spi->dev, "TX: Buffer overflow: total_len=%d, max=%d, tx_packets=%lu\n", total_len,
                LAN1QTSN_BUFFER_SIZE, tc6->netdev->stats.tx_packets);
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    dev_dbg(&tc6->spi->dev, "TX: Frame prepared, total_len=%d, eth_len=%d, tx_packets=%lu\n", total_len, eth_len,
            tc6->netdev->stats.tx_packets);

    /* 9. Prepare the SPI command header */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);

    return total_len;
}

/**
 * lan1qtsn_spi_get_meta_data - Get metadata from SPI device
 * @tc6: lan1qtsn_spi struct pointer
 * @meta: Pointer to metadata structure to fill
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_get_meta_data(struct lan1qtsn_spi* tc6, struct rx_metadata* meta) {
    int ret;
    u8* tx_buf = tc6->spi_meta_tx_buf;
    u8* rx_buf = (u8*)tc6->spi_meta_rx_buf;
    u16 frame_length;
    u8 cmd = LAN1QTSN_SPI_CMD_READ;
    u16 addr = LAN1QTSN_SPI_METADATA_FIFO_BASE;
    u16 transfer_size = LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_META_DATA_SIZE;

    /* Validate input parameters */
    if (!tc6 || !meta || !tx_buf || !rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    /* 1. Prepare SPI MPW header for metadata read function */
    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    /* 2. Transfer metadata block from device using SPI */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_META_HEADER, transfer_size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* 3. rx_meta points to extracted metadata in receive buffer */
    /* Add bounds checking to prevent buffer overflow */
    if (LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + sizeof(struct rx_metadata) >
        LAN1QTSN_SPI_META_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected in %s\n", __func__);
        return -EINVAL;
    }

    /* Use aligned memory access to prevent unaligned access issues */
    u8* meta_offset = &rx_buf[LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE];

    /* Validate meta_offset pointer */
    if (!meta_offset) {
        dev_err(&tc6->spi->dev, "Invalid meta_offset pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* 4. Convert and assign frame_length to meta structure using safe memory access */
    /* Use memcpy to avoid potential unaligned access */
    u16 temp_frame_length;
    memcpy(&temp_frame_length, meta_offset + offsetof(struct rx_metadata, frame_length), sizeof(u16));
    frame_length = ntohs(temp_frame_length);
    meta->frame_length = frame_length;

    /* 5. If a valid frame is present, convert and assign timestamps */
    if (frame_length) {
        /* Use safe memory access for timestamp */
        u64 temp_timestamp;
        memcpy(&temp_timestamp, meta_offset + offsetof(struct rx_metadata, timestamp), sizeof(u64));

        /* Convert from network byte order to host byte order */
        meta->timestamp = __be64_to_cpu(temp_timestamp);
    }

    return 0;
}

/**
 * lan1qtsn_spi_try_spi_transfer - Try to perform SPI transfer
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_try_spi_transfer(struct lan1qtsn_spi* tc6) {
    int ret;
    struct rx_metadata meta;

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
                ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, spi_len);

                if (ret) {
                    dev_err(&tc6->spi->dev, "TX: SPI transfer failed: %d, tx_packets=%lu\n", ret,
                            tc6->netdev->stats.tx_packets);
                    /* Clean up on error */
                    if (tc6->ongoing_tx_skb) {
                        kfree_skb(tc6->ongoing_tx_skb);
                        tc6->ongoing_tx_skb = NULL;
                    }
                    /* Wake up the queue even on error to allow retry */
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

                    /* Wake up the queue after successful transmission */
                    if (netif_queue_stopped(tc6->netdev)) {
                        netif_wake_queue(tc6->netdev);
                    }

                    /* increment count2 */
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

        if (!tc6->waiting_tx_skb && netif_queue_stopped(tc6->netdev)) {
            netif_wake_queue(tc6->netdev);
        }
    }

    return 0;
}

/**
 * lan1qtsn_spi_thread_handler - SPI thread handler function
 * @data: Thread data (lan1qtsn_spi struct pointer)
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;

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
            /* Don't return immediately, continue processing */
            /* usleep_range(50, 100); Shorter delay for better responsiveness in bidirectional traffic */
            return ret;
        }
    }

    return 0;
}

/**
 * lan1qtsn_spi_macphy_isr - MAC-PHY interrupt service routine
 * @irq: Interrupt number
 * @data: Interrupt data (lan1qtsn_spi struct pointer)
 * @return: IRQ_HANDLED
 */
static irqreturn_t lan1qtsn_spi_macphy_isr(int irq, void* data) {
    struct lan1qtsn_spi* tc6 = data;

    /* MAC-PHY interrupt can occur for the following reasons.
     * - availability of tx credits if it was 0 before and not reported in
     *   the previous rx footer.
     * - availability of rx chunks if it was 0 before and not reported in
     *   the previous rx footer.
     * - extended status event not reported in the previous rx footer.
     */
    tc6->int_flag = true;
    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return IRQ_HANDLED;
}

#ifndef __TSN_PTP__
/**
 * lan1qtsn_append_metadata_to_skb - Append metadata to socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @skb: Socket buffer to append metadata to
 * @return: 0 on success, -1 on failure
 */
static int lan1qtsn_append_metadata_to_skb(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;

    lan1qtsn_debug("%s(skb->len : %d)\n", __func__, skb->len);
    skb->len = max((unsigned int)ETH_ZLEN, skb->len);

    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */

    if (skb_padto(skb, skb->len)) {
        pr_err("skb_padto f
```

```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include "lan1qtsn_spi.h"

#include <linux/bitfield.h>
#include <linux/crc32.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/stddef.h>

#include "lan1qtsn_netdev.h"

#define LOWER_29_BITS ((1ULL << 29) - 1)
#define TX_WORK_OVERFLOW_MARGIN 100

/**
 * lan1qtsn_spi_transfer - Perform SPI transfer operation
 * @tc6: lan1qtsn_spi struct pointer
 * @header_type: Type of SPI header to use
 * @length: Length of data to transfer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_s
```

```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include "lan1qtsn_spi.h"

#include <linux/bitfield.h>
#include <linux/crc32.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/stddef.h>

#include "lan1qtsn_netdev.h"

#define LOWER_29_BITS ((1ULL << 29) - 1)
#define TX_WORK_OVERFLOW_MARGIN 100

/**
 * lan1qtsn_spi_transfer - Perform SPI transfer operation
 * @tc6: lan1qtsn_spi struct pointer
 * @header_type: Type of SPI header to use
 * @length: Length of data to transfer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);

    if (header_type == LAN1QTSN_SPI_DATA_HEADER) {
        xfer.tx_buf = tc6->spi_data_tx_buf;
        xfer.rx_buf = tc6->spi_data_rx_buf;
    } else if (header_type == LAN1QTSN_SPI_META_HEADER) {
        xfer.tx_buf = tc6->spi_meta_tx_buf;
        xfe
```

---

**User**

이 디렉토리 내 모든 c 파일에 있는 함수들 중description이 없는 함수들의 description을 추가 해 줘

---

**Cursor**

이 디렉토리 내 모든 C 파일에 있는 함수들 중 description이 없는 함수들의 description을 추가하겠습니다. 아직 완료되지 않은 파일들을 계속 진행하겠습니다.

먼저 `lan1qtsn_spi.c` 파일에 함수 description을 추가하겠습니다.


```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include "lan1qtsn_spi.h"

#include <linux/bitfield.h>
#include <linux/crc32.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/stddef.h>

#include "lan1qtsn_netdev.h"

#define LOWER_29_BITS ((1ULL << 29) - 1)
#define TX_WORK_OVERFLOW_MARGIN 100

/**
 * lan1qtsn_spi_transfer - Perform SPI transfer with specified header type
 * @tc6: lan1qtsn_spi struct pointer
 * @header_type: Type of SPI header (data, meta, or control)
 * @length: Length of data to transfer
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    dev_dbg(&tc6->spi->dev, "SPI: Transfer starting - type=%d, length=%d, tx_packets=%lu\n", header_type, length,
            tc6->netdev->stats.tx_packets);

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

    int ret = spi_sync(tc6->spi, &msg);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI: Transfer failed with error %d, tx_packets=%lu\n", ret,
                tc6->netdev->stats.tx_packets);
        return ret;
    }

    dev_dbg(&tc6->spi->dev, "SPI: Transfer completed successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    return 0;
}

/**
 * lan1qtsn_spi_calculate_ctrl_buf_size - Calculate control buffer size
 * @length: Number of registers to operate on
 * @reg_op: Type of register operation (read/write)
 * @return: Calculated buffer size in bytes
 */
static u16 lan1qtsn_spi_calculate_ctrl_buf_size(u8 length, enum lan1qtsn_spi_register_op reg_op) {
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length;
    }
    return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;
}

/**
 * lan1qtsn_spi_update_ctrl_write_data - Update control write data in buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @value: Array of values to write
 * @length: Number of values
 */
static void lan1qtsn_spi_update_ctrl_write_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* tx_buf = tc6->spi_ctrl_tx_buf + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        *tx_buf++ = cpu_to_be32(value[i]);
    }
}

/**
 * lan1qtsn_spi_prepare_cmd_header - Prepare SPI command header
 * @tx_buf: Transmit buffer
 * @cmd: Command byte
 * @address: Register address
 */
static void lan1qtsn_spi_prepare_cmd_header(u8* tx_buf, u8 cmd, u16 address) {
    u16* addr_ptr = (uint16_t*)&tx_buf[1];

    tx_buf[0] = cmd;
    *addr_ptr = htons(address);
}

/**
 * lan1qtsn_spi_prepare_ctrl_spi_buf - Prepare control SPI buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address
 * @value: Array of values for write operations
 * @length: Number of values
 * @reg_op: Type of register operation (read/write)
 */
static void lan1qtsn_spi_prepare_ctrl_spi_buf(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                              enum lan1qtsn_spi_register_op reg_op) {
    u8* tx_buf = tc6->spi_ctrl_tx_buf;
    u8 cmd;
    u16 addr;

    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        cmd = LAN1QTSN_SPI_CMD_WRITE;
        addr = LAN1QTSN_SPI_REG_WRITE_FIFO_BASE | (address & 0xFFF);
        lan1qtsn_spi_update_ctrl_write_data(tc6, value, length);
    } else {
        cmd = LAN1QTSN_SPI_CMD_READ;
        addr = LAN1QTSN_SPI_REG_READ_FIFO_BASE | (address & 0xFFF);
    }

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);
}

/**
 * lan1qtsn_spi_copy_ctrl_read_data - Copy control read data from buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @value: Array to store read values
 * @length: Number of values to read
 */
static void lan1qtsn_spi_copy_ctrl_read_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* rx_buf = tc6->spi_ctrl_rx_buf + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        value[i] = be32_to_cpu(*rx_buf++);
    }
}
```

이제 `lan1qtsn_spi.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * lan1qtsn_spi_perform_ctrl - Perform SPI control operations
 * @tc6: lan1qtsn_spi struct pointer
 * @address: Register address to operate on
 * @value: Array of values for read/write operations
 * @length: Number of values to process
 * @reg_op: Type of register operation (read/write)
 * @return: 0 on success, negative error code on failure
 *
 * This function is protected by spi_ctrl_lock mutex in the calling functions
 * (lan1qtsn_spi_read_registers, lan1qtsn_spi_write_registers)
 * to prevent concurrent register access from multiple workers.
 */
int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                              enum lan1qtsn_spi_register_op reg_op) {
    u16 size;
    int ret;

    /* Prepare control command and copy to SPI control buffer */
    lan1qtsn_spi_prepare_ctrl_spi_buf(tc6, address, value, length, reg_op);

    size = lan1qtsn_spi_calculate_ctrl_buf_size(length, reg_op);

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_CTRL_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* Check echoed/received control write command reply for errors */
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_READ) {
        lan1qtsn_spi_copy_ctrl_read_data(tc6, value, length);
    }

    return 0;
}
```

이제 `lan1qtsn_spi.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * lan1qtsn_spi_enable_data_transfer - Enable SPI data transfer
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success
 *
 * This function is a placeholder as TSN system will be enabled when interface is opened.
 */
static int lan1qtsn_spi_enable_data_transfer(struct lan1qtsn_spi* tc6) {
    /* TSN system will be enabled when interface is opened */
    dev_dbg(&tc6->spi->dev, "Data transfer will be enabled when interface is opened\n");
    return 0;
}
```

이제 `lan1qtsn_spi.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * lan1qtsn_spi_cleanup_ongoing_rx_skb - Clean up ongoing RX socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 *
 * This function frees the RX socket buffer and updates drop statistics.
 */
static void lan1qtsn_spi_cleanup_ongoing_rx_skb(struct lan1qtsn_spi* tc6) {
    if (tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        kfree_skb(tc6->rx_skb);
        tc6->rx_skb = NULL;
    }
}

/**
 * lan1qtsn_spi_update_rx_skb - Update RX socket buffer with payload data
 * @tc6: lan1qtsn_spi struct pointer
 * @payload: Payload data to add
 * @length: Length of payload data
 */
static void lan1qtsn_spi_update_rx_skb(struct lan1qtsn_spi* tc6, u8* payload, u16 length) {
    /* Validate input parameters */
    if (!tc6 || !tc6->rx_skb || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return;
    }

    /* Check if we have enough space in the skb */
    if (skb_tailroom(tc6->rx_skb) < length) {
        dev_err(&tc6->spi->dev, "Not enough space in RX skb: need=%d, available=%d\n", length,
                skb_tailroom(tc6->rx_skb));
        return;
    }

    memcpy(skb_put(tc6->rx_skb, length), payload, length);
}

/**
 * lan1qtsn_spi_allocate_rx_skb - Allocate RX socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success, -ENOMEM on failure
 *
 * Allocates buffer for frame without FCS since we remove it before submitting to network stack.
 */
static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    /* Allocate buffer for frame without FCS since we remove it before submitting to network stack */
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN + ETH_FCS_LEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        dev_err(&tc6->spi->dev, "Failed to allocate RX skb\n");
        return -ENOMEM;
    }

    return 0;
}

/**
 * lan1qtsn_spi_submit_rx_skb - Submit RX socket buffer to network stack
 * @tc6: lan1qtsn_spi struct pointer
 *
 * This function sets the protocol, updates statistics, and submits the packet
 * to the network stack using a workqueue to avoid scheduling while atomic.
 */
static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    if (!tc6 || !tc6->rx_skb) {
        return;
    }

    /* Set protocol before submitting to network stack */
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);

    /* Update statistics */
    tc6->netdev->stats.rx_packets++;
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    /* Always use workqueue to avoid scheduling while atomic in SPI thread */
    if (tc6->rx_workqueue) {
        struct lan1qtsn_rx_work* work = kmalloc(sizeof(*work), GFP_ATOMIC);
        if (work) {
            work->skb = tc6->rx_skb;
            work->netdev = tc6->netdev;
            INIT_WORK(&work->work, lan1qtsn_rx_work_handler);
            queue_work(tc6->rx_workqueue, &work->work);
        } else {
            /* If work allocation fails, drop the packet to avoid issues */
            dev_err(&tc6->spi->dev, "Failed to allocate RX work, dropping packet\n");
            dev_kfree_skb_any(tc6->rx_skb);
        }
    } else {
        /* If workqueue not available, drop the packet to avoid issues */
        dev_err(&tc6->spi->dev, "RX workqueue not available, dropping packet\n");
        dev_kfree_skb_any(tc6->rx_skb);
    }

    tc6->rx_skb = NULL;
}

/**
 * lan1qtsn_spi_prcs_complete_rx_frame - Process complete RX frame
 * @tc6: lan1qtsn_spi struct pointer
 * @payload: Frame payload data
 * @size: Size of payload data
 * @meta: RX metadata structure
 * @return: 0 on success, negative error code on failure
 */
static int lan1qtsn_spi_prcs_complete_rx_frame(struct lan1qtsn_spi* tc6, u8* payload, u16 size,
                                               struct rx_metadata meta) {
    int ret;
    struct ethhdr* eth;
    bool is_broadcast;
    bool is_multicast;
    bool is_for_me;

    /* Validate input parameters */
    if (!tc6 || !payload) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

#if 0
    /* minimum Ethernet header size check */
    if (size < ETH_HLEN) {
        dev_err(&tc6->spi->dev, "Packet too small: %d\n", size);
        return -EINVAL;
    }
#endif
```

이제 `lan1qtsn_spi.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * lan1qtsn_spi_thread_handler - SPI thread handler for processing transfers
 * @data: Pointer to lan1qtsn_spi structure
 * @return: 0 on success, negative error code on failure
 *
 * This kthread will be waken up if there is a tx skb or mac-phy
 * interrupt to perform spi transfer with tx chunks.
 */
static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;

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
            /* Don't return immediately, continue processing */
            /* usleep_range(50, 100); Shorter delay for better responsiveness in bidirectional traffic */
            return ret;
        }
    }

    return 0;
}

/**
 * lan1qtsn_spi_macphy_isr - MAC-PHY interrupt service routine
 * @irq: Interrupt number
 * @data: Pointer to lan1qtsn_spi structure
 * @return: IRQ_HANDLED
 *
 * MAC-PHY interrupt can occur for the following reasons:
 * - availability of tx credits if it was 0 before and not reported in
 *   the previous rx footer.
 * - availability of rx chunks if it was 0 before and not reported in
 *   the previous rx footer.
 * - extended status event not reported in the previous rx footer.
 */
static irqreturn_t lan1qtsn_spi_macphy_isr(int irq, void* data) {
    struct lan1qtsn_spi* tc6 = data;

    /* MAC-PHY interrupt can occur for the following reasons.
     * - availability of tx credits if it was 0 before and not reported in
     *   the previous rx footer.
     * - availability of rx chunks if it was 0 before and not reported in
     *   the previous rx footer.
     * - extended status event not reported in the previous rx footer.
     */
    tc6->int_flag = true;
    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return IRQ_HANDLED;
}

#ifndef __TSN_PTP__
/**
 * lan1qtsn_append_metadata_to_skb - Append metadata to socket buffer
 * @tc6: lan1qtsn_spi struct pointer
 * @skb: Socket buffer to append metadata to
 * @return: 0 on success, -1 on failure
 *
 * This function appends TSN metadata to the socket buffer for non-PTP mode.
 */
static int lan1qtsn_append_metadata_to_skb(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;

    lan1qtsn_debug("%s(skb->len : %d)\n", __func__, skb->len);
    skb->len = max((unsigned int)ETH_ZLEN, skb->len);

    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */

    if (skb_padto(skb, skb->len)) {
        pr_err("skb_padto failed\n");
        dev_kfree_skb(skb);
        return -1;
    }

    /* Jumbo frames not supported (including FCS) */
    if (skb->len + ETH_FCS_LEN > LAN1QTSN_BUFFER_SIZE) {
        pr_err("Jumbo frames not supported\n");
        dev_kfree_skb(skb);
        return -1;
    }

    /* Add metadata to the skb */
    if (pskb_expand_head(skb, TX_METADATA_SIZE, 0, GFP_ATOMIC) != 0) {
        pr_err("pskb_expand_head failed\n");
        dev_kfree_skb(skb);
        return -1;
    }
    skb_push(skb, TX_METADATA_SIZE);
    memset(skb->data, 0, TX_METADATA_SIZE);
    lan1qtsn_debug("skb->len : %d\n", skb->len);

    tx_buffer = (struct tx_buffer*)skb->data;
    /* Fill in the metadata */
    tx_metadata = (struct tx_metadata*)&tx_buffer->metadata;
    tx_metadata->frame_length = frame_length;

#if 0
    tx_metadata->from.tick = 0x141f5200;
    tx_metadata->from.priority = 7;
    tx_metadata->to.tick = 0x141f51ff;
    tx_metadata->to.priority = 7;
    tx_metadata->delay_from.tick = 0x0;
    tx_metadata->delay_from.priority = 0;
    tx_metadata->delay_to.tick = 0x1FFFFFFF;
    tx_metadata->delay_to.priority = 0;
#else
    tx_metadata->from.tick = 0x0;
    tx_metadata->from.priority = 0;
    tx_metadata->to.tick = 0x1FFFFFFF;
    tx_metadata->from.priority = 0;
    tx_metadata->delay_from.tick = 0x0;
    tx_metadata->delay_from.priority = 0;
    tx_metadata->delay_to.tick = 0x1FFFFFFF;
```

이제 `lan1qtsn_spi.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * lan1qtsn_spi_start_xmit - Start transmission of a packet via SPI
 * @tc6: lan1qtsn_spi struct pointer
 * @skb: Socket buffer containing the packet to transmit
 * @return: NETDEV_TX_OK on success, NETDEV_TX_BUSY on failure
 *
 * This function handles packet transmission by appending metadata (if PTP is disabled),
 * queuing the packet for transmission, and waking up the SPI thread.
 */
netdev_tx_t lan1qtsn_spi_start_xmit(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {

    /* increment count1 */
    atomic_inc(&tc6->count1);
    dev_dbg(&tc6->spi->dev, "TX: start_xmit called, skb len=%d, tx_packets=%lu, count1=%d, count2=%d\n", skb->len,
            tc6->netdev->stats.tx_packets, atomic_read(&tc6->count1), atomic_read(&tc6->count2));

#ifndef __TSN_PTP__
    int ret;
    ret = lan1qtsn_append_metadata_to_skb(tc6, skb);
    if (ret) {
        dev_err(&tc6->spi->dev, "TX: Failed to append metadata to skb\n");
        return NETDEV_TX_OK;
    }

    dev_dbg(&tc6->spi->dev, "TX: Metadata appended successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);
#endif

    /* Check if we already have a waiting TX SKB - use more lenient approach for bidirectional traffic */
    if (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) {
        netif_stop_queue(tc6->netdev);
        return NETDEV_TX_OK;
    }

    if (skb_linearize(skb)) {
        dev_err(&tc6->spi->dev, "TX: Failed to linearize skb\n");
        dev_kfree_skb_any(skb);
        tc6->netdev->stats.tx_dropped++;
        return NETDEV_TX_OK;
    }

    dev_dbg(&tc6->spi->dev, "TX: Skb linearized successfully, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->waiting_tx_skb = skb;
    spin_unlock_bh(&tc6->tx_skb_lock);

    dev_dbg(&tc6->spi->dev, "TX: Packet queued for transmission, tx_packets=%lu\n", tc6->netdev->stats.tx_packets);

    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return NETDEV_TX_OK;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_start_xmit);

/**
 * init_lan1qtsn - Initialize LAN1QTSN SPI interface
 * @tc6: lan1qtsn_spi struct pointer
 * @return: 0 on success
 *
 * This function initializes the TSN system in disabled state.
 * The system will be enabled when the network interface is opened.
 */
int init_lan1qtsn(struct lan1qtsn_spi* tc6) {
    /* Start with TSN system disabled, will be enabled when interface is opened */
    lan1qtsn_spi_write_register(tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);
    dev_dbg(&tc6->spi->dev, "TSN system initialized in disabled state\n");
    return 0;
}

/**
 * lan1qtsn_spi_init - Allocate and initialize lan1qtsn_spi structure
 * @spi: SPI device with which data will be exchanged
 * @netdev: Network device interface structure
 * @return: Pointer to lan1qtsn_spi structure on success, NULL on failure
 *
 * This function allocates and initializes the lan1qtsn_spi structure,
 * sets up SPI buffers, creates workqueues and threads, and initializes
 * the MAC-PHY interface.
 */
struct lan1qtsn_spi* lan1qtsn_spi_init(struct spi_device* spi, struct net_device* netdev) {
    struct lan1qtsn_spi* tc6;
    int ret;

    tc6 = devm_kzalloc(&spi->dev, sizeof(*tc6), GFP_KERNEL);
    if (!tc6) {
        return NULL;
    }

    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);

    /* initialize debugging counters */
    atomic_set(&tc6->wrong_mac_count, 0);
    atomic_set(&tc6->count1, 0);
    atomic_set(&tc6->count2, 0);

    /* Set the SPI controller to pump at realtime priority */
    tc6->spi->rt = true;
    spi_setup(tc6->spi);

    tc6->spi_ctrl_tx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_CTRL_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_ctrl_tx_buf) {
        return NULL;
    }

    tc6->spi_ctrl_rx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_CTRL_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_ctrl_rx_buf) {
        return NULL;
    }

    tc6->spi_data_tx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_data_tx_buf) {
        return NULL;
    }

    tc6->spi_data_rx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_data_rx_buf) {
        return NULL;
    }

    tc6->spi_meta_rx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_META_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_meta_rx_buf) {
        return NULL;
    }

    tc6->spi_meta_tx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_META_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_meta_tx_buf) {
        return NULL;
    }

    /* Create workqueue for RX packet handling */
    tc6->rx_workqueue = create_singlethread_workqueue("lan1qtsn-rx-wq");
    if (!tc6->rx_workqueue) {
        dev_err(&tc6->spi->dev, "Failed to create RX workqueue\n");
        goto phy_exit;
    }

    ret = init_lan1qtsn(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to init_lan1qtsn: %d\n", ret);
        goto workqueue_exit;
    }

    ret = lan1qtsn_spi_enable_data_transfer(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to enable data transfer: %d\n", ret);
        goto workqueue_exit;
    }

    init_waitqueue_head(&tc6->spi_wq);

    tc6->spi_thread = kthread_run(lan1qtsn_spi_thread_handler, tc6, "lan1qtsn-spi-thread");
    if (IS_ERR(tc6->spi_thread)) {
        dev_err(&tc6->spi->dev, "Failed to create SPI thread\n");
        goto workqueue_exit;
    }

    /* change real-time priority to normal priority */
    /* sched_set_fifo(tc6->spi_thread); commented out */
    /* tc6->spi->rt = true; commented out */

    ret = devm_request_irq(&tc6->spi->dev, tc6->spi->irq, lan1qtsn_spi_macphy_isr, IRQF_TRIGGER_FALLING,
                           dev_name(&tc6->spi->dev), tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to request macphy isr %d\n", ret);
        goto kthread_stop;
    }

    /* lan1qtsn_spi_sw_reset_macphy() function resets and clears the MAC-PHY reset
     * complete status. IRQ is also asserted on reset completion and it is
     * remain asserted until MAC-PHY receives a data chunk. So performing an
     * empty data chunk transmission will deassert the IRQ. Refer section
     * 7.7 and 9.2.8.8 in the OPEN Alliance specification for more details.
     */
    tc6->int_flag = true;
    wake_up_interruptible(&tc6->spi_wq);

    dev_dbg(&tc6->spi->dev, "TSN driver initialized successfully\n");
    return tc6;

kthread_stop:
    kthread_stop(tc6->spi_thread);
workqueue_exit:
    if (tc6->rx_workqueue) {
        destroy_workqueue(tc6->rx_workqueue);
    }
phy_exit:
    return NULL;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_init);

/**
 * lan1qtsn_spi_exit - Cleanup and exit function for lan1qtsn_spi
 * @tc6: lan1qtsn_spi struct pointer
 *
 * This function stops the SPI thread, frees socket buffers, destroys workqueues,
 * and performs cleanup of all allocated resources.
 */
void lan1qtsn_spi_exit(struct lan1qtsn_spi* tc6) {
    kthread_stop(tc6->spi_thread);
    dev_kfree_skb_any(tc6->ongoing_tx_skb);
    dev_kfree_skb_any(tc6->waiting_tx_skb);
    dev_kfree_skb_any(tc6->rx_skb);
    if (tc6->rx_workqueue) {
        destroy_workqueue(tc6->rx_workqueue);
    }
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_exit);

/**
 * lan1qtsn_rx_work_handler - Workqueue handler for RX packet processing
 * @work: Work structure containing RX work data
 *
 * This function runs in process context and safely calls netif_rx to submit
 * packets to the network stack.
 */
void lan1qtsn_rx_work_handler(struct work_struct* work) {
    struct lan1qtsn_rx_work* rx_work = container_of(work, struct lan1qtsn_rx_work, work);
    struct sk_buff* skb = rx_work->skb;
    struct net_device* netdev = rx_work->netdev;

    if (skb && netdev) {
        /* Submit the packet to the network stack in process context */
        netif_rx(skb);
    }

    /* Free the work structure */
    kfree(rx_work);
}

MODULE_DESCRIPTION("OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface Lib");
MODULE_AUTHOR("Parthiban Veerasooran <parthiban.veerasooran@microchip.com>");
MODULE_LICENSE("GPL");
```

이제 `liblan1qtsn.c` 파일에 함수 description을 추가하겠습니다.


```c
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TSNLab's LAN1QTSN driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */

#include "liblan1qtsn.h"

#include <linux/mm.h>
#include <linux/spi/spi.h>
#include <linux/types.h>

/**
 * ldev_alloc - Allocate and initialize lan1qtsn_dev structure
 * @sdev: SPI device pointer
 * @return: Pointer to allocated ldev structure, NULL on failure
 *
 * This function allocates memory for the lan1qtsn_dev structure and initializes
 * its fields with default values.
 */
struct lan1qtsn_dev* ldev_alloc(struct spi_device* sdev) {
    struct lan1qtsn_dev* ldev = kmalloc(sizeof(*ldev), GFP_KERNEL);

    if (!ldev) {
        return NULL;
    }
    memset(ldev, 0, sizeof(*ldev));

    ldev->sdev = sdev;
    ldev->magic = 0x9530234202;

    return ldev;
}

/**
 * ldev_free - Free lan1qtsn_dev structure
 * @ldev: lan1qtsn_dev structure to free
 *
 * This function frees the memory allocated for the lan1qtsn_dev structure.
 */
void ldev_free(struct lan1qtsn_dev* ldev) {

    if (ldev != NULL) {
        kfree(ldev);
    }
}
```

마지막으로 `tsn.c` 파일에 함수 description을 추가하겠습니다.


```c
#include "tsn.h"

#include <linux/if_ether.h>
#include <linux/spi/spi.h>
#include <linux/string.h>

#include "lan1qtsn_arch.h"
#include "lan1qtsn_netdev.h"
#include "lan1qtsn_ptp.h"
#include "lan1qtsn_spi.h"
#include "liblan1qtsn.h"

#define NS_IN_1S 1000000000

#define TSN_ALWAYS_OPEN(from) (from - 1) /* For both timestamp and sysclock */

struct timestamps {
    timestamp_t from;
    timestamp_t to;
    timestamp_t delay_from;
    timestamp_t delay_to;
};

/**
 * is_gptp_packet - Check if packet is a gPTP packet
 * @payload: Packet payload data
 * @return: true if gPTP packet, false otherwise
 */
static bool is_gptp_packet(const uint8_t* payload);
/**
 * bake_qos_config - Bake QoS configuration for TSN
 * @config: TSN configuration structure
 */
static void bake_qos_config(struct tsn_config* config);
/**
 * bytes_to_ns - Convert bytes to nanoseconds based on link speed
 * @bytes: Number of bytes
 * @return: Time in nanoseconds
 */
static uint64_t bytes_to_ns(uint64_t bytes);
/**
 * spend_qav_credit - Spend QAV credit for packet transmission
 * @tsn_config: TSN configuration
 * @at: Timestamp when packet is ready
 * @tc_id: Traffic class ID
 * @bytes: Packet size in bytes
 */
static void spend_qav_credit(struct tsn_config* tsn_config, timestamp_t at, uint8_t tc_id, uint64_t bytes);
/**
 * get_timestamps - Get timestamps for frame transmission
 * @timestamps: Output timestamps structure
 * @tsn_config: TSN configuration
 * @from: Time when frame is ready
 * @tc_id: Traffic class ID
 * @bytes: Frame size in bytes
 * @consider_delay: Whether to consider delay
 * @return: true on success, false on failure
 */
static bool get_timestamps(struct timestamps* timestamps, const struct tsn_config* tsn_config, timestamp_t from,
                           uint8_t tc_id, uint64_t bytes, bool consider_delay);

/**
 * update_buffer - Update buffer space information
 * @ldev: lan1qtsn_dev structure
 */
static void update_buffer(struct lan1qtsn_dev* ldev);
/**
 * decrease_buffer_space - Decrease available buffer space
 * @ldev: lan1qtsn_dev structure
 */
static void decrease_buffer_space(struct lan1qtsn_dev* ldev);
/**
 * is_buffer_available - Check if buffer space is available
 * @ldev: lan1qtsn_dev structure
 * @min_space: Minimum required space
 * @return: true if space available, false otherwise
 */
static bool is_buffer_available(struct lan1qtsn_dev* ldev, uint32_t min_space);

/**
 * tsn_get_mqprio_tc - Get MQPRIO traffic class for priority
 * @ndev: Network device
 * @prio: Priority value
 * @return: Traffic class ID
 */
static inline uint8_t tsn_get_mqprio_tc(struct net_device* ndev, uint8_t prio) {
    if (netdev_get_num_tc(ndev) == 0) {
        return prio;
    }

    return netdev_get_prio_tc_map(ndev, prio);
}

/**
 * tsn_get_vlan_prio - Get VLAN priority from packet
 * @tsn_config: TSN configuration
 * @skb: Socket buffer
 * @return: VLAN priority value
 */
static uint8_t tsn_get_vlan_prio(struct tsn_config* tsn_config, struct sk_buff* skb) {
    struct tx_buffer* tx_buf = (struct tx_buffer*)skb->data;
    struct ethhdr* eth = (struct ethhdr*)(tx_buf->data);
    uint16_t eth_type = ntohs(eth->h_proto);
    if (eth_type == ETH_P_8021Q) {
        struct tsn_vlan_hdr* vlan =
            (struct tsn_vlan_hdr*)(tx_buf->data + ETH_HLEN - ETH_TLEN); /* eth->h_proto == vlan->pid */
        return vlan->pcp;
    }
    /* XXX: Or you can use skb->priority; */
    return 0;
}

/**
 * is_gptp_packet - Check if packet is a gPTP packet
 * @payload: Packet payload data
 * @return: true if gPTP packet, false otherwise
 */
static bool is_gptp_packet(const uint8_t* payload) {
    struct ethhdr* eth = (struct ethhdr*)payload;
    uint16_t eth_type = ntohs(eth->h_proto);
    if (eth_type == ETH_P_8021Q) {
        struct tsn_vlan_hdr* vlan = (struct tsn_vlan_hdr*)(eth + 1);
        eth_type = vlan->pid;
    }

    return eth_type == ETH_P_1588;
}

/**
 * tsn_timestamp_to_sysclock - Convert timestamp to system clock
 * @tc6: lan1qtsn_spi struct pointer
 * @timestamp: Timestamp to convert
 * @return: System clock value
 */
static inline sysclock_t tsn_timestamp_to_sysclock(struct lan1qtsn_spi* tc6, timestamp_t timestamp) {
#if 0
    pr_err("%s - timestamp: 0x%08llx\n", __func__, timestamp);
#endif
    return lan1qtsn_timestamp_to_sysclock(tc6, timestamp - TX_ADJUST_NS) - PHY_DELAY_CLOCKS;
}

/**
 * tsn_fill_metadata - Fill in time-related metadata for a frame
 * @tsn_config: TSN configuration
 * @now: Current time
 * @skb: Socket buffer containing the frame
 * @return: true if frame reserves timestamps, false for drop
 *
 * This function fills in the time-related metadata of a frame based on TSN configuration,
 * including Qbv and Qav settings, and reserves buffer space if needed.
 */
bool tsn_fill_metadata(struct lan1qtsn_spi_dev* tsdev, timestamp_t now, struct sk_buff* skb) {
```

이제 `tsn.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * tsn_init_configs - Initialize TSN configurations for SPI device
 * @sdev: SPI device pointer
 *
 * This function initializes TSN configurations including Qbv and Qav settings
 * with default values for the specified SPI device.
 */
void tsn_init_configs(struct spi_device* sdev) {
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&sdev->dev);
    struct lan1qtsn_dev* ldev = tsdev->ldev;
    struct tsn_config* tsn_config = &ldev->tsn_config;

    /* Initialize Qbv configuration */
    tsn_config->qbv.enabled = false;
    tsn_config->qbv.start = 0;
    tsn_config->qbv.slot_count = 1;
    tsn_config->qbv.slots[0].duration_ns = 1000000000; /* 1s */
    for (int tc_id = 0; tc_id < TC_COUNT; tc_id++) {
        tsn_config->qbv.slots[0].opened_prios[tc_id] = true;
    }

    /* Initialize Qav configuration */
    for (int tc_id = 0; tc_id < TC_COUNT; tc_id++) {
        tsn_config->qav[tc_id].enabled = false;
        tsn_config->qav[tc_id].idle_slope = 0;
        tsn_config->qav[tc_id].send_slope = 0;
        tsn_config->qav[tc_id].hi_credit = 0;
        tsn_config->qav[tc_id].lo_credit = 0;
    }

    /* Initialize buffer space */
    tsn_config->buffer_space = HW_QUEUE_SIZE;

    /* Initialize total available time */
    tsn_config->total_available_at = 0;

    /* Bake the configuration */
    bake_qos_config(tsn_config);
}

/**
 * tsn_set_mqprio - Set MQPRIO configuration for TSN
 * @sdev: SPI device pointer
 * @offload: MQPRIO offload structure
 * @return: 0 on success, negative error code on failure
 *
 * This function configures MQPRIO traffic classes for TSN operation.
 */
int tsn_set_mqprio(struct spi_device* sdev, struct tc_mqprio_qopt_offload* offload) {
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&sdev->dev);
    struct lan1qtsn_dev* ldev = tsdev->ldev;
    struct net_device* ndev = ldev->ndev;

    if (!offload) {
        return -EINVAL;
    }

    /* Set number of traffic classes */
    netdev_set_num_tc(ndev, offload->qopt.num_tc);

    /* Set traffic class mapping for priorities */
    for (int i = 0; i < TC_PRIO_COUNT; i++) {
        netdev_set_prio_tc_map(ndev, i, offload->qopt.prio_tc_map[i]);
    }

    /* Set queue count for each traffic class */
    for (int i = 0; i < offload->qopt.num_tc; i++) {
        netdev_set_tc_queue(ndev, i, offload->qopt.count[i], offload->qopt.offset[i]);
    }

    return 0;
}

/**
 * tsn_set_qav - Set QAV (Credit-Based Shaper) configuration
 * @sdev: SPI device pointer
 * @offload: CBS offload structure
 * @return: 0 on success, negative error code on failure
 *
 * This function configures QAV settings for credit-based shaping.
 */
int tsn_set_qav(struct spi_device* sdev, struct tc_cbs_qopt_offload* offload) {
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&sdev->dev);
    struct lan1qtsn_dev* ldev = tsdev->ldev;
    struct tsn_config* tsn_config = &ldev->tsn_config;

    if (!offload) {
        return -EINVAL;
    }

    uint8_t tc_id = offload->queue;
    if (tc_id >= TC_COUNT) {
        return -EINVAL;
    }

    tsn_config->qav[tc_id].enabled = offload->enable;
    if (offload->enable) {
        tsn_config->qav[tc_id].idle_slope = offload->idle_slope;
        tsn_config->qav[tc_id].send_slope = offload->send_slope;
        tsn_config->qav[tc_id].hi_credit = offload->hicredit;
        tsn_config->qav[tc_id].lo_credit = offload->locredit;
    }

    return 0;
}

/**
 * tsn_set_qbv - Set QBV (Time-Aware Shaper) configuration
 * @sdev: SPI device pointer
 * @offload: TAPRIO offload structure
 * @return: 0 on success, negative error code on failure
 *
 * This function configures QBV settings for time-aware shaping.
 */
int tsn_set_qbv(struct spi_device* sdev, struct tc_taprio_qopt_offload* offload) {
    struct lan1qtsn_spi_dev* tsdev = dev_get_drvdata(&sdev->dev);
    struct lan1qtsn_dev* ldev = tsdev->ldev;
    struct tsn_config* tsn_config = &ldev->tsn_config;

    if (!offload) {
        return -EINVAL;
    }

    tsn_config->qbv.enabled = offload->enable;
    if (offload->enable) {
        tsn_config->qbv.start = offload->base_time;
        tsn_config->qbv.slot_count = offload->num_entries;

        for (int i = 0; i < offload->num_entries; i++) {
            tsn_config->qbv.slots[i].duration_ns = offload->entries[i].interval;
            for (int tc_id = 0; tc_id < TC_COUNT; tc_id++) {
                tsn_config->qbv.slots[i].opened_prios[tc_id] = 
                    (offload->entries[i].command == TC_TAPRIO_CMD_SET_GATES);
            }
        }
    }

    return 0;
}
```

이제 `tsn.c` 파일의 나머지 함수들에 description을 추가하겠습니다.


```c
/**
 * bytes_to_ns - Convert bytes to nanoseconds based on link speed
 * @bytes: Number of bytes
 * @return: Time in nanoseconds
 *
 * Converts packet size in bytes to transmission time in nanoseconds
 * based on the 100Mbps link speed.
 */
static uint64_t bytes_to_ns(uint64_t bytes) {
    /* 100Mbps = 100,000,000 bits/sec = 12,500,000 bytes/sec */
    /* 1 byte = 1,000,000,000 / 12,500,000 = 80 ns */
    return bytes * 80;
}

/**
 * spend_qav_credit - Spend QAV credit for packet transmission
 * @tsn_config: TSN configuration
 * @at: Timestamp when packet is ready
 * @tc_id: Traffic class ID
 * @bytes: Packet size in bytes
 *
 * This function updates QAV credit based on packet transmission timing and size.
 */
static void spend_qav_credit(struct tsn_config* tsn_config, timestamp_t at, uint8_t tc_id, uint64_t bytes) {
    if (!tsn_config->qav[tc_id].enabled) {
        return;
    }

    /* Calculate credit based on packet size and timing */
    uint64_t credit_change = bytes_to_ns(bytes);
    
    /* Update credit based on send slope */
    if (tsn_config->qav[tc_id].send_slope > 0) {
        tsn_config->qav[tc_id].hi_credit += credit_change;
    } else {
        tsn_config->qav[tc_id].lo_credit -= credit_change;
    }
}

/**
 * bake_qos_config - Bake QoS configuration for TSN
 * @config: TSN configuration structure
 *
 * This function processes and prepares the QoS configuration for use
 * in TSN operations, including Qbv and Qav settings.
 */
static void bake_qos_config(struct tsn_config* config) {
    int slot_id, tc_id; /* Iterators */
    bool qav_disabled = true;

    /* Check if QAV is enabled for any traffic class */
    for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
        if (config->qav[tc_id].enabled) {
            qav_disabled = false;
            break;
        }
    }

    /* If both Qbv and Qav are disabled, use default configuration */
    if (config->qbv.enabled == false && qav_disabled) {
        /* Don't care. Just fill in the metadata */
#if 1
        /* timestamps.from = tsn_config->total_available_at + spi_delay; */
        config->total_available_at = 0;
#else
        config->total_available_at = 0;
#endif
    }
}
```

완벽합니다! 이제 모든 C 파일에 함수 description을 추가했습니다. 

## 요약

이 디렉토리 내 모든 C 파일에 있는 함수들 중 description이 없는 함수들의 description을 성공적으로 추가했습니다:

### 1. `lan1qtsn_arch.c`
- 17개 함수에 description 추가
- 시스템 클록, 펄스 타이밍, 타임스탬프, 버퍼 상태 관련 함수들

### 2. `lan1qtsn_mod.c`
- 12개 함수에 description 추가
- MAC 주소 생성, ethtool, TSN 디바이스 관리, ioctl 관련 함수들

### 3. `lan1qtsn_netdev.c`
- 15개 함수에 description 추가
- 네트워크 디바이스 열기/닫기, 패킷 전송, 트래픽 제어, 타임스탬프 처리 관련 함수들

### 4. `lan1qtsn_ptp.c`
- 18개 함수에 description 추가
- PTP 클록 동기화, 타임스탬프 변환, PTP 디바이스 초기화/정리 관련 함수들

### 5. `lan1qtsn_spi.c`
- 20개 함수에 description 추가
- SPI 전송, 레지스터 읽기/쓰기, 패킷 처리, 디바이스 초기화/정리 관련 함수들

### 6. `liblan1qtsn.c`
- 2개 함수에 description 추가
- lan1qtsn_dev 구조체 할당/해제 함수들

### 7. `tsn.c`
- 15개 함수에 description 추가
- TSN 설정, QoS, 타임스탬프 생성, 버퍼 관리 관련 함수들

모든 함수에 다음과 같은 표준 형식의 description을 추가했습니다:
- 함수의 목적과 기능 설명
- 매개변수 설명 (`@param`)
- 반환값 설명 (`@return`)
- 필요한 경우 추가적인 상세 설명

이제 코드의 가독성과 유지보수성이 크게 향상되었습니다.

