# Raspberry Pi kernel scheduling issue
_Exported on 8/25/2025 at 08:10:54 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있다.

대국에서 1초 주기로 arping 시험을 하던 중 timeout 이 발생하여 확인해 보니,
본 host kernel 메시지로 아래와 같은 내용이 발생했다.

무엇이 문제인지 확인해 주고 햐결 방안을 알려 주세요.

[  598.446503] BUG: scheduling while atomic: kworker/u16:1/4569/0x00000304
[  598.446541] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc binfmt_misc brcmfmac brcmutil spidev cfg80211 joydev vc4 hci_uart btbcm bluetooth snd_soc_hdmi_codec drm_display_helper ecdh_generic ecc rfkill cec drm_dma_helper libaes bcm2835_codec(C) bcm2835_v4l2(C) bcm2835_isp(C) raspberrypi_hwmon snd_soc_core rpi_hevc_dec bcm2835_mmal_vchiq(C) snd_compress vc_sm_cma(C) v4l2_mem2mem snd_bcm2835(C) snd_pcm_dmaengine videobuf2_vmalloc v3d spi_bcm2835 videobuf2_dma_contig videobuf2_memops videobuf2_v4l2 gpu_sched videodev snd_pcm i2c_brcmstb drm_shmem_helper videobuf2_common mc drm_kms_helper raspberrypi_gpiomem snd_timer snd nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern fuse dm_mod drm_panel_orientation_quirks backlight ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[  598.447009] CPU: 0 UID: 0 PID: 4569 Comm: kworker/u16:1 Tainted: G         C O       6.12.41-v8+ #5
[  598.447029] Tainted: [C]=CRAP, [O]=OOT_MODULE
[  598.447033] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[  598.447042] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[  598.447083] Call trace:
[  598.447088]  dump_backtrace+0xa0/0x100
[  598.447106]  show_stack+0x20/0x38
[  598.447115]  dump_stack_lvl+0x78/0x90
[  598.447133]  dump_stack+0x18/0x28
[  598.447146]  __schedule_bug+0x58/0x78
[  598.447163]  __schedule+0x954/0xb50
[  598.447173]  schedule+0x3c/0x148
[  598.447182]  schedule_preempt_disabled+0x2c/0x50
[  598.447193]  __mutex_lock.constprop.0+0x2a4/0x980
[  598.447204]  __mutex_lock_slowpath+0x1c/0x30
[  598.447215]  mutex_lock+0x50/0x68
[  598.447225]  spi_sync+0x2c/0x68
[  598.447237]  lan1qtsn_spi_transfer+0xcc/0x148 [lan1qtsn]
[  598.447257]  lan1qtsn_spi_perform_ctrl+0x64/0x148 [lan1qtsn]
[  598.447275]  lan1qtsn_get_sys_clock_by_ldev+0x58/0xe8 [lan1qtsn]
[  598.447292]  lan1qtsn_netdev_start_xmit+0xd0/0x528 [lan1qtsn]
[  598.447308]  dev_hard_start_xmit+0xac/0x208
[  598.447321]  sch_direct_xmit+0x98/0x1e8
[  598.447337]  __dev_queue_xmit+0x544/0xed0
[  598.447348]  arp_xmit+0x4c/0xe0
[  598.447358]  arp_send_dst+0x88/0xd0
[  598.447365]  arp_process+0x6c0/0x7a8
[  598.447373]  arp_rcv+0x144/0x1e0
[  598.447380]  __netif_receive_skb_one_core+0x60/0x98
[  598.447392]  __netif_receive_skb+0x20/0x70
[  598.447404]  process_backlog+0x8c/0x150
[  598.447414]  __napi_poll+0x40/0x218
[  598.447425]  net_rx_action+0x314/0x370
[  598.447436]  handle_softirqs+0x114/0x348
[  598.447451]  __do_softirq+0x1c/0x28
[  598.447459]  ____do_softirq+0x18/0x30
[  598.447471]  call_on_irq_stack+0x30/0x70
[  598.447482]  do_softirq_own_stack+0x24/0x38
[  598.447494]  do_softirq+0x5c/0x80
[  598.447507]  __local_bh_enable_ip+0x94/0xa8
[  598.447521]  netif_rx+0xac/0x178
[  598.447530]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[  598.447546]  process_one_work+0x158/0x3d0
[  598.447560]  worker_thread+0x2d8/0x3e8
[  598.447573]  kthread+0x11c/0x128
[  598.447583]  ret_from_fork+0x10/0x20
[  598.447596] ------------[ cut here ]------------
[  598.447633] Voluntary context switch within RCU read-side critical section!
[  598.447662] WARNING: CPU: 0 PID: 4569 at kernel/rcu/tree_plugin.h:331 rcu_note_context_switch+0x430/0x520
[  598.447693] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc binfmt_misc brcmfmac brcmutil spidev cfg80211 joydev vc4 hci_uart btbcm bluetooth snd_soc_hdmi_codec drm_display_helper ecdh_generic ecc rfkill cec drm_dma_helper libaes bcm2835_codec(C) bcm2835_v4l2(C) bcm2835_isp(C) raspberrypi_hwmon snd_soc_core rpi_hevc_dec bcm2835_mmal_vchiq(C) snd_compress vc_sm_cma(C) v4l2_mem2mem snd_bcm2835(C) snd_pcm_dmaengine videobuf2_vmalloc v3d spi_bcm2835 videobuf2_dma_contig videobuf2_memops videobuf2_v4l2 gpu_sched videodev snd_pcm i2c_brcmstb drm_shmem_helper videobuf2_common mc drm_kms_helper raspberrypi_gpiomem snd_timer snd nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern fuse dm_mod drm_panel_orientation_quirks backlight ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[  598.448117] CPU: 0 UID: 0 PID: 4569 Comm: kworker/u16:1 Tainted: G        WC O       6.12.41-v8+ #5
[  598.448136] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[  598.448143] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[  598.448151] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[  598.448176] pstate: 600000c5 (nZCv daIF -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  598.448190] pc : rcu_note_context_switch+0x430/0x520
[  598.448209] lr : rcu_note_context_switch+0x430/0x520
[  598.448227] sp : ffffffc0800035f0
[  598.448235] x29: ffffffc0800035f0 x28: ffffff8100c98000 x27: ffffff8103f8bc40
[  598.448261] x26: ffffff8117e09402 x25: ffffffea523f8000 x24: ffffff8100c98000
[  598.448286] x23: 0000000000000000 x22: ffffff8100c98000 x21: ffffffea5262e730
[  598.448310] x20: 0000000000000000 x19: ffffff8103f8ca40 x18: 00000000fffffffd
[  598.448334] x17: 4320202020202020 x16: 202047203a646574 x15: 6e69615420313a36
[  598.448358] x14: 31752f72656b726f x13: 216e6f6974636573 x12: 206c616369746972
[  598.448382] x11: 6320656469732d64 x10: ffffffea52480a30 x9 : ffffffea50d1d1f0
[  598.448405] x8 : 00000000ffffefff x7 : ffffffea5247d4f0 x6 : 0000000000000238
[  598.448429] x5 : ffffff8103f793c8 x4 : 0000000000000000 x3 : 0000000000000027
[  598.448453] x2 : 0000000000000000 x1 : 0000000000000000 x0 : ffffff8100c98000
[  598.448477] Call trace:
[  598.448484]  rcu_note_context_switch+0x430/0x520
[  598.448504]  __schedule+0xa8/0xb50
[  598.448517]  schedule+0x3c/0x148
[  598.448530]  schedule_preempt_disabled+0x2c/0x50
[  598.448543]  __mutex_lock.constprop.0+0x2a4/0x980
[  598.448557]  __mutex_lock_slowpath+0x1c/0x30
[  598.448570]  mutex_lock+0x50/0x68
[  598.448584]  spi_sync+0x2c/0x68
[  598.448597]  lan1qtsn_spi_transfer+0xcc/0x148 [lan1qtsn]
[  598.448617]  lan1qtsn_spi_perform_ctrl+0x64/0x148 [lan1qtsn]
[  598.448636]  lan1qtsn_get_sys_clock_by_ldev+0x58/0xe8 [lan1qtsn]
[  598.448654]  lan1qtsn_netdev_start_xmit+0xd0/0x528 [lan1qtsn]
[  598.448672]  dev_hard_start_xmit+0xac/0x208
[  598.448687]  sch_direct_xmit+0x98/0x1e8
[  598.448704]  __dev_queue_xmit+0x544/0xed0
[  598.448718]  arp_xmit+0x4c/0xe0
[  598.448729]  arp_send_dst+0x88/0xd0
[  598.448740]  arp_process+0x6c0/0x7a8
[  598.448750]  arp_rcv+0x144/0x1e0
[  598.448761]  __netif_receive_skb_one_core+0x60/0x98
[  598.448775]  __netif_receive_skb+0x20/0x70
[  598.448789]  process_backlog+0x8c/0x150
[  598.448803]  __napi_poll+0x40/0x218
[  598.448817]  net_rx_action+0x314/0x370
[  598.448831]  handle_softirqs+0x114/0x348
[  598.448849]  __do_softirq+0x1c/0x28
[  598.448860]  ____do_softirq+0x18/0x30
[  598.448874]  call_on_irq_stack+0x30/0x70
[  598.448888]  do_softirq_own_stack+0x24/0x38
[  598.448902]  do_softirq+0x5c/0x80
[  598.448919]  __local_bh_enable_ip+0x94/0xa8
[  598.448936]  netif_rx+0xac/0x178
[  598.448948]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[  598.448967]  process_one_work+0x158/0x3d0
[  598.448983]  worker_thread+0x2d8/0x3e8
[  598.448999]  kthread+0x11c/0x128
[  598.449012]  ret_from_fork+0x10/0x20
[  598.449026] ---[ end trace 0000000000000000 ]---
[  598.449205] ------------[ cut here ]------------
[  598.449215] WARNING: CPU: 0 PID: 4569 at kernel/softirq.c:386 __local_bh_enable_ip+0x98/0xa8
[  598.449242] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc binfmt_misc brcmfmac brcmutil spidev cfg80211 joydev vc4 hci_uart btbcm bluetooth snd_soc_hdmi_codec drm_display_helper ecdh_generic ecc rfkill cec drm_dma_helper libaes bcm2835_codec(C) bcm2835_v4l2(C) bcm2835_isp(C) raspberrypi_hwmon snd_soc_core rpi_hevc_dec bcm2835_mmal_vchiq(C) snd_compress vc_sm_cma(C) v4l2_mem2mem snd_bcm2835(C) snd_pcm_dmaengine videobuf2_vmalloc v3d spi_bcm2835 videobuf2_dma_contig videobuf2_memops videobuf2_v4l2 gpu_sched videodev snd_pcm i2c_brcmstb drm_shmem_helper videobuf2_common mc drm_kms_helper raspberrypi_gpiomem snd_timer snd nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern fuse dm_mod drm_panel_orientation_quirks backlight ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[  598.449670] CPU: 0 UID: 0 PID: 4569 Comm: kworker/u16:1 Tainted: G        WC O       6.12.41-v8+ #5
[  598.449688] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[  598.449695] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[  598.449703] Workqueue: lan1qtsn-rx-wq lan1qtsn_rx_work_handler [lan1qtsn]
[  598.449728] pstate: 00000005 (nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  598.449741] pc : __local_bh_enable_ip+0x98/0xa8
[  598.449759] lr : __dev_queue_xmit+0x260/0xed0
[  598.449773] sp : ffffffc080003a30
[  598.449781] x29: ffffffc080003a30 x28: ffffffc080003eb8 x27: 0000000000000000
[  598.449806] x26: ffffff804a9969e0 x25: ffffffea51ffe000 x24: ffffff804a9969d6
[  598.449830] x23: ffffff81069bcd44 x22: ffffff8040337000 x21: 0000000000000000
[  598.449854] x20: ffffff81069bcc00 x19: ffffff8103539900 x18: 00000000000021a8
[  598.449878] x17: c03c48eb0f2dd802 x16: 0004060008010006 x15: 083c48eb0f2dd84c
[  598.449902] x14: 48eb0f2dd8000000 x13: 3841f6cf00000000 x12: 0000000000000000
[  598.449925] x11: 00000000000000c0 x10: 0000000000001a40 x9 : ffffffea51769718
[  598.449949] x8 : ffffff8100c99aa0 x7 : 000000000000000f x6 : 0000000000000000
[  598.449974] x5 : 0000000000000001 x4 : 0000000000001ab0 x3 : 0000000000000000
[  598.449997] x2 : 0000000000000000 x1 : 0000000000000200 x0 : 00000000fffffffe
[  598.450021] Call trace:
[  598.450028]  __local_bh_enable_ip+0x98/0xa8
[  598.450046]  __dev_queue_xmit+0x260/0xed0
[  598.450060]  arp_xmit+0x4c/0xe0
[  598.450072]  arp_send_dst+0x88/0xd0
[  598.450083]  arp_process+0x6c0/0x7a8
[  598.450094]  arp_rcv+0x144/0x1e0
[  598.450105]  __netif_receive_skb_one_core+0x60/0x98
[  598.450120]  __netif_receive_skb+0x20/0x70
[  598.450133]  process_backlog+0x8c/0x150
[  598.450147]  __napi_poll+0x40/0x218
[  598.450160]  net_rx_action+0x314/0x370
[  598.450173]  handle_softirqs+0x114/0x348
[  598.450190]  __do_softirq+0x1c/0x28
[  598.450201]  ____do_softirq+0x18/0x30
[  598.450215]  call_on_irq_stack+0x30/0x70
[  598.450229]  do_softirq_own_stack+0x24/0x38
[  598.450244]  do_softirq+0x5c/0x80
[  598.450260]  __local_bh_enable_ip+0x94/0xa8
[  598.450278]  netif_rx+0xac/0x178
[  598.450289]  lan1qtsn_rx_work_handler+0x48/0x68 [lan1qtsn]
[  598.450310]  process_one_work+0x158/0x3d0
[  598.450326]  worker_thread+0x2d8/0x3e8
[  598.450342]  kthread+0x11c/0x128
[  598.450354]  ret_from_fork+0x10/0x20
[  598.450368] ---[ end trace 0000000000000000 ]---
[  598.450387] ------------[ cut here ]------------


