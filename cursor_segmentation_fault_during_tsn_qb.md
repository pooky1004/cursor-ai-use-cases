# Segmentation fault during TSN Qbv setup
_Exported on 8/18/2025 at 15:07:37 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있어.

아래 명령을 사용하여 TSN Qbv 설정을 할 때 Segmentation fault 가 발생해

원인을 분석해 주고 수정해 줘

sudo tc qdisc replace dev eth1 parent root handle 100 taprio \
        num_tc 2 \
        map 0 0 1 1 0 1 0 0 0 0 0 0 0 0 0 0 \
        queues 1@0 1@1 \
        base-time 0 \
        sched-entry S 01 500000000 \
        sched-entry S 00 500000000 \
        flags 0x2 \
        txtime-delay 0


[  200.614688] Unable to handle kernel NULL pointer dereference at virtual address 00000000000003ac
[  200.614722] Mem abort info:
[  200.614728]   ESR = 0x0000000096000005
[  200.614735]   EC = 0x25: DABT (current EL), IL = 32 bits
[  200.614742]   SET = 0, FnV = 0
[  200.614748]   EA = 0, S1PTW = 0
[  200.614754]   FSC = 0x05: level 1 translation fault
[  200.614760] Data abort info:
[  200.614765]   ISV = 0, ISS = 0x00000005, ISS2 = 0x00000000
[  200.614771]   CM = 0, WnR = 0, TnD = 0, TagAccess = 0
[  200.614777]   GCS = 0, Overlay = 0, DirtyBit = 0, Xs = 0
[  200.614784] user pgtable: 4k pages, 39-bit VAs, pgdp=0000000116d8b000
[  200.614792] [00000000000003ac] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[  200.614812] Internal error: Oops: 0000000096000005 [#1] PREEMPT SMP
[  200.614822] Modules linked in: sch_taprio sch_mqprio_lib lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc spidev brcmfmac_wcc joydev hci_uart btbcm bluetooth brcmfmac vc4 bcm2835_v4l2(C) rpi_hevc_dec bcm2835_codec(C) bcm2835_isp(C) brcmutil snd_soc_hdmi_codec bcm2835_mmal_vchiq(C) cfg80211 drm_display_helper v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc v3d cec videobuf2_dma_contig videobuf2_memops drm_dma_helper videobuf2_v4l2 ecdh_generic gpu_sched snd_soc_core ecc videodev rfkill snd_compress libaes drm_shmem_helper snd_bcm2835(C) snd_pcm_dmaengine drm_kms_helper snd_pcm videobuf2_common raspberrypi_hwmon snd_timer i2c_brcmstb mc snd spi_bcm2835 raspberrypi_gpiomem nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern drm_panel_orientation_quirks backlight dm_mod fuse ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[  200.615092] CPU: 0 UID: 0 PID: 2884 Comm: tc Tainted: G         C O       6.12.41-v8+ #8
[  200.615106] Tainted: [C]=CRAP, [O]=OOT_MODULE
[  200.615112] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[  200.615118] pstate: 60000005 (nZCv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  200.615128] pc : ptp_clock_index+0x8/0x18
[  200.615147] lr : lan1qtsn_ethtool_get_ts_info+0x30/0x68 [lan1qtsn]
[  200.615171] sp : ffffffc08464b580
[  200.615177] x29: ffffffc08464b580 x28: 0000000000000000 x27: ffffff81063f85c0
[  200.615193] x26: ffffff81063f85f6 x25: 0000000000000000 x24: ffffff8102424600
[  200.615208] x23: ffffff8101b92000 x22: ffffffdd42e4d1e8 x21: ffffff804475a580
[  200.615223] x20: ffffffc08464b970 x19: ffffffc08464b5cc x18: 0000007fb375efff
[  200.615237] x17: ffffffa38957f000 x16: ffffffdd7a08b8a0 x15: ffffff81063f85f0
[  200.615251] x14: ffffff8101b93000 x13: 0000000000000000 x12: 0000000000000000
[  200.615265] x11: 0000000000000040 x10: ffffffffffffffff x9 : ffffffdd42d12260
[  200.615279] x8 : ffffff8102424710 x7 : 0000000000000001 x6 : ffffff81024245d0
[  200.615293] x5 : 0000000000000041 x4 : ffffff8102424a00 x3 : 0000000000000000
[  200.615307] x2 : ffffffdd42d12230 x1 : ffffffc08464b5cc x0 : 0000000000000000
[  200.615321] Call trace:
[  200.615328]  ptp_clock_index+0x8/0x18
[  200.615338]  taprio_change+0x540/0xaf0 [sch_taprio]
[  200.615354]  taprio_init+0x1dc/0x298 [sch_taprio]
[  200.615365]  qdisc_create+0x164/0x510
[  200.615380]  tc_modify_qdisc+0x390/0x798
[  200.615391]  rtnetlink_rcv_msg+0x138/0x3b8
[  200.615403]  netlink_rcv_skb+0x68/0x140
[  200.615415]  rtnetlink_rcv+0x20/0x38
[  200.615423]  netlink_unicast+0x330/0x3b8
[  200.615435]  netlink_sendmsg+0x19c/0x3f8
[  200.615447]  __sock_sendmsg+0x64/0xc0
[  200.615461]  ____sys_sendmsg+0x268/0x2a0
[  200.615470]  ___sys_sendmsg+0xb8/0x118
[  200.615481]  __sys_sendmsg+0x90/0xf8
[  200.615492]  __arm64_sys_sendmsg+0x2c/0x40
[  200.615502]  invoke_syscall+0x50/0x120
[  200.615515]  el0_svc_common.constprop.0+0x48/0xf0
[  200.615526]  do_el0_svc+0x24/0x38
[  200.615537]  el0_svc+0x30/0x100
[  200.615555]  el0t_64_sync_handler+0x100/0x130
[  200.615566]  el0t_64_sync+0x190/0x198
[  200.615578] Code: 7a4052a0 ffffffdd aa1e03e9 d503201f (b943ac00)
[  200.615588] ---[ end trace 0000000000000000 ]---

---

**Cursor**

커널 로그를 분석해보니 TSN Qbv 설정 시 발생하는 Segmentation fault의 원인을 파악했습니다. 문제는 `ptp_clock_index+0x8/0x18`에서 NULL 포인터 역참조가 발생하는 것입니다.

먼저 관련 코드를 살펴보겠습니다.


이제 `lan1qtsn_ethtool_get_ts_info` 함수를 찾아보겠습니다.


문제를 찾았습니다! `lan1qtsn_ethtool_get_ts_info` 함수에서 `tsdev->ptp->ptp_clock`가 NULL인 상태에서 `ptp_clock_index()`를 호출하고 있습니다. 

이제 `tsdev->ptp`가 어떻게 초기화되는지 확인해보겠습니다.


이제 `__TSN_PTP__` 매크로가 정의되어 있는지 확인해보겠습니다.


Makefile에서 `__TSN_PTP__`가 정의되어 있지만, 실제로 PTP 초기화가 제대로 되지 않는 것 같습니다. 이제 문제를 수정해보겠습니다.

문제는 `lan1qtsn_ethtool_get_ts_info` 함수에서 `tsdev->ptp`가 NULL인 상태에서 `ptp_clock_index()`를 호출하는 것입니다. 이를 수정해야 합니다.


```c
static int lan1qtsn_ethtool_get_ts_info(struct net_device *ndev,
					struct kernel_ethtool_ts_info *info)
{
	struct lan1qtsn_priv *priv = netdev_priv(ndev);
	struct lan1qtsn_spi_dev *tsdev = dev_get_drvdata(&priv->sdev->dev);

	// Check if PTP is available before accessing it
	if (!tsdev || !tsdev->ptp || !tsdev->ptp->ptp_clock) {
		info->phc_index = -1;
	} else {
		info->phc_index = ptp_clock_index(tsdev->ptp->ptp_clock);
	}

	info->so_timestamping =
		SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE |
		SOF_TIMESTAMPING_SOFTWARE | SOF_TIMESTAMPING_TX_HARDWARE |
		SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_RAW_HARDWARE;

	info->tx_types = BIT(HWTSTAMP_TX_OFF) | BIT(HWTSTAMP_TX_ON);

	info->rx_filters = BIT(HWTSTAMP_FILTER_NONE) |
			   BIT(HWTSTAMP_FILTER_ALL) |
			   BIT(HWTSTAMP_FILTER_PTP_V2_L2_EVENT) |
			   BIT(HWTSTAMP_FILTER_PTP_V2_L2_SYNC) |
			   BIT(HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ);

	return 0;
}
```

이제 PTP 초기화 부분도 확인해보겠습니다. `ptp_device_init` 함수가 실패할 수 있는 경우를 더 안전하게 처리해야 합니다.


PTP 초기화 실패 시 더 안전한 처리를 위해 코드를 수정하겠습니다.


```c
#ifdef __TSN_PTP__
#if 0
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
```

이제 PTP 관련 함수들에서도 NULL 체크를 추가해야 합니다. `lan1qtsn_ptp.c` 파일의 함수들을 수정하겠습니다.


```c
sysclock_t lan1qtsn_timestamp_to_sysclock(struct lan1qtsn_spi* tc6, timestamp_t timestamp) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0;
    }

    long scaled_val = (long)(ptp_data->ticks_scale * 1000000);
    pr_err("%s - ptp_data: %p, timestamp: 0x%08llx, ptp_data->offset: 0x%016llx, ptp_data->ticks_scale: %ld.%06ld\n",
           __func__, ptp_data, timestamp, ptp_data->offset, scaled_val / 1000000, scaled_val % 1000000);
    return (timestamp - ptp_data->offset) / ptp_data->ticks_scale;
}

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
```

```c
timestamp_t lan1qtsn_get_rx_timestamp(struct lan1qtsn_spi* tc6, sysclock_t sysclock) {
    return lan1qtsn_sysclock_to_timestamp(tc6, sysclock) - RX_ADJUST_NS;
}

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

timestamp_t lan1qtsn_sysclock_to_txtstamp(struct lan1qtsn_spi* tc6, sysclock_t sysclock) {
    return lan1qtsn_sysclock_to_timestamp(tc6, sysclock) + TX_ADJUST_NS;
}

double lan1qtsn_get_ticks_scale(struct lan1qtsn_spi* tc6) {
    struct lan1qtsn_spi_dev* tsdev = tc6->tsdev;
    struct ptp_device_data* ptp_data = tsdev->ptp;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0.0;
    }

    return ptp_data->ticks_scale;
}

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
```

이제 `ptp_device_init` 함수에서도 더 안전한 초기화를 위해 수정하겠습니다.


```c
struct ptp_device_data* ptp_device_init(struct device* dev, struct lan1qtsn_dev* ldev) {
    struct ptp_device_data* ptp;
    struct timespec64 ts;
#ifdef __LIBXDMA_DEBUG__
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

#ifdef __LIBXDMA_DEBUG__
    ptp->ptp_id = ptp_cnt++;
#endif

    /* Set offset, cycle_1s */
    ts = ktime_to_timespec64(ktime_get_real());
    lan1qtsn_ptp_settime(&ptp->ptp_info, &ts);

    return ptp;
}
```

이제 `ptp_device_destroy` 함수도 안전하게 수정하겠습니다.


```c
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

이제 `lan1qtsn_remove` 함수에서도 PTP 정리를 안전하게 처리하도록 수정하겠습니다.


`lan1qtsn_remove` 함수에서 PTP 정리를 추가하겠습니다.


```c
static void lan1qtsn_remove(struct spi_device *spi)
{
	struct lan1qtsn_priv *priv = spi_get_drvdata(spi);
	struct lan1qtsn_spi_dev *tsdev = dev_get_drvdata(&priv->sdev->dev);

	unregister_netdev(priv->ndev);
	lan1qtsn_spi_exit(priv->tc6);
	
#ifdef __TSN_PTP__
	// Clean up PTP device if it exists
	if (tsdev && tsdev->ptp) {
		ptp_device_destroy(tsdev->ptp);
		tsdev->ptp = NULL;
	}
#endif
	
	free_netdev(priv->ndev);
	misc_deregister(&lan1qtsn_miscdev);
}
```

이제 `tsdev_free` 함수도 확인해보겠습니다.


`tsdev_free` 함수에서도 PTP 정리를 추가하겠습니다.


```c
static void tsdev_free(struct lan1qtsn_spi_dev *tsdev)
{
	struct lan1qtsn_dev *ldev = tsdev->ldev;

	pr_info("tsdev 0x%p, destroy_interfaces, ldev 0x%p.\n", tsdev, ldev);
	
#ifdef __TSN_PTP__
	// Clean up PTP device if it exists
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
```

이제 수정된 코드를 컴파일해보겠습니다.


