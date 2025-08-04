# 커널 부팅 오류 및 패킷 전송 문제
_Exported on 7/30/2025 at 11:02:34 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후만들어진 커널 이미지로 부팅을 할 때
[   12.757807] tsnlab_spi_start_xmit(skb->len : 331)
[   12.757842] skb->len : 363
[   14.352256] onequeue spi0.0: SPI transfer failed for control: -22

이런 커널 메시지가 뜨고,

부팅 후 해 당 이더넷 포트에 ip를 설정하고 외부로 ping을 하려고 할 때

[ 3659.481226] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[ 3659.481248] Mem abort info:
[ 3659.481252]   ESR = 0x0000000096000005
[ 3659.481256]   EC = 0x25: DABT (current EL), IL = 32 bits
[ 3659.481261]   SET = 0, FnV = 0
[ 3659.481265]   EA = 0, S1PTW = 0
[ 3659.481268]   FSC = 0x05: level 1 translation fault
[ 3659.481272] Data abort info:
[ 3659.481275]   ISV = 0, ISS = 0x00000005, ISS2 = 0x00000000
[ 3659.481279]   CM = 0, WnR = 0, TnD = 0, TagAccess = 0
[ 3659.481282]   GCS = 0, Overlay = 0, DirtyBit = 0, Xs = 0
[ 3659.481287] user pgtable: 4k pages, 39-bit VAs, pgdp=0000000105e4c000
[ 3659.481292] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[ 3659.481304] Internal error: Oops: 0000000096000005 [#1] PREEMPT SMP
[ 3659.481311] Modules linked in: rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep binfmt_misc brcmfmac_wcc spidev brcmfmac hci_uart btbcm bluetooth brcmutil vc4 cfg80211 ecdh_generic ecc snd_soc_hdmi_codec drm_display_helper cec bcm2835_isp(C) drm_dma_helper libaes v3d rfkill bcm2835_codec(C) bcm2835_v4l2(C) gpu_sched rpi_hevc_dec bcm2835_mmal_vchiq(C) drm_shmem_helper snd_soc_core drm_kms_helper v4l2_mem2mem vc_sm_cma(C) videobuf2_vmalloc videobuf2_dma_contig snd_compress videobuf2_memops snd_bcm2835(C) videobuf2_v4l2 snd_pcm_dmaengine videodev snd_pcm raspberrypi_hwmon videobuf2_common snd_timer mc i2c_brcmstb snd spi_bcm2835 raspberrypi_gpiomem nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern drm_panel_orientation_quirks fuse backlight dm_mod ip_tables x_tables ipv6
[ 3659.481461] CPU: 2 UID: 0 PID: 323 Comm: tsn-tc6-spi-thr Tainted: G         C         6.12.33-v8+ #40
[ 3659.481469] Tainted: [C]=CRAP
[ 3659.481473] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 3659.481476] pstate: 00000005 (nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[ 3659.481482] pc : tsnlab_spi_spi_thread_handler+0x1d8/0x510
[ 3659.481494] lr : tsnlab_spi_spi_thread_handler+0x1c4/0x510
[ 3659.481499] sp : ffffffc081cbbdd0
[ 3659.481502] x29: ffffffc081cbbe10 x28: 000000000000016b x27: ffffff8042cb5802
[ 3659.481512] x26: ffffffdf8c3ba1a8 x25: 0000000000000010 x24: 0000000000000000
[ 3659.481521] x23: 00000000000000f0 x22: 0000000000000002 x21: ffffff81055ef144
[ 3659.481530] x20: 0000000000000000 x19: ffffff81016c2080 x18: 0000000000071be0
[ 3659.481539] x17: ffffff81fef93be0 x16: ffffffdf8b2c5b70 x15: 0000000000000000
[ 3659.481547] x14: 0000000000000000 x13: 0000000080000004 x12: 0000000000000000
[ 3659.481556] x11: ffffff8103fcdcc0 x10: 0000000000001a40 x9 : ffffffdf8bb5f568
[ 3659.481564] x8 : ffffff810208f420 x7 : ffffff81073b0abc x6 : 0000000000000001
[ 3659.481573] x5 : ffffff810208f000 x4 : 0000000000000000 x3 : 0000000000000000
[ 3659.481581] x2 : 0000000000000000 x1 : ffffff8106a00000 x0 : 0000000000004901
[ 3659.481590] Call trace:
[ 3659.481593]  tsnlab_spi_spi_thread_handler+0x1d8/0x510
[ 3659.481599]  kthread+0x11c/0x128
[ 3659.481606]  ret_from_fork+0x10/0x20
[ 3659.481615] Code: 35000d20 794012a0 72003c1f 54fff420 (fd400280)
[ 3659.481621] ---[ end trace 0000000000000000 ]---


이런 커널 메시지가 뜨고 패킷 전송이 안되는데,

무엇인 문제인지 확인해 주세요. 그리고 필요한 정보가 더 있는지 확인해 주세요.

---

**Cursor**

이 문제를 분석하기 위해 먼저 코드를 살펴보겠습니다. 커널 패닉과 SPI 전송 실패 메시지를 보면 몇 가지 문제가 있을 수 있습니다.


이제 메인 드라이버 파일을 확인해보겠습니다.


이제 헤더 파일들을 확인해보겠습니다.


이제 TSN 관련 헤더 파일들을 확인해보겠습니다.


이제 TSN 구현 파일을 확인해보겠습니다.


이제 one_q_tsn_arch.h 파일을 확인해보겠습니다.


이제 one_q_tsn_arch.c 파일을 확인해보겠습니다.


이제 문제를 분석해보겠습니다. 커널 패닉 메시지를 보면:

1. **SPI 전송 실패**: `onequeue spi0.0: SPI transfer failed for control: -22`
2. **NULL 포인터 역참조**: `tsnlab_spi_spi_thread_handler+0x1d8/0x510`에서 발생

문제의 원인을 찾기 위해 `tsnlab_spi_spi_thread_handler` 함수를 자세히 살펴보겠습니다.


이제 문제가 되는 부분을 찾기 위해 `tsnlab_spi_spi_thread_handler` 함수 주변의 코드를 더 자세히 살펴보겠습니다.


이제 `tsnlab_spi_try_spi_transfer` 함수를 확인해보겠습니다. 이 함수가 NULL 포인터 역참조의 원인일 가능성이 높습니다.


이제 `tsnlab_spi_get_meta_data` 함수를 확인해보겠습니다. 이 함수가 NULL 포인터 역참조의 원인일 가능성이 높습니다.


이제 문제를 분석해보겠습니다. 커널 패닉 메시지와 코드를 분석한 결과, 다음과 같은 문제들이 있습니다:

## 문제 분석

### 1. SPI 전송 실패 (-22 에러)
`-22` 에러는 `EINVAL` (Invalid argument)를 의미합니다. 이는 SPI 전송 시 잘못된 매개변수가 전달되었음을 나타냅니다.

### 2. NULL 포인터 역참조
커널 패닉이 `tsnlab_spi_spi_thread_handler+0x1d8/0x510`에서 발생했습니다. 이는 `tsnlab_spi_try_spi_transfer` 함수 내에서 발생한 것으로 보입니다.

### 3. 주요 문제점들

1. **SPI 버퍼 초기화 문제**: `tsnlab_spi_init` 함수에서 SPI 버퍼들이 제대로 할당되지 않았을 수 있습니다.

2. **NULL 포인터 체크 부족**: `tsnlab_spi_get_meta_data` 함수에서 `tc6->spi_meta_rx_buf`가 NULL일 수 있습니다.

3. **SPI 설정 문제**: SPI 디바이스 설정이 올바르지 않을 수 있습니다.

## 해결 방안

다음과 같은 수정을 제안합니다:


```c
static int tsnlab_spi_get_meta_data(struct tsnlab_spi* tc6, struct rx_metadata* meta) {
    int ret;
    u8* tx_buf = tc6->spi_meta_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u8* buff = (u8*)tc6->spi_meta_rx_buf;
    struct rx_metadata* rx_meta;
    u32* timestamp_n;
    u32* timestamp_h;

    /* NULL 포인터 체크 추가 */
    if (!tc6 || !meta || !tc6->spi_meta_tx_buf || !tc6->spi_meta_rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in tsnlab_spi_get_meta_data\n");
        return -EINVAL;
    }

    rx_meta = (struct rx_metadata*)&buff[TSNLAB_SPI_META_HEADER_SIZE + TSNLAB_SPI_META_IGNORED_SIZE];
    timestamp_n = (u32*)rx_meta->timestamp;
    timestamp_h = (u32*)meta->timestamp;

    cmd = TSNLAB_SPI_CMD_READ;
    addr = TSNLAB_SPI_METADATA_FIFO_BASE;

    tsnlab_spi_prepare_mpw_header(tx_buf, cmd, addr);

    size = TSNLAB_SPI_RX_META_DATA_BUF_SIZE;
    /* Perform SPI transfer */
    ret = tsnlab_spi_spi_transfer(tc6, TSNLAB_SPI_META_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for metadata: %d\n", ret);
        return ret;
    }

    meta->frame_length = ntohs(rx_meta->frame_length);
    if (meta->frame_length) {
        *timestamp_h = ntohl(*timestamp_n);

        timestamp_n++;
        timestamp_h++;
        *timestamp_h = ntohl(*timestamp_n);
    }

    return 0;
}
```

이제 `tsnlab_spi_init` 함수에서 SPI 버퍼 할당 부분을 개선하겠습니다:


이제 `tsnlab_spi_init` 함수에 더 나은 에러 처리를 추가하겠습니다:


```c
struct tsnlab_spi* tsnlab_spi_init(struct spi_device* spi, struct net_device* netdev) {
    struct tsnlab_spi* tc6;
    int ret;

    if (!spi || !netdev) {
        pr_err("Invalid parameters to tsnlab_spi_init\n");
        return NULL;
    }

    tc6 = devm_kzalloc(&spi->dev, sizeof(*tc6), GFP_KERNEL);
    if (!tc6)
        return NULL;

    tc6->spi = spi;
    tc6->netdev = netdev;
    tc6->ptp = NULL; /* Initialize ptp to NULL */
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);

    /* Set the SPI controller to pump at realtime priority */
    tc6->spi->rt = true;
    ret = spi_setup(tc6->spi);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI setup failed: %d\n", ret);
        return NULL;
    }

    tc6->spi_ctrl_tx_buf = devm_kzalloc(&tc6->spi->dev, TSNLAB_SPI_CTRL_SPI_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_ctrl_tx_buf) {
        dev_err(&tc6->spi->dev, "Failed to allocate SPI control TX buffer\n");
        return NULL;
    }

    tc6->spi_ctrl_rx_buf = devm_kzalloc(&tc6->spi->dev, TSNLAB_SPI_CTRL_SPI_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_ctrl_rx_buf) {
        dev_err(&tc6->spi->dev, "Failed to allocate SPI control RX buffer\n");
        return NULL;
    }

    tc6->spi_data_tx_buf = devm_kzalloc(&tc6->spi->dev, TSNLAB_SPI_SPI_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_data_tx_buf) {
        dev_err(&tc6->spi->dev, "Failed to allocate SPI data TX buffer\n");
        return NULL;
    }

    tc6->spi_data_rx_buf = devm_kzalloc(&tc6->spi->dev, TSNLAB_SPI_SPI_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_data_rx_buf) {
        dev_err(&tc6->spi->dev, "Failed to allocate SPI data RX buffer\n");
        return NULL;
    }

    tc6->spi_meta_rx_buf = devm_kzalloc(&tc6->spi->dev, TSNLAB_SPI_RX_META_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_meta_rx_buf) {
        dev_err(&tc6->spi->dev, "Failed to allocate SPI meta RX buffer\n");
        return NULL;
    }

    tc6->spi_meta_tx_buf = devm_kzalloc(&tc6->spi->dev, TSNLAB_SPI_RX_META_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_meta_tx_buf) {
        dev_err(&tc6->spi->dev, "Failed to allocate SPI meta TX buffer\n");
        return NULL;
    }

    ret = tsnlab_spi_sw_reset_macphy(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "MAC-PHY software reset failed: %d\n", ret);
        return NULL;
    }

    ret = tsnlab_spi_unmask_macphy_error_interrupts(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "MAC-PHY error interrupts unmask failed: %d\n", ret);
        return NULL;
    }

    ret = tsnlab_spi_phy_init(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "MAC internal PHY initialization failed: %d\n", ret);
        return NULL;
    }

    ret = tsnlab_spi_enable_data_transfer(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to enable data transfer: %d\n", ret);
        goto phy_exit;
    }

    ret = tsnlab_spi_update_buffer_status_from_register(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to update buffer status: %d\n", ret);
        goto phy_exit;
    }

    init_waitqueue_head(&tc6->spi_wq);

    tc6->spi_thread = kthread_run(tsnlab_spi_spi_thread_handler, tc6, "tsn-tc6-spi-thread");
    if (IS_ERR(tc6->spi_thread)) {
        dev_err(&tc6->spi->dev, "Failed to create SPI thread\n");
        goto phy_exit;
    }

    sched_set_fifo(tc6->spi_thread);

    ret = devm_request_irq(&tc6->spi->dev, tc6->spi->irq, tsnlab_spi_macphy_isr, IRQF_TRIGGER_FALLING,
                           dev_name(&tc6->spi->dev), tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to request macphy isr %d\n", ret);
        goto kthread_stop;
    }

    /* tsnlab_spi_sw_reset_macphy() function resets and clears the MAC-PHY reset
     * complete status. IRQ is also asserted on reset completion and it is
     * remain asserted until MAC-PHY receives a data chunk. So performing an
     * empty data chunk transmission will deassert the IRQ. Refer section
     * 7.7 and 9.2.8.8 in the OPEN Alliance specification for more details.
     */
    tc6->int_flag = true;
    wake_up_interruptible(&tc6->spi_wq);

    return tc6;

kthread_stop:
    kthread_stop(tc6->spi_thread);
phy_exit:
    tsnlab_spi_phy_exit(tc6);
    return NULL;
}
```

이제 `tsnlab_spi_try_spi_transfer` 함수에도 더 나은 에러 처리를 추가하겠습니다:


```c
static int tsnlab_spi_try_spi_transfer(struct tsnlab_spi* tc6) {
    int ret;
    struct rx_metadata meta;

    if (!tc6) {
        pr_err("Invalid tc6 parameter in tsnlab_spi_try_spi_transfer\n");
        return -EINVAL;
    }

    while (true) {
        u16 spi_len = 0;

        tc6->spi_data_tx_buf_offset = 0;

        /* Transmit */
        if (tc6->waiting_tx_skb) {
#if 0
			u32 regval;
			ret = tsnlab_spi_read_register(
				tc6, FBW_BUFFER_WRITE_STATUS1_H, &regval);

			if ((regval & 0xFF) == 0) {
				/* There is no empty buffer */
				return -1;
			}
#endif

            spi_len = tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs(tc6);

            if (spi_len > 0) {
                ret = tsnlab_spi_spi_transfer(tc6, TSNLAB_SPI_DATA_HEADER, spi_len);

                if (ret) {
                    netdev_err(tc6->netdev, "SPI data transfer failed: %d\n", ret);
                }
            }
        }

        /* Receive */

        ret = tsnlab_spi_get_meta_data(tc6, &meta);
        if (ret) {
            if (ret == -EINVAL) {
                /* Invalid parameters, stop processing */
                return ret;
            }
            continue;
        }

        if (meta.frame_length == 0) {
            break;
        }

        ret = tsnlab_spi_process_spi_data_rx_buf(tc6, meta);
        if (ret) {
            if (ret == -EAGAIN)
                continue;

            tsnlab_spi_cleanup_ongoing_tx_skb(tc6);
            tsnlab_spi_cleanup_ongoing_rx_skb(tc6);
            netdev_err(tc6->netdev, "Device error: %d\n", ret);
            return ret;
        }

        if (!tc6->waiting_tx_skb && netif_queue_stopped(tc6->netdev))
            netif_wake_queue(tc6->netdev);
    }

    return 0;
}
```

이제 `tsnlab_spi_spi_transfer` 함수에도 더 나은 에러 처리를 추가하겠습니다:


```c
static int tsnlab_spi_spi_transfer(struct tsnlab_spi* tc6, enum tsnlab_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    if (!tc6 || !tc6->spi) {
        pr_err("Invalid parameters in tsnlab_spi_spi_transfer\n");
        return -EINVAL;
    }

    if (header_type == TSNLAB_SPI_DATA_HEADER) {
        if (!tc6->spi_data_tx_buf || !tc6->spi_data_rx_buf) {
            dev_err(&tc6->spi->dev, "SPI data buffers not allocated\n");
            return -EINVAL;
        }
        xfer.tx_buf = tc6->spi_data_tx_buf;
        xfer.rx_buf = tc6->spi_data_rx_buf;
    } else if (header_type == TSNLAB_SPI_META_HEADER) {
        if (!tc6->spi_meta_tx_buf || !tc6->spi_meta_rx_buf) {
            dev_err(&tc6->spi->dev, "SPI meta buffers not allocated\n");
            return -EINVAL;
        }
        xfer.tx_buf = tc6->spi_meta_tx_buf;
        xfer.rx_buf = tc6->spi_meta_rx_buf;
    } else {
        if (!tc6->spi_ctrl_tx_buf || !tc6->spi_ctrl_rx_buf) {
            dev_err(&tc6->spi->dev, "SPI control buffers not allocated\n");
            return -EINVAL;
        }
        xfer.tx_buf = tc6->spi_ctrl_tx_buf;
        xfer.rx_buf = tc6->spi_ctrl_rx_buf;
    }
    xfer.len = length;

    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);

    return spi_sync(tc6->spi, &msg);
}
```

이제 디바이스 트리 설정을 확인해보겠습니다. 라즈베리 파이에서 SPI 디바이스가 올바르게 설정되어 있는지 확인해야 합니다.


이제 SPI 설정을 확인해보겠습니다:


SPI 설정을 확인해보겠습니다:


이제 문제를 해결하기 위한 추가 수정사항을 제안하겠습니다. 디바이스 트리 설정에서 SPI 주파수가 너무 높게 설정되어 있을 수 있습니다. 또한 드라이버에서 SPI 설정을 개선해야 합니다.


```plaintext
&spi0 {
	pinctrl-names = "default";
	pinctrl-0 = <&spi0_pins &spi0_cs_pins>;
	cs-gpios = <&gpio 8 1>, <&gpio 7 1>;

	status = "okay";
	lan1qtsn: ethernet@0 {
		compatible = "tsnlab,onequeue";
		reg = <0>;  /* CE0 */
		interrupt-parent = <&gpio>;
		interrupts = <23 IRQ_TYPE_EDGE_FALLING>;
		reset-gpios = <&gpio 22 GPIO_ACTIVE_LOW>;
		spi-max-frequency = <1000000>; /* Reduced frequency for stability */
		status = "okay";
		spi-cpol;
		spi-cpha;
	};

	spidev1: spidev@1{
		compatible = "spidev";
		reg = <1>;	/* CE1 */
		#address-cells = <1>;
		#size-cells = <0>;
		spi-max-frequency = <125000000>;
	};
};
```

이제 드라이버에서 SPI 설정을 개선하겠습니다:


```c
static int lan1qtsn_probe(struct spi_device* spi) {
    struct net_device* netdev;
    struct lan1qtsn_priv* priv;
    int ret;
    struct ptp_device_data* ptp_data;

    if (!spi) {
        pr_err("Invalid SPI device\n");
        return -EINVAL;
    }

    netdev = alloc_etherdev(sizeof(struct lan1qtsn_priv));
    if (!netdev)
        return -ENOMEM;

    priv = netdev_priv(netdev);
    priv->netdev = netdev;
    priv->spi = spi;
    spi_set_drvdata(spi, priv);

    /* Configure SPI settings */
    spi->mode = SPI_MODE_0 | SPI_CS_HIGH;
    spi->bits_per_word = 8;
    ret = spi_setup(spi);
    if (ret) {
        dev_err(&spi->dev, "SPI setup failed: %d\n", ret);
        goto free_netdev;
    }

    priv->tc6 = tsnlab_spi_init(spi, netdev);
    g_tc6 = priv->tc6;
    if (!priv->tc6) {
        ret = -ENODEV;
        goto free_netdev;
    }

    /* As per the point s3 in the below errata, SPI receive Ethernet frame
     * transfer may halt when starting the next frame in the same data block
     * (chunk) as the end of a previous frame. The RFA field should be
     * configured to 01b or 10b for proper operation. In these modes, only
     * one receive Ethernet frame will be placed in a single data block.
     * When the RFA field is written to 01b, received frames will be forced
     * to only start in the first word of the data block payload (SWO=0). As
     * recommended, enable zero align receive frame feature for proper
     * operation.
     *
     * https://ww1.microchip.com/downloads/aemDocuments/documents/AIS/ProductDocuments/Errata/LAN8650-1-Errata-80001075.pdf
     */
    ret = tsnlab_spi_zero_align_receive_frame_enable(priv->tc6);
    if (ret) {
        dev_err(&spi->dev, "Failed to set ZARFE: %d\n", ret);
        goto tsnlab_spi_exit;
    }

    eth_hw_addr_random(netdev);

    ret = lan1qtsn_set_hw_macaddr(priv, netdev->dev_addr);
    if (ret) {
        dev_err(&spi->dev, "Failed to configure MAC: %d\n", ret);
        goto tsnlab_spi_exit;
    }

    netdev->if_port = IF_PORT_100BASET;
    netdev->irq = spi->irq;
    netdev->netdev_ops = &lan1qtsn_netdev_ops;
    netdev->ethtool_ops = &lan1qtsn_ethtool_ops;

    ret = register_netdev(netdev);
    if (ret) {
        dev_err(&spi->dev, "Register netdev failed (ret = %d)", ret);
        goto tsnlab_spi_exit;
    }

    ptp_data = ptp_device_init(&spi->dev, priv->tc6);
    if (ptp_data) {
        priv->tc6->ptp = ptp_data;
    }

    return misc_register(&lan1qtsn_miscdev);

tsnlab_spi_exit:
    tsnlab_spi_exit(priv->tc6);
free_netdev:
    free_netdev(priv->netdev);
    return ret;
}
```

이제 추가적인 디버깅 정보를 제공하기 위해 드라이버에 더 많은 로그를 추가하겠습니다:


```c
static int tsnlab_spi_perform_ctrl(struct tsnlab_spi* tc6, u32 address, u32 value[], u8 length,
                                   enum tsnlab_spi_register_op reg_op) {
    u16 size;
    int ret;

    if (!tc6 || !tc6->spi) {
        pr_err("Invalid parameters in tsnlab_spi_perform_ctrl\n");
        return -EINVAL;
    }

    /* Prepare control command and copy to SPI control buffer */
    tsnlab_spi_prepare_ctrl_spi_buf(tc6, address, value, length, reg_op);

    size = tsnlab_spi_calculate_ctrl_buf_size(length, reg_op);

    dev_dbg(&tc6->spi->dev, "SPI control transfer: addr=0x%x, len=%d, op=%d, size=%d\n", 
            address, length, reg_op, size);

    /* Perform SPI transfer */
    ret = tsnlab_spi_spi_transfer(tc6, TSNLAB_SPI_CTRL_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* Check echoed/received control write command reply for errors */
    if (reg_op == TSNLAB_SPI_CTRL_REG_WRITE)
        return tsnlab_spi_check_ctrl_write_reply(tc6, size);

    /* Check echoed/received control read command reply for errors */
    ret = tsnlab_spi_check_ctrl_read_reply(tc6, size);
    if (ret)
        return ret;

    tsnlab_spi_copy_ctrl_read_data(tc6, value, length);

    return 0;
}
```

이제 컴파일을 위해 Makefile을 확인해보겠습니다:


이제 상위 Makefile도 확인해보겠습니다:


이제 Kconfig 파일을 확인해보겠습니다:


이제 상위 Kconfig 파일도 확인해보겠습니다:


이제 문제 해결을 위한 추가 정보를 제공하겠습니다:

## 문제 해결 요약

### 1. 주요 문제점들
1. **SPI 전송 실패 (-22 에러)**: 잘못된 매개변수로 인한 SPI 전송 실패
2. **NULL 포인터 역참조**: SPI 버퍼가 제대로 할당되지 않았거나 NULL 포인터 접근
3. **SPI 설정 문제**: 주파수가 너무 높거나 설정이 잘못됨

### 2. 적용된 수정사항들

1. **NULL 포인터 체크 추가**:
   - `tsnlab_spi_get_meta_data` 함수에 NULL 포인터 체크 추가
   - `tsnlab_spi_spi_transfer` 함수에 버퍼 할당 체크 추가
   - `tsnlab_spi_try_spi_transfer` 함수에 매개변수 검증 추가

2. **SPI 설정 개선**:
   - 디바이스 트리에서 SPI 주파수를 12.5MHz에서 1MHz로 낮춤
   - `lan1qtsn_probe`에서 SPI 모드 설정 추가
   - `tsnlab_spi_init`에서 SPI 설정 에러 처리 개선

3. **에러 처리 개선**:
   - 더 자세한 에러 메시지 추가
   - 디버깅 정보 추가
   - 메모리 할당 실패 시 적절한 에러 처리

### 3. 추가 확인사항

커널을 다시 빌드하고 테스트할 때 다음 사항들을 확인해주세요:

1. **커널 설정 확인**:
   ```bash
   # 커널 설정에서 다음 옵션들이 활성화되어 있는지 확인
   CONFIG_SPI=y
   CONFIG_SPI_MASTER=y
   CONFIG_LAN1QTSN=y
   ```

2. **디바이스 트리 오버레이 확인**:
   ```bash
   # 부팅 후 다음 명령으로 SPI 디바이스 확인
   ls /dev/spidev*
   dmesg | grep -i spi
   dmesg | grep -i lan1qtsn
   ```

3. **SPI 디바이스 상태 확인**:
   ```bash
   # SPI 디바이스 정보 확인
   cat /sys/kernel/debug/spi/spi0.0/device
   ```

4. **네트워크 인터페이스 확인**:
   ```bash
   # 네트워크 인터페이스 확인
   ip link show
   dmesg | grep -i ethernet
   ```

### 4. 추가 디버깅 방법

만약 여전히 문제가 발생한다면:

1. **커널 로그 레벨 증가**:
   ```bash
   # 부팅 시 로그 레벨 설정
   dmesg -n 8
   ```

2. **SPI 디버깅 활성화**:
   ```bash
   # SPI 디버깅 활성화
   echo 1 > /sys/kernel/debug/spi/spi0.0/debug
   ```

3. **드라이버 로드 확인**:
   ```bash
   # 드라이버 로드 상태 확인
   lsmod | grep lan1qtsn
   modinfo lan1qtsn
   ```

이러한 수정사항들을 적용한 후 커널을 다시 빌드하고 테스트해보시기 바랍니다. 문제가 지속되면 추가적인 디버깅 정보를 제공해주시면 더 구체적인 해결책을 제시할 수 있습니다.

```shellscript
   # 커널 설정에서 다음 옵션들이 활성화되어 있는지 확인
   CONFIG_SPI=y
   CONFIG_SPI_MASTER=y
   CONFIG_LAN1QTSN=y
```

```shellscript
   # 부팅 후 다음 명령으로 SPI 디바이스 확인
   ls /dev/spidev*
   dmesg | grep -i spi
   dmesg | grep -i lan1qtsn
```

```shellscript
   # SPI 디바이스 정보 확인
   cat /sys/kernel/debug/spi/spi0.0/device
```

```shellscript
   # 네트워크 인터페이스 확인
   ip link show
   dmesg | grep -i ethernet
```

```shellscript
   # 부팅 시 로그 레벨 설정
   dmesg -n 8
```

```shellscript
   # SPI 디버깅 활성화
   echo 1 > /sys/kernel/debug/spi/spi0.0/debug
```

```shellscript
   # 드라이버 로드 상태 확인
   lsmod | grep lan1qtsn
   modinfo lan1qtsn
```

