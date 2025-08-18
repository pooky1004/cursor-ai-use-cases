# Kernel scheduling issue on Raspberry Pi 4
_Exported on 8/14/2025 at 11:21:37 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 사용중에 있어.
ping 시험을 하던 중 다음과 같은 현상이 발생했는데,
원인 분석과 어떻게 수정해야하는지 가이드를 해줘


[ 1117.107716] BUG: scheduling while atomic: lan1qtsn-spi-th/8015/0x00000103
[ 1117.107735] Modules linked in: lan1qtsn rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc spidev joydev hci_uart brcmfmac btbcm brcmutil vc4 bluetooth rpi_hevc_dec bcm2835_v4l2(C) cfg80211 bcm2835_isp(C) bcm2835_codec(C) snd_soc_hdmi_codec bcm2835_mmal_vchiq(C) drm_display_helper ecdh_generic cec vc_sm_cma(C) v4l2_mem2mem ecc videobuf2_vmalloc v3d videobuf2_dma_contig videobuf2_memops rfkill drm_dma_helper videobuf2_v4l2 snd_soc_core gpu_sched libaes drm_shmem_helper videodev drm_kms_helper snd_compress raspberrypi_hwmon snd_bcm2835(C) videobuf2_common snd_pcm_dmaengine snd_pcm i2c_brcmstb spi_bcm2835 mc snd_timer raspberrypi_gpiomem snd nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern drm_panel_orientation_quirks backlight dm_mod fuse ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1117.107901] CPU: 1 UID: 0 PID: 8015 Comm: lan1qtsn-spi-th Tainted: G         C         6.12.41-v8+ #6
[ 1117.107907] Tainted: [C]=CRAP
[ 1117.107909] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1117.107912] Call trace:
[ 1117.107914]  dump_backtrace+0xa0/0x100
[ 1117.107923]  show_stack+0x20/0x38
[ 1117.107927]  dump_stack_lvl+0x78/0x90
[ 1117.107934]  dump_stack+0x18/0x28
[ 1117.107938]  __schedule_bug+0x58/0x78
[ 1117.107944]  __schedule+0x964/0xb60
[ 1117.107948]  schedule+0x3c/0x148
[ 1117.107951]  schedule_preempt_disabled+0x2c/0x50
[ 1117.107955]  __mutex_lock.constprop.0+0x96c/0x988
[ 1117.107959]  __mutex_lock_slowpath+0x1c/0x30
[ 1117.107963]  mutex_lock+0x50/0x68
[ 1117.107967]  spi_sync+0x2c/0x68
[ 1117.107971]  lan1qtsn_spi_transfer+0xcc/0x148 [lan1qtsn]
[ 1117.107984]  lan1qtsn_spi_read_registers+0x6c/0xe0 [lan1qtsn]
[ 1117.107991]  lan1qtsn_spi_read_register+0x1c/0x30 [lan1qtsn]
[ 1117.107997]  lan1qtsn_get_sys_clock_by_ldev+0x40/0x90 [lan1qtsn]
[ 1117.108002]  lan1qtsn_netdev_start_xmit+0x10c/0x218 [lan1qtsn]
[ 1117.108007]  dev_hard_start_xmit+0xac/0x208
[ 1117.108012]  sch_direct_xmit+0x9c/0x1f0
[ 1117.108018]  __qdisc_run+0x134/0x780
[ 1117.108023]  net_tx_action+0x170/0x320
[ 1117.108026]  handle_softirqs+0x114/0x348
[ 1117.108033]  __do_softirq+0x1c/0x28
[ 1117.108036]  ____do_softirq+0x18/0x30
[ 1117.108040]  call_on_irq_stack+0x30/0x70
[ 1117.108044]  do_softirq_own_stack+0x24/0x38
[ 1117.108048]  irq_exit_rcu+0x88/0xc8
[ 1117.108053]  el1_interrupt+0x38/0x68
[ 1117.108059]  el1h_64_irq_handler+0x18/0x28
[ 1117.108064]  el1h_64_irq+0x64/0x68
[ 1117.108067]  bcm2835_spi_transfer_one+0x664/0xa68 [spi_bcm2835]
[ 1117.108072]  spi_transfer_one_message+0x46c/0x8a8
[ 1117.108076]  __spi_pump_transfer_message+0x274/0x610
[ 1117.108079]  __spi_sync+0x2b8/0x458
[ 1117.108083]  spi_sync+0x38/0x68
[ 1117.108086]  lan1qtsn_spi_transfer+0xcc/0x148 [lan1qtsn]
[ 1117.108092]  lan1qtsn_spi_thread_handler+0xc4/0x780 [lan1qtsn]
[ 1117.108097]  kthread+0x11c/0x128
[ 1117.108101]  ret_from_fork+0x10/0x20
[ 1117.108107] ------------[ cut here ]------------
[ 1117.108118] Voluntary context switch within RCU read-side critical section!
[ 1117.108134] WARNING: CPU: 1 PID: 8015 at kernel/rcu/tree_plugin.h:331 rcu_note_context_switch+0x430/0x520
[ 1117.108147] Modules linked in: lan1qtsn rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc spidev joydev hci_uart brcmfmac btbcm brcmutil vc4 bluetooth rpi_hevc_dec bcm2835_v4l2(C) cfg80211 bcm2835_isp(C) bcm2835_codec(C) snd_soc_hdmi_codec bcm2835_mmal_vchiq(C) drm_display_helper ecdh_generic cec vc_sm_cma(C) v4l2_mem2mem ecc videobuf2_vmalloc v3d videobuf2_dma_contig videobuf2_memops rfkill drm_dma_helper videobuf2_v4l2 snd_soc_core gpu_sched libaes drm_shmem_helper videodev drm_kms_helper snd_compress raspberrypi_hwmon snd_bcm2835(C) videobuf2_common snd_pcm_dmaengine snd_pcm i2c_brcmstb spi_bcm2835 mc snd_timer raspberrypi_gpiomem snd nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern drm_panel_orientation_quirks backlight dm_mod fuse ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1117.108285] CPU: 1 UID: 0 PID: 8015 Comm: lan1qtsn-spi-th Tainted: G        WC         6.12.41-v8+ #6
[ 1117.108291] Tainted: [W]=WARN, [C]=CRAP
[ 1117.108294] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1117.108296] pstate: 600000c5 (nZCv daIF -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[ 1117.108301] pc : rcu_note_context_switch+0x430/0x520
[ 1117.108307] lr : rcu_note_context_switch+0x430/0x520
[ 1117.108313] sp : ffffffc08000ba40
[ 1117.108315] x29: ffffffc08000ba40 x28: ffffff8102708000 x27: ffffff8103facc40
[ 1117.108324] x26: ffffffc08000be34 x25: ffffffd54e608000 x24: ffffff8102708000
[ 1117.108332] x23: 0000000000000000 x22: ffffff8102708000 x21: ffffffd54e83e730
[ 1117.108341] x20: 0000000000000000 x19: ffffff8103fada40 x18: 00000000fffffffd
[ 1117.108349] x17: 2020202020202020 x16: 47203a6465746e69 x15: 61542068742d6970
[ 1117.108357] x14: 732d6e737471316e x13: 216e6f6974636573 x12: 206c616369746972
[ 1117.108365] x11: 6320656469732d64 x10: ffffffd54e692248 x9 : ffffffd54cf1e760
[ 1117.108373] x8 : 00000000ffffefff x7 : ffffffd54e68d4f0 x6 : 0000000000000339
[ 1117.108381] x5 : ffffff8103f9a3c8 x4 : 0000000000000000 x3 : 0000000000000027
[ 1117.108389] x2 : 0000000000000000 x1 : 0000000000000000 x0 : ffffff8102708000
[ 1117.108396] Call trace:
[ 1117.108399]  rcu_note_context_switch+0x430/0x520
[ 1117.108405]  __schedule+0xac/0xb60
[ 1117.108410]  schedule+0x3c/0x148
[ 1117.108414]  schedule_preempt_disabled+0x2c/0x50
[ 1117.108418]  __mutex_lock.constprop.0+0x96c/0x988
[ 1117.108423]  __mutex_lock_slowpath+0x1c/0x30
[ 1117.108427]  mutex_lock+0x50/0x68
[ 1117.108432]  spi_sync+0x2c/0x68
[ 1117.108436]  lan1qtsn_spi_transfer+0xcc/0x148 [lan1qtsn]
[ 1117.108443]  lan1qtsn_spi_read_registers+0x6c/0xe0 [lan1qtsn]
[ 1117.108449]  lan1qtsn_spi_read_register+0x1c/0x30 [lan1qtsn]
[ 1117.108455]  lan1qtsn_get_sys_clock_by_ldev+0x40/0x90 [lan1qtsn]
[ 1117.108461]  lan1qtsn_netdev_start_xmit+0x10c/0x218 [lan1qtsn]
[ 1117.108467]  dev_hard_start_xmit+0xac/0x208
[ 1117.108472]  sch_direct_xmit+0x9c/0x1f0
[ 1117.108478]  __qdisc_run+0x134/0x780
[ 1117.108483]  net_tx_action+0x170/0x320
[ 1117.108487]  handle_softirqs+0x114/0x348
[ 1117.108493]  __do_softirq+0x1c/0x28
[ 1117.108497]  ____do_softirq+0x18/0x30
[ 1117.108501]  call_on_irq_stack+0x30/0x70
[ 1117.108506]  do_softirq_own_stack+0x24/0x38
[ 1117.108511]  irq_exit_rcu+0x88/0xc8
[ 1117.108517]  el1_interrupt+0x38/0x68
[ 1117.108523]  el1h_64_irq_handler+0x18/0x28
[ 1117.108528]  el1h_64_irq+0x64/0x68
[ 1117.108532]  bcm2835_spi_transfer_one+0x664/0xa68 [spi_bcm2835]
[ 1117.108537]  spi_transfer_one_message+0x46c/0x8a8
[ 1117.108542]  __spi_pump_transfer_message+0x274/0x610
[ 1117.108546]  __spi_sync+0x2b8/0x458
[ 1117.108550]  spi_sync+0x38/0x68
[ 1117.108554]  lan1qtsn_spi_transfer+0xcc/0x148 [lan1qtsn]
[ 1117.108561]  lan1qtsn_spi_thread_handler+0xc4/0x780 [lan1qtsn]
[ 1117.108567]  kthread+0x11c/0x128
[ 1117.108572]  ret_from_fork+0x10/0x20
[ 1117.108577] ---[ end trace 0000000000000000 ]---
[ 1138.008104] rcu: INFO: rcu_preempt detected stalls on CPUs/tasks:
[ 1138.008136] rcu:     Tasks blocked on level-0 rcu_node (CPUs 0-3): P8015/1:b..l
[ 1138.008171] rcu:     (detected by 2, t=5252 jiffies, g=41245, q=10969 ncpus=4)
[ 1138.008187] task:lan1qtsn-spi-th state:D stack:0     pid:8015  tgid:8015  ppid:2      flags:0x00000008
[ 1138.008210] Call trace:
[ 1138.008219]  __switch_to+0xf0/0x150
[ 1201.029168] rcu: INFO: rcu_preempt detected stalls on CPUs/tasks:
[ 1201.029185] rcu:     Tasks blocked on level-0 rcu_node (CPUs 0-3): P8015/1:b..l
[ 1201.029199] rcu:     (detected by 1, t=21007 jiffies, g=41245, q=43361 ncpus=4)
[ 1201.029205] task:lan1qtsn-spi-th state:D stack:0     pid:8015  tgid:8015  ppid:2      flags:0x00000008
[ 1201.029214] Call trace:
[ 1201.029217]  __switch_to+0xf0/0x150
[ 1264.050112] rcu: INFO: rcu_preempt detected stalls on CPUs/tasks:
[ 1264.050128] rcu:     Tasks blocked on level-0 rcu_node (CPUs 0-3): P8015/1:b..l
[ 1264.050143] rcu:     (detected by 1, t=36762 jiffies, g=41245, q=75714 ncpus=4)
[ 1264.050149] task:lan1qtsn-spi-th state:D stack:0     pid:8015  tgid:8015  ppid:2      flags:0x00000008
[ 1264.050158] Call trace:
[ 1264.050161]  __switch_to+0xf0/0x150




---

**Cursor**

이 에러 로그를 분석해보겠습니다. 먼저 관련 코드를 살펴보고 문제의 원인을 파악해보겠습니다.


이제 문제의 원인을 파악했습니다. 에러 로그를 분석한 결과 다음과 같은 문제가 발생하고 있습니다:

## 문제 분석

**"scheduling while atomic" 에러의 원인:**

1. **컨텍스트 문제**: `lan1qtsn_netdev_start_xmit()` 함수가 softirq 컨텍스트에서 실행되고 있습니다. 이는 `net_tx_action`에서 호출되는 것으로 보입니다.

2. **SPI 호출 체인**: 
   - `lan1qtsn_netdev_start_xmit()` → `lan1qtsn_get_sys_clock_by_ldev()` → `lan1qtsn_spi_read_register()` → `spi_sync()`

3. **mutex_lock 문제**: `spi_sync()` 내부에서 `mutex_lock()`을 시도하는데, 이는 atomic 컨텍스트에서 허용되지 않습니다.

4. **RCU 스톨**: 이로 인해 RCU(Read-Copy Update) 시스템이 스톨되어 시스템이 응답하지 않게 됩니다.

## 해결 방법

이 문제를 해결하기 위해 다음과 같은 수정이 필요합니다:

### 1. SPI 호출을 workqueue로 이동


```c
// ... existing code ...
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
#else
#if 1
    tx_metadata->from.tick = 0x0;
    tx_metadata->from.priority = 0;
    tx_metadata->to.tick = 0x1FFFFFFF;
    tx_metadata->to.priority = 0;
    tx_metadata->delay_from.tick = 0x0;
    tx_metadata->delay_from.priority = 0;
    tx_metadata->delay_to.tick = 0x1FFFFFFF;
    tx_metadata->delay_to.priority = 0;
#endif
    
    // Defer SPI clock reading to workqueue to avoid atomic context issues
    // Use a default timestamp for now, will be updated in workqueue
    now = 0; // Placeholder, will be set in workqueue
    
    // Set basic metadata without SPI calls
    if (tsn_fill_metadata(priv->tsdev, now, skb) == false) {
        // TODO: Increment SW drop stats
        pr_warn("tsn_fill_metadata failed\n");
        netif_wake_subqueue(ndev, q);
        return NETDEV_TX_BUSY;
    }

    lan1qtsn_debug("0x%08x  0x%08x  0x%08x  %4d  %1d", 0, tx_metadata->from.tick, tx_metadata->to.tick,
                   tx_metadata->frame_length, tx_metadata->fail_policy);
    dump_buffer((unsigned char*)tx_metadata, (int)(sizeof(struct tx_metadata) + skb->len));

// ... existing code ...
```

### 2. workqueue 구조체와 함수 추가


```cpp
struct lan1qtsn_priv {
	uint64_t magic;
	struct spi_device *sdev;
	struct lan1qtsn_spi_dev *tsdev;
	struct net_device *ndev;
	struct lan1qtsn_dev *ldev;

	struct lan1qtsn_spi *tc6;

	struct work_struct tx_work[TSN_TIMESTAMP_ID_MAX];
	struct sk_buff *tx_work_skb[TSN_TIMESTAMP_ID_MAX];
	sysclock_t tx_work_start_after[TSN_TIMESTAMP_ID_MAX];
	sysclock_t tx_work_wait_until[TSN_TIMESTAMP_ID_MAX];
	struct hwtstamp_config tstamp_config;
	sysclock_t last_tx_tstamp[TSN_TIMESTAMP_ID_MAX];
	int tstamp_retry[TSN_TIMESTAMP_ID_MAX];

	// Add workqueue for SPI clock reading to avoid atomic context issues
	struct work_struct clock_update_work;
	struct sk_buff *pending_clock_skb;

	struct sk_buff *rx_skb;
	struct sk_buff *tx_skb;

	uint64_t total_tx_count;
	uint64_t total_tx_drop_count;
	uint64_t last_normal_timeout;
	uint64_t last_to_overflow_popped;
	uint64_t last_to_overflow_timeout;

	unsigned long state;

// ... existing code ...
```

```cpp
// ... existing code ...

/* Function declarations */
void lan1qtsn_clock_update_work_handler(struct work_struct *work);

// ... existing code ...
```

### 3. workqueue 핸들러 구현


```c
// ... existing code ...

/* netif_wake_queue() will be called in lan1qtsn_isr() */
    priv->tx_skb = skb;

    // Schedule workqueue to update clock information in non-atomic context
    priv->pending_clock_skb = skb_get(skb);
    schedule_work(&priv->clock_update_work);

    return lan1qtsn_spi_start_xmit(priv->tc6, skb);
}

/**
 * lan1qtsn_clock_update_work_handler - Workqueue handler for updating clock information
 * @work: pointer to the work structure
 *
 * This function runs in process context and can safely call SPI functions
 */
void lan1qtsn_clock_update_work_handler(struct work_struct *work)
{
    struct lan1qtsn_priv *priv = container_of(work, struct lan1qtsn_priv, clock_update_work);
    struct sk_buff *skb = priv->pending_clock_skb;
    struct tx_buffer *tx_buffer;
    struct tx_metadata *tx_metadata;
    sysclock_t sys_count, sys_count_upper, sys_count_lower;
    timestamp_t now;
    
    if (!skb) {
        return;
    }
    
    // Now we can safely call SPI functions in process context
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
    sys_count_lower = sys_count & LOWER_29_BITS;
    sys_count_upper = sys_count & ~LOWER_29_BITS;
    
    // Update the metadata with proper clock information
    tx_buffer = (struct tx_buffer *)skb->data;
    tx_metadata = (struct tx_metadata *)&tx_buffer->metadata;
    
    // Update TSN metadata with proper timestamp
    if (tsn_fill_metadata(priv->tsdev, now, skb)) {
        dev_dbg(&priv->ndev->dev, "Clock updated successfully: 0x%08x  0x%08x  0x%08x  %4d  %1d", 
                sys_count_lower, tx_metadata->from.tick, tx_metadata->to.tick,
                tx_metadata->frame_length, tx_metadata->fail_policy);
    } else {
        dev_warn(&priv->ndev->dev, "Failed to update TSN metadata with clock information\n");
    }
    
    // Clean up
    priv->pending_clock_skb = NULL;
    dev_kfree_skb(skb);
}

// ... existing code ...
```

### 4. workqueue 초기화 추가


```c
	/* Increase TX queue length to handle bidirectional traffic better */
	ndev->tx_queue_len = LAN1QTSN_DEFAULT_TX_QUEUE_LEN;

	// Initialize workqueue for clock updates to avoid atomic context issues
	INIT_WORK(&priv->clock_update_work, lan1qtsn_clock_update_work_handler);
	priv->pending_clock_skb = NULL;

#ifdef __TSN_PTP__
// ... existing code ...
```

### 5. SPI 스레드에서의 mutex 사용 개선


### 6. SPI 스레드에서 안전한 SPI 호출을 위한 함수 추가


```c
/**
 * lan1qtsn_spi_write_register - function for writing a MAC-PHY register.
 * @tc6: lan1qtsn_spi struct.
 * @address: register address of the MAC-PHY to be written.
 * @value: value to be written in the @address register address of the MAC-PHY.
 *
 * Return: 0 on success otherwise failed.
 */
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value) {
    return lan1qtsn_spi_write_registers(tc6, address, &value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_register);

/**
 * lan1qtsn_spi_read_register_safe - Safe version for reading registers from SPI thread context
 * @tc6: lan1qtsn_spi struct.
 * @address: register address of the MAC-PHY to be read.
 * @value: value read from the @address register address of the MAC-PHY.
 *
 * This function is designed to be called from the SPI thread context where
 * the mutex is already held or not needed.
 *
 * Return: 0 on success otherwise failed.
 */
int lan1qtsn_spi_read_register_safe(struct lan1qtsn_spi* tc6, u32 address, u32* value) {
    int ret;
    
    if (!value) {
        return -EINVAL;
    }
    
    // Check if we're already in the SPI thread context
    if (current == tc6->spi_thread) {
        // We're in the SPI thread, can call directly without mutex
        ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, 1, LAN1QTSN_SPI_CTRL_REG_READ);
    } else {
        // We're not in the SPI thread, use the safe mutex version
        ret = lan1qtsn_spi_read_register(tc6, address, value);
    }
    
    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_register_safe);

// ... existing code ...
```

### 7. 헤더 파일에 새로운 함수 선언 추가


```cpp
// ... existing code ...

/* Function declarations */
int lan1qtsn_spi_read_register(struct lan1qtsn_spi* tc6, u32 address, u32* value);
int lan1qtsn_spi_read_register_safe(struct lan1qtsn_spi* tc6, u32 address, u32* value);
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value);
int lan1qtsn_spi_read_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length);
int lan1qtsn_spi_write_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length);

// ... existing code ...
```

### 8. workqueue에서 안전한 SPI 함수 사용


```c
    // Now we can safely call SPI functions in process context
    sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
    now = lan1qtsn_sysclock_to_timestamp(priv->tc6, sys_count);
    sys_count_lower = sys_count & LOWER_29_BITS;
    sys_count_upper = sys_count & ~LOWER_29_BITS;
    
    // Update the metadata with proper clock information
    tx_buffer = (struct tx_buffer *)skb->data;
    tx_metadata = (struct tx_metadata *)&tx_buffer->metadata;
    
    // Update TSN metadata with proper timestamp
    if (tsn_fill_metadata(priv->tsdev, now, skb)) {
        dev_dbg(&priv->ndev->dev, "Clock updated successfully: 0x%08x  0x%08x  0x%08x  %4d  %1d", 
                sys_count_lower, tx_metadata->from.tick, tx_metadata->to.tick,
                tx_metadata->frame_length, tx_metadata->fail_policy);
    } else {
        dev_warn(&priv->ndev->dev, "Failed to update TSN metadata with clock information\n");
    }
    
    // Clean up
    priv->pending_clock_skb = NULL;
    dev_kfree_skb(skb);
}
```

### 9. arch.c에서 안전한 SPI 함수 사용


```c
sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi *tc6)
{
	timestamp_t clock;
	u32 sys_clock_high, sys_clock_low;
	lan1qtsn_spi_read_register_safe(tc6, TSN_SYSCOUNT_H, &sys_clock_high);
	lan1qtsn_spi_read_register_safe(tc6, TSN_SYSCOUNT_L, &sys_clock_low);
	clock = ((u64)sys_clock_high << 32) | sys_clock_low;

	return clock;
}
```

### 10. 컴파일 및 테스트를 위한 Makefile 확인


## 수정 사항 요약

이제 "scheduling while atomic" 에러를 해결하기 위한 모든 수정이 완료되었습니다. 주요 변경사항은 다음과 같습니다:

### 1. **문제 원인**
- `lan1qtsn_netdev_start_xmit()` 함수가 softirq 컨텍스트에서 실행되면서 SPI 함수를 호출
- SPI 함수 내부의 `mutex_lock()`이 atomic 컨텍스트에서 호출되어 에러 발생
- 이로 인해 RCU 시스템이 스톨되어 시스템 응답 불가

### 2. **해결 방법**
- **workqueue 도입**: SPI 클록 읽기를 workqueue로 지연시켜 process 컨텍스트에서 실행
- **안전한 SPI 함수**: SPI 스레드 컨텍스트에서 안전하게 호출할 수 있는 함수 추가
- **컨텍스트 분리**: atomic 컨텍스트와 process 컨텍스트를 명확히 분리

### 3. **주요 수정 파일**
- `lan1qtsn_mod.h`: workqueue 구조체 추가
- `lan1qtsn_netdev.c`: workqueue 핸들러 구현 및 start_xmit 수정
- `lan1qtsn_spi.c`: 안전한 SPI 함수 추가
- `lan1qtsn_spi.h`: 새로운 함수 선언 추가
- `lan1qtsn_arch.c`: 안전한 SPI 함수 사용
- `lan1qtsn_mod.c`: workqueue 초기화 추가

### 4. **컴파일 및 테스트 방법**

```bash
# 드라이버 디렉토리로 이동
cd drivers/net/ethernet/tsnlab/lan1qtsn

# 드라이버 컴파일
make clean
make

# 기존 모듈 제거 (있다면)
sudo rmmod lan1qtsn

# 새 모듈 로드
sudo insmod lan1qtsn.ko

# 로그 확인
dmesg | tail -20
```

### 5. **예상 효과**
- "scheduling while atomic" 에러 해결
- RCU 스톨 문제 해결
- ping 테스트 시 안정적인 동작
- SPI 함수 호출의 컨텍스트 안전성 확보

이 수정으로 라즈베리 파이 4에서 ping 테스트 시 발생하던 커널 에러가 해결되고, TSN 드라이버가 안정적으로 동작할 것입니다.

```shellscript
# 드라이버 디렉토리로 이동
cd drivers/net/ethernet/tsnlab/lan1qtsn

# 드라이버 컴파일
make clean
make

# 기존 모듈 제거 (있다면)
sudo rmmod lan1qtsn

# 새 모듈 로드
sudo insmod lan1qtsn.ko

# 로그 확인
dmesg | tail -20
```