---

**Cursor**

커널 메시지를 분석해보니 "scheduling while atomic" 버그가 발생하고 있습니다. 이는 atomic context에서 스케줄링을 시도할 때 발생하는 심각한 문제입니다. 코드를 확인해보겠습니다.


이제 `netif_rx` 함수가 어떻게 호출되는지 확인해보겠습니다.


이제 `lan1qtsn_netdev_start_xmit` 함수를 확인해보겠습니다.


이제 `lan1qtsn_get_sys_clock_by_ldev` 함수를 확인해보겠습니다.


이제 `lan1qtsn_spi_perform_ctrl` 함수를 확인해보겠습니다.


이제 `lan1qtsn_spi_transfer` 함수를 확인해보겠습니다.


이제 문제를 파악했습니다. 커널 메시지를 분석한 결과 다음과 같은 문제가 발생하고 있습니다:

## 문제 분석

**"scheduling while atomic" 버그**가 발생하고 있으며, 이는 atomic context에서 스케줄링을 시도할 때 발생하는 심각한 문제입니다.

### 문제의 원인

1. **ARP 처리 중 SPI 호출**: `arp_process` → `arp_xmit` → `__dev_queue_xmit` → `lan1qtsn_netdev_start_xmit` → `lan1qtsn_get_sys_clock_by_ldev` → `lan1qtsn_spi_perform_ctrl` → `spi_sync`

