# Kernel message analysis for Raspberry Pi 4
_Exported on 8/20/2025 at 11:27:23 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있어.

SPI 인터페이스를 통해 구성된 네트워 포트를 통해 ping 시험(상대편에서도 ping을 하고, 라즈베리 파이 4에서도 ping을 수행)을 하던 중에
다음과 같은 커널 메시지가 발생했어.
확인 해 줘

[ 1948.319733] BUG: scheduling while atomic: ping/22628/0x00000203
[ 1948.319760] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc binfmt_misc hci_uart spidev btbcm joydev bluetooth vc4 brcmfmac brcmutil snd_soc_hdmi_codec ecdh_generic cfg80211 drm_display_helper cec ecc rfkill drm_dma_helper bcm2835_v4l2(C) libaes bcm2835_codec(C) rpi_hevc_dec bcm2835_isp(C) snd_soc_core raspberrypi_hwmon bcm2835_mmal_vchiq(C) v4l2_mem2mem snd_compress vc_sm_cma(C) snd_bcm2835(C) snd_pcm_dmaengine i2c_brcmstb videobuf2_vmalloc videobuf2_dma_contig snd_pcm v3d raspberrypi_gpiomem videobuf2_memops videobuf2_v4l2 spi_bcm2835 videodev snd_timer gpu_sched videobuf2_common snd drm_shmem_helper mc drm_kms_helper nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern fuse drm_panel_orientation_quirks backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1948.320229] CPU: 2 UID: 1000 PID: 22628 Comm: ping Tainted: G         C O       6.12.41-v8+ #3
[ 1948.320249] Tainted: [C]=CRAP, [O]=OOT_MODULE
[ 1948.320254] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1948.320261] Call trace:
[ 1948.320266]  dump_backtrace+0xa0/0x100
[ 1948.320287]  show_stack+0x20/0x38
[ 1948.320297]  dump_stack_lvl+0x78/0x90
[ 1948.320314]  dump_stack+0x18/0x28
[ 1948.320327]  __schedule_bug+0x58/0x78
[ 1948.320344]  __schedule+0x954/0xb50
[ 1948.320354]  schedule+0x3c/0x148
[ 1948.320363]  schedule_preempt_disabled+0x2c/0x50
[ 1948.320374]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1948.320386]  __mutex_lock_slowpath+0x1c/0x30
[ 1948.320397]  mutex_lock+0x50/0x68
[ 1948.320407]  lan1qtsn_spi_transfer+0x88/0x180 [lan1qtsn]
[ 1948.320442]  lan1qtsn_spi_read_registers+0x6c/0xe0 [lan1qtsn]
[ 1948.320461]  lan1qtsn_spi_read_register+0x1c/0x30 [lan1qtsn]
[ 1948.320478]  lan1qtsn_get_sys_clock_by_ldev+0x3c/0x88 [lan1qtsn]
[ 1948.320494]  lan1qtsn_netdev_start_xmit+0xc8/0x4d8 [lan1qtsn]
[ 1948.320510]  dev_hard_start_xmit+0xac/0x208
[ 1948.320523]  sch_direct_xmit+0x98/0x1e8
[ 1948.320540]  __dev_queue_xmit+0x544/0xed0
[ 1948.320551]  ip_finish_output2+0x3c4/0x660
[ 1948.320565]  __ip_finish_output+0xac/0x1b0
[ 1948.320576]  ip_finish_output+0x3c/0x120
[ 1948.320588]  ip_output+0x70/0x110
[ 1948.320599]  ip_send_skb+0x58/0x100
[ 1948.320611]  ip_push_pending_frames+0x38/0x68
[ 1948.320623]  ping_v4_sendmsg+0x338/0x630
[ 1948.320636]  inet_sendmsg+0x4c/0x80
[ 1948.320648]  __sock_sendmsg+0x64/0xc0
[ 1948.320662]  __sys_sendto+0x10c/0x170
[ 1948.320675]  __arm64_sys_sendto+0x30/0x48
[ 1948.320688]  invoke_syscall+0x50/0x120
[ 1948.320704]  el0_svc_common.constprop.0+0x48/0xf0
[ 1948.320719]  do_el0_svc+0x24/0x38
[ 1948.320733]  el0_svc+0x30/0x100
[ 1948.320749]  el0t_64_sync_handler+0x100/0x130
[ 1948.320764]  el0t_64_sync+0x190/0x198
[ 1948.320776] ------------[ cut here ]------------
[ 1948.320810] Voluntary context switch within RCU read-side critical section!
[ 1948.320838] WARNING: CPU: 2 PID: 22628 at kernel/rcu/tree_plugin.h:331 rcu_note_context_switch+0x430/0x520
[ 1948.320869] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc binfmt_misc hci_uart spidev btbcm joydev bluetooth vc4 brcmfmac brcmutil snd_soc_hdmi_codec ecdh_generic cfg80211 drm_display_helper cec ecc rfkill drm_dma_helper bcm2835_v4l2(C) libaes bcm2835_codec(C) rpi_hevc_dec bcm2835_isp(C) snd_soc_core raspberrypi_hwmon bcm2835_mmal_vchiq(C) v4l2_mem2mem snd_compress vc_sm_cma(C) snd_bcm2835(C) snd_pcm_dmaengine i2c_brcmstb videobuf2_vmalloc videobuf2_dma_contig snd_pcm v3d raspberrypi_gpiomem videobuf2_memops videobuf2_v4l2 spi_bcm2835 videodev snd_timer gpu_sched videobuf2_common snd drm_shmem_helper mc drm_kms_helper nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern fuse drm_panel_orientation_quirks backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[ 1948.321295] CPU: 2 UID: 1000 PID: 22628 Comm: ping Tainted: G        WC O       6.12.41-v8+ #3
[ 1948.321314] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[ 1948.321321] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[ 1948.321330] pstate: 600000c5 (nZCv daIF -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[ 1948.321344] pc : rcu_note_context_switch+0x430/0x520
[ 1948.321363] lr : rcu_note_context_switch+0x430/0x520
[ 1948.321381] sp : ffffffc0853633d0
[ 1948.321389] x29: ffffffc0853633d0 x28: ffffff8050662100 x27: ffffff8103fcdc40
[ 1948.321417] x26: ffffffc085363814 x25: ffffffdbdfbf8000 x24: ffffff8050662100
[ 1948.321443] x23: 0000000000000000 x22: ffffff8050662100 x21: ffffffdbdfe2e730
[ 1948.321467] x20: 0000000000000000 x19: ffffff8103fcea40 x18: 00000000fffffffd
[ 1948.321493] x17: 2020204f20432020 x16: 2020202020202047 x15: 203a6465746e6961
[ 1948.321518] x14: 5420676e6970203a x13: 216e6f6974636573 x12: 206c616369746972
[ 1948.321543] x11: 6320656469732d64 x10: ffffffdbdfc86868 x9 : ffffffdbde51d1f0
[ 1948.321567] x8 : 00000000fffff626 x7 : ffffffdbdfc7d4f0 x6 : 0000000000001625
[ 1948.321592] x5 : ffffff8103fbb3c8 x4 : 0000000000000000 x3 : 0000000000000027
[ 1948.321616] x2 : 0000000000000000 x1 : 0000000000000000 x0 : ffffff8050662100
[ 1948.321640] Call trace:
[ 1948.321647]  rcu_note_context_switch+0x430/0x520
[ 1948.321668]  __schedule+0xa8/0xb50
[ 1948.321682]  schedule+0x3c/0x148
[ 1948.321694]  schedule_preempt_disabled+0x2c/0x50
[ 1948.321708]  __mutex_lock.constprop.0+0x2a4/0x980
[ 1948.321722]  __mutex_lock_slowpath+0x1c/0x30
[ 1948.321736]  mutex_lock+0x50/0x68
[ 1948.321750]  lan1qtsn_spi_transfer+0x88/0x180 [lan1qtsn]
[ 1948.321770]  lan1qtsn_spi_read_registers+0x6c/0xe0 [lan1qtsn]
[ 1948.321790]  lan1qtsn_spi_read_register+0x1c/0x30 [lan1qtsn]
[ 1948.321808]  lan1qtsn_get_sys_clock_by_ldev+0x3c/0x88 [lan1qtsn]
[ 1948.321827]  lan1qtsn_netdev_start_xmit+0xc8/0x4d8 [lan1qtsn]
[ 1948.321846]  dev_hard_start_xmit+0xac/0x208
[ 1948.321860]  sch_direct_xmit+0x98/0x1e8
[ 1948.321878]  __dev_queue_xmit+0x544/0xed0
[ 1948.321891]  ip_finish_output2+0x3c4/0x660
[ 1948.321907]  __ip_finish_output+0xac/0x1b0
[ 1948.321921]  ip_finish_output+0x3c/0x120
[ 1948.321935]  ip_output+0x70/0x110
[ 1948.321949]  ip_send_skb+0x58/0x100
[ 1948.321964]  ip_push_pending_frames+0x38/0x68
[ 1948.321980]  ping_v4_sendmsg+0x338/0x630
[ 1948.321994]  inet_sendmsg+0x4c/0x80
[ 1948.322010]  __sock_sendmsg+0x64/0xc0
[ 1948.322026]  __sys_sendto+0x10c/0x170
[ 1948.322042]  __arm64_sys_sendto+0x30/0x48
[ 1948.322058]  invoke_syscall+0x50/0x120
[ 1948.322076]  el0_svc_common.constprop.0+0x48/0xf0
[ 1948.322094]  do_el0_svc+0x24/0x38
[ 1948.322111]  el0_svc+0x30/0x100
[ 1948.322128]  el0t_64_sync_handler+0x100/0x130
[ 1948.322147]  el0t_64_sync+0x190/0x198
[ 1948.322159] ---[ end trace 0000000000000000 ]---
[ 1948.322328] lan1qtsn_netdev_start_xmit - sys_count_lower: 0x02e2e52e  tx_metadata->from.tick: 0x0027d774  tx_metadata->from.priority: 0x7
[ 1948.322345] lan1qtsn_netdev_start_xmit - tx_metadata->to.tick: 0x0027d773  tx_metadata->to.priority: 0x7  tx_metadata->frame_length:  102  tx_metadata->fail_policy: 0
[ 1948.322425] ------------[ cut here ]------------


---

**Cursor**

이 커널 메시지를 분석해보겠습니다. 먼저 관련 코드를 살펴보겠습니다.


이제 문제를 분석해보겠습니다. 커널 메시지를 보면 다음과 같은 문제가 발생했습니다:

## 문제 분석

**"BUG: scheduling while atomic"** 오류가 발생했습니다. 이는 **atomic context에서 스케줄링을 시도할 때** 발생하는 심각한 커널 버그입니다.

### 문제의 원인

1. **호출 스택 분석:**
   ```
   ping_v4_sendmsg → ip_output → dev_hard_start_xmit → lan1qtsn_netdev_start_xmit → 
   lan1qtsn_get_sys_clock_by_ldev → lan1qtsn_spi_read_register → lan1qtsn_spi_transfer → 
   mutex_lock → __mutex_lock_slowpath → schedule
   ```

2. **핵심 문제:**
   - `lan1qtsn_netdev_start_xmit()` 함수는 **softirq context**에서 실행됩니다
   - 이 함수 내에서 `lan1qtsn_get_sys_clock_by_ldev()`를 호출합니다
   - 이 함수는 SPI 레지스터를 읽기 위해 `mutex_lock()`을 사용합니다
   - **atomic context에서 mutex를 사용하면 스케줄링이 발생할 수 있습니다**

### 구체적인 문제점

1. **`lan1qtsn_spi_transfer()` 함수:**
   ```c
   static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, ...) {
       /* Acquire mutex to prevent concurrent SPI access */
       mutex_lock(&tc6->spi_transfer_lock);  // ← 문제!
       // ... SPI 전송 ...
       mutex_unlock(&tc6->spi_transfer_lock);
   }
   ```

2. **`lan1qtsn_get_sys_clock_by_ldev()` 함수:**
   ```c
   sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi *tc6) {
       timestamp_t clock;
       u32 sys_clock_high, sys_clock_low;
       lan1qtsn_spi_read_register(tc6, TSN_SYSCOUNT_H, &sys_clock_high);  // ← mutex 사용
       lan1qtsn_spi_read_register(tc6, TSN_SYSCOUNT_L, &sys_clock_low);   // ← mutex 사용
       clock = ((u64)sys_clock_high << 32) | sys_clock_low;
       return clock;
   }
   ```

## 해결 방법

### 1. **즉시 해결 (권장):**
`lan1qtsn_netdev_start_xmit()`에서 시스템 클록을 읽는 부분을 제거하거나, atomic context에서 안전한 방법으로 변경해야 합니다.

### 2. **코드 수정 방안:**

**방안 A: 시스템 클록 읽기 제거**
```c
// lan1qtsn_netdev_start_xmit에서 이 부분을 제거하거나 주석 처리
// sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
```

**방안 B: spinlock 사용 (더 안전)**
```c
// mutex 대신 spinlock 사용
spin_lock_irqsave(&tc6->spi_spinlock, flags);
// ... SPI 작업 ...
spin_unlock_irqrestore(&tc6->spi_spinlock, flags);
```

**방안 C: workqueue 사용**
```c
// atomic context에서 시스템 클록 읽기를 workqueue로 지연
schedule_work(&priv->clock_read_work);
```

### 3. **권장 해결책:**
가장 간단하고 안전한 방법은 `lan1qtsn_netdev_start_xmit()`에서 시스템 클록을 읽는 부분을 제거하는 것입니다. 이 함수는 패킷 전송에만 집중하고, 시스템 클록은 다른 안전한 컨텍스트에서 읽어야 합니다.

이 문제는 **커널 패닉을 일으킬 수 있는 심각한 버그**이므로 즉시 수정이 필요합니다.

```plaintext
   ping_v4_sendmsg → ip_output → dev_hard_start_xmit → lan1qtsn_netdev_start_xmit → 
   lan1qtsn_get_sys_clock_by_ldev → lan1qtsn_spi_read_register → lan1qtsn_spi_transfer → 
   mutex_lock → __mutex_lock_slowpath → schedule
```

```c
   static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, ...) {
       /* Acquire mutex to prevent concurrent SPI access */
       mutex_lock(&tc6->spi_transfer_lock);  // ← 문제!
       // ... SPI 전송 ...
       mutex_unlock(&tc6->spi_transfer_lock);
   }
```

```c
   sysclock_t lan1qtsn_get_sys_clock_by_ldev(struct lan1qtsn_spi *tc6) {
       timestamp_t clock;
       u32 sys_clock_high, sys_clock_low;
       lan1qtsn_spi_read_register(tc6, TSN_SYSCOUNT_H, &sys_clock_high);  // ← mutex 사용
       lan1qtsn_spi_read_register(tc6, TSN_SYSCOUNT_L, &sys_clock_low);   // ← mutex 사용
       clock = ((u64)sys_clock_high << 32) | sys_clock_low;
       return clock;
   }
```

```c
// lan1qtsn_netdev_start_xmit에서 이 부분을 제거하거나 주석 처리
// sys_count = lan1qtsn_get_sys_clock_by_ldev(priv->tc6);
```

```c
// mutex 대신 spinlock 사용
spin_lock_irqsave(&tc6->spi_spinlock, flags);
// ... SPI 작업 ...
spin_unlock_irqrestore(&tc6->spi_spinlock, flags);
```

```c
// atomic context에서 시스템 클록 읽기를 workqueue로 지연
schedule_work(&priv->clock_read_work);
```

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있어.

SPI 인터페이스를 통해 구성된 네트워 포트를 통해 ping 시험(상대편에서도 ping을 하고, 라즈베리 파이 4에서도 ping을 수행)을 하던 중에
다음과 같은 커널 메시지가 발생했어.
메모리 누수가 발생하는 곳이 있는지 확인해 줘



[  494.737887] ------------[ cut here ]------------
[  494.737906] refcount_t: saturated; leaking memory.
[  494.737949] WARNING: CPU: 0 PID: 1 at lib/refcount.c:22 refcount_warn_saturate+0x74/0x148
[  494.737983] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc binfmt_misc brcmfmac_wcc spidev joydev brcmfmac hci_uart btbcm bluetooth vc4 brcmutil cfg80211 bcm2835_v4l2(C) snd_soc_hdmi_codec bcm2835_codec(C) drm_display_helper bcm2835_isp(C) ecdh_generic rpi_hevc_dec cec bcm2835_mmal_vchiq(C) ecc vc_sm_cma(C) videobuf2_vmalloc v4l2_mem2mem videobuf2_dma_contig rfkill drm_dma_helper libaes snd_soc_core videobuf2_memops videobuf2_v4l2 v3d snd_bcm2835(C) videodev raspberrypi_hwmon i2c_brcmstb snd_compress snd_pcm_dmaengine gpu_sched snd_pcm raspberrypi_gpiomem spi_bcm2835 videobuf2_common drm_shmem_helper snd_timer drm_kms_helper snd mc nvmem_rmem uio_pdrv_genirq uio i2c_dev drm ledtrig_pattern fuse drm_panel_orientation_quirks backlight dm_mod ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[  494.738301] CPU: 0 UID: 0 PID: 1 Comm: systemd Tainted: G        WC O       6.12.41-v8+ #3
[  494.738318] Tainted: [W]=WARN, [C]=CRAP, [O]=OOT_MODULE
[  494.738323] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[  494.738330] pstate: 60000005 (nZCv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  494.738340] pc : refcount_warn_saturate+0x74/0x148
[  494.738354] lr : refcount_warn_saturate+0x74/0x148
[  494.738367] sp : ffffffc08006bd50
[  494.738372] x29: ffffffc08006bd50 x28: ffffff8104108000 x27: 0000000000000000
[  494.738391] x26: ffffff8109aa3a80 x25: 0000000000000002 x24: ffffff8101049650
[  494.738409] x23: ffffff8101049600 x22: ffffff804dacefa0 x21: 0000000000000002
[  494.738425] x20: ffffff8101049600 x19: ffffff804dacefa0 x18: 0000000000000000
[  494.738441] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[  494.738457] x14: 0000000000000000 x13: 2e79726f6d656d20 x12: 676e696b61656c20
[  494.738473] x11: 3b64657461727574 x10: ffffffd3118915f0 x9 : ffffffd31011d1f0
[  494.738489] x8 : 00000000fffffd60 x7 : ffffffd31187d4f0 x6 : 0000000000000d60
[  494.738506] x5 : ffffff8103f793c8 x4 : 0000000000000000 x3 : 0000000000000027
[  494.738521] x2 : 0000000000000000 x1 : 0000000000000000 x0 : ffffff8104108000
[  494.738539] Call trace:
[  494.738545]  refcount_warn_saturate+0x74/0x148
[  494.738560]  fsnotify_get_group+0x84/0x98
[  494.738575]  fsnotify_init_mark+0x3c/0x60
[  494.738588]  __arm64_sys_inotify_add_watch+0x2dc/0x428
[  494.738597]  invoke_syscall+0x50/0x120
[  494.738612]  el0_svc_common.constprop.0+0x48/0xf0
[  494.738624]  do_el0_svc+0x24/0x38
[  494.738636]  el0_svc+0x30/0x100
[  494.738650]  el0t_64_sync_handler+0x100/0x130
[  494.738662]  el0t_64_sync+0x190/0x198
[  494.738672] ---[ end trace 0000000000000000 ]---

---

**Cursor**

이 커널 메시지를 분석해보겠습니다. 메모리 누수와 관련된 문제를 찾아보겠습니다.


이제 문제를 분석해보겠습니다. 커널 메시지를 보면 다음과 같은 문제가 발생했습니다:

## 문제 분석

**"refcount_t: saturated; leaking memory"** 오류가 발생했습니다. 이는 **refcount가 포화되어 메모리 누수가 발생**하고 있음을 나타냅니다.

### 문제의 원인

1. **호출 스택 분석:**
   ```
   __arm64_sys_inotify_add_watch → fsnotify_init_mark → fsnotify_get_group → 
   refcount_warn_saturate
   ```

2. **핵심 문제:**
   - 이 오류는 **fsnotify 시스템**과 관련이 있습니다
   - `inotify_add_watch` 시스템 콜에서 발생했습니다
   - **refcount가 최대값에 도달하여 더 이상 증가할 수 없습니다**

### 구체적인 문제점

1. **fsnotify 그룹 참조 카운트 문제:**
   - fsnotify 그룹의 refcount가 계속 증가하고 있습니다
   - 이는 보통 **그룹이 제대로 해제되지 않을 때** 발생합니다

2. **메모리 누수의 원인:**
   - **`dev_set_drvdata()`가 호출되지 않음**: `tsdev` 구조체가 SPI 디바이스에 연결되지 않았습니다
   - **`dev_get_drvdata()` 호출 시 잘못된 데이터 반환**: 이는 `tsdev_free()`에서 문제를 일으킬 수 있습니다

### 코드의 문제점

**`lan1qtsn_probe()` 함수에서:**
```c
tsdev = tsdev_alloc(sdev);  // tsdev 할당
// ... 
spi_set_drvdata(sdev, priv);  // priv만 설정됨
// tsdev를 SPI 디바이스에 연결하지 않음!
```

**`lan1qtsn_remove()` 함수에서:**
```c
struct lan1qtsn_spi_dev *tsdev = dev_get_drvdata(&priv->sdev->dev);
// 이는 잘못된 데이터를 반환할 수 있음
```

## 해결 방법

### 1. **즉시 해결 (권장):**
`tsdev`를 SPI 디바이스에 올바르게 연결해야 합니다.

### 2. **코드 수정:**

**방안 A: `dev_set_drvdata()` 추가**
```c
static int lan1qtsn_probe(struct spi_device *sdev) {
    // ... 기존 코드 ...
    
    tsdev = tsdev_alloc(sdev);
    if (!tsdev) {
        pr_err("tsdev_alloc failed\n");
        return -ENOMEM;
    }
    
    // tsdev를 SPI 디바이스에 연결
    dev_set_drvdata(&sdev->dev, tsdev);
    
    // ... 나머지 코드 ...
}
```

**방안 B: `tsdev` 참조 방식 변경**
```c
static void lan1qtsn_remove(struct spi_device *spi) {
    struct lan1qtsn_priv *priv = spi_get_drvdata(spi);
    struct lan1qtsn_spi_dev *tsdev = priv->tsdev;  // 직접 참조
    
    // ... 정리 코드 ...
}
```

### 3. **추가 확인사항:**

1. **fsnotify 관련 문제:**
   - 시스템에서 `inotify` 사용을 모니터링
   - `lsof | grep inotify` 명령으로 확인

2. **메모리 사용량 모니터링:**
   - `cat /proc/meminfo`
   - `dmesg | grep -i memory`

3. **커널 로그 확인:**
   - `dmesg | grep -i refcount`
   - `dmesg | grep -i fsnotify`

### 4. **권장 해결책:**
가장 간단하고 안전한 방법은 **방안 B**입니다. `tsdev`를 직접 참조하여 `dev_get_drvdata()` 호출을 피하는 것입니다.

이 문제는 **메모리 누수를 일으키는 심각한 버그**이므로 즉시 수정이 필요합니다.

```plaintext
   __arm64_sys_inotify_add_watch → fsnotify_init_mark → fsnotify_get_group → 
   refcount_warn_saturate
```

```c
tsdev = tsdev_alloc(sdev);  // tsdev 할당
// ... 
spi_set_drvdata(sdev, priv);  // priv만 설정됨
// tsdev를 SPI 디바이스에 연결하지 않음!
```

```c
struct lan1qtsn_spi_dev *tsdev = dev_get_drvdata(&priv->sdev->dev);
// 이는 잘못된 데이터를 반환할 수 있음
```

```c
static int lan1qtsn_probe(struct spi_device *sdev) {
    // ... 기존 코드 ...
    
    tsdev = tsdev_alloc(sdev);
    if (!tsdev) {
        pr_err("tsdev_alloc failed\n");
        return -ENOMEM;
    }
    
    // tsdev를 SPI 디바이스에 연결
    dev_set_drvdata(&sdev->dev, tsdev);
    
    // ... 나머지 코드 ...
}
```

```c
static void lan1qtsn_remove(struct spi_device *spi) {
    struct lan1qtsn_priv *priv = spi_get_drvdata(spi);
    struct lan1qtsn_spi_dev *tsdev = priv->tsdev;  // 직접 참조
    
    // ... 정리 코드 ...
}
```