2. **Softirq context에서 mutex 사용**: ARP 처리가 softirq context에서 실행되는데, 이 context에서 `mutex_lock`을 호출하여 스케줄링이 발생

3. **Workqueue에서 netif_rx 호출**: `lan1qtsn_rx_work_handler`에서 `netif_rx`를 호출하여 네트워크 스택에 패킷을 전달

### 구체적인 문제점

- **Line 47**: `lan1qtsn_arch.c`에서 `mutex_trylock`을 사용하지만, 실패 시 0을 반환
- **Line 133**: `lan1qtsn_netdev.c`에서 `lan1qtsn_get_sys_clock_by_ldev` 호출 시 SPI 동기화가 필요
- **Line 166**: `lan1qtsn_spi.c`에서 `spi_sync` 호출이 atomic context에서 실행

## 해결 방안

### 1. 즉시 적용 가능한 해결책

`lan1qtsn_get_sys_clock_by_ldev` 함수를 수정하여 atomic context에서 안전하게 동작하도록 합니다:

```c:drivers/net/ethernet/tsnlab/lan1qtsn/lan1qtsn_arch.c
sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi* tc6) {
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    int ret;

    /* Check if we're in atomic context */
    if (in_atomic() || in_softirq()) {
        /* In atomic context, return cached value or 0 */
        dev_dbg(&tc6->spi->dev, "Atomic context detected, returning cached clock\n");
        return tc6->cached_sys_clock; /* Add this field to struct */
    }

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
            tc6->cached_sys_clock = clock; /* Cache the value */
            return clock;
        }
    }

    /* If we couldn't acquire the lock or read failed, return cached value */
    dev_info(&tc6->spi->dev, "Could not read system clock safely, using cached value\n");
    return tc6->cached_sys_clock;
}
```

### 2. 근본적인 해결책

#### A. Workqueue 사용
`lan1qtsn_netdev_start_xmit`에서 시스템 클록 읽기를 workqueue로 지연:

```c:drivers/net/ethernet/tsnlab/lan1qtsn/lan1qtsn_netdev.c
// ... existing code ...
/* Defer system clock reading to workqueue if in atomic context */
if (in_atomic() || in_softirq()) {
    struct lan1qtsn_clock_work *clock_work;
    clock_work = kmalloc(sizeof(*clock_work), GFP_ATOMIC);
    if (clock_work) {
        clock_work->tc6 = priv->tc6;
        clock_work->skb = skb;
        INIT_WORK(&clock_work->work, lan1qtsn_clock_work_handler);
        queue_work(priv->clock_workqueue, &clock_work->work);
        return NETDEV_TX_OK;
    }
}
// ... existing code ...
```

#### B. 비동기 SPI 사용
`spi_async`를 사용하여 non-blocking 방식으로 변경:

```c:drivers/net/ethernet/tsnlab/lan1qtsn/lan1qtsn_spi.c
static int lan1qtsn_spi_transfer_async(struct lan1qtsn_spi* tc6, 
                                       enum lan1qtsn_spi_header_type header_type, 
                                       u16 length,
                                       void (*complete)(void *context)) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;
    
    // ... setup xfer and msg ...
    
    return spi_async(tc6->spi, &msg);
}
```

### 3. 구조체 수정

`lan1qtsn_spi` 구조체에 캐시 필드 추가:

```c:drivers/net/ethernet/tsnlab/lan1qtsn/lan1qtsn_spi.h
struct lan1qtsn_spi {
    // ... existing fields ...
    sysclock_t cached_sys_clock;
    spinlock_t clock_cache_lock;
    // ... existing fields ...
};
```

### 4. 권장 적용 순서

1. **즉시**: `lan1qtsn_get_sys_clock_by_ldev` 함수에 atomic context 체크 추가
2. **단기**: Workqueue를 사용한 시스템 클록 읽기 지연
3. **장기**: 비동기 SPI 전송으로 전체적인 구조 개선

이러한 수정을 통해 "scheduling while atomic" 버그를 해결하고, 시스템의 안정성을 크게 향상시킬 수 있습니다.

```c
sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi* tc6) {
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    int ret;

    /* Check if we're in atomic context */
    if (in_atomic() || in_softirq()) {
        /* In atomic context, return cached value or 0 */
        dev_dbg(&tc6->spi->dev, "Atomic context detected, returning cached clock\n");
        return tc6->cached_sys_clock; /* Add this field to struct */
    }

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
            tc6->cached_sys_clock = clock; /* Cache the value */
            return clock;
        }
    }

    /* If we couldn't acquire the lock or read failed, return cached value */
    dev_info(&tc6->spi->dev, "Could not read system clock safely, using cached value\n");
    return tc6->cached_sys_clock;
}
```

```c
// ... existing code ...
/* Defer system clock reading to workqueue if in atomic context */
if (in_atomic() || in_softirq()) {
    struct lan1qtsn_clock_work *clock_work;
    clock_work = kmalloc(sizeof(*clock_work), GFP_ATOMIC);
    if (clock_work) {
        clock_work->tc6 = priv->tc6;
        clock_work->skb = skb;
        INIT_WORK(&clock_work->work, lan1qtsn_clock_work_handler);
        queue_work(priv->clock_workqueue, &clock_work->work);
        return NETDEV_TX_OK;
    }
}
// ... existing code ...
```

```c
static int lan1qtsn_spi_transfer_async(struct lan1qtsn_spi* tc6, 
                                       enum lan1qtsn_spi_header_type header_type, 
                                       u16 length,
                                       void (*complete)(void *context)) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;
    
    // ... setup xfer and msg ...
    
    return spi_async(tc6->spi, &msg);
}
```

```c
struct lan1qtsn_spi {
    // ... existing fields ...
    sysclock_t cached_sys_clock;
    spinlock_t clock_cache_lock;
    // ... existing fields ...
};
```

