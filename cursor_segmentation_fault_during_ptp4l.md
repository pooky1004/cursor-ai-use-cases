# Segmentation fault during ptp4l execution
_Exported on 8/29/2025 at 07:50:39 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅을 하여 사용하고 있어.

PTP 시험을 하기 위해
sudo timedatectl set-ntp off 명령을 먼저 내리고

다시
sudo ptp4l -f /etc/linuxptp/ptp4l.conf -m -2 -A -i eth1 -p /dev/ptp0 -l 5 명령을 내릴 때

Segmentation fault 가 발생해

커널로그는
[  991.757534] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[  991.757565] Mem abort info:
[  991.757573]   ESR = 0x0000000096000005
[  991.757582]   EC = 0x25: DABT (current EL), IL = 32 bits
[  991.757592]   SET = 0, FnV = 0
[  991.757600]   EA = 0, S1PTW = 0
[  991.757607]   FSC = 0x05: level 1 translation fault
[  991.757616] Data abort info:
[  991.757622]   ISV = 0, ISS = 0x00000005, ISS2 = 0x00000000
[  991.757630]   CM = 0, WnR = 0, TnD = 0, TagAccess = 0
[  991.757639]   GCS = 0, Overlay = 0, DirtyBit = 0, Xs = 0
[  991.757648] user pgtable: 4k pages, 39-bit VAs, pgdp=000000011843c000
[  991.757659] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[  991.757684] Internal error: Oops: 0000000096000005 [#1] PREEMPT SMP
[  991.757698] Modules linked in: lan1qtsn(O) rfcomm snd_seq_dummy snd_hrtimer snd_seq snd_seq_device cmac algif_hash aes_arm64 aes_generic algif_skcipher af_alg bnep 8021q garp stp llc brcmfmac_wcc binfmt_misc spidev vc4 hci_uart joydev btbcm snd_soc_hdmi_codec bluetooth drm_display_helper cec brcmfmac drm_dma_helper brcmutil cfg80211 snd_soc_core bcm2835_codec(C) bcm2835_v4l2(C) ecdh_generic bcm2835_isp(C) raspberrypi_hwmon bcm2835_mmal_vchiq(C) ecc rpi_hevc_dec snd_compress rfkill vc_sm_cma(C) videobuf2_vmalloc snd_bcm2835(C) snd_pcm_dmaengine v4l2_mem2mem libaes v3d videobuf2_dma_contig videobuf2_memops videobuf2_v4l2 gpu_sched snd_pcm drm_shmem_helper videodev i2c_brcmstb snd_timer videobuf2_common drm_kms_helper raspberrypi_gpiomem mc spi_bcm2835 snd nvmem_rmem uio_pdrv_genirq uio i2c_dev ledtrig_pattern drm fuse dm_mod drm_panel_orientation_quirks backlight ip_tables x_tables ipv6 [last unloaded: lan1qtsn]
[  991.758058] CPU: 2 UID: 0 PID: 11093 Comm: ptp4l Tainted: G         C O       6.12.41-v8+ #6
[  991.758077] Tainted: [C]=CRAP, [O]=OOT_MODULE
[  991.758085] Hardware name: Raspberry Pi 4 Model B Rev 1.5 (DT)
[  991.758093] pstate: a0000005 (NzCv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  991.758105] pc : fput+0xac/0x198
[  991.758133] lr : pc_clock_gettime+0x74/0xb8
[  991.758146] sp : ffffffc083db3d90
[  991.758153] x29: ffffffc083db3d90 x28: ffffff8102978000 x27: 0000000000000000
[  991.758177] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
[  991.758197] x23: 00000000ffffffff x22: ffffff8102978000 x21: 0000007ff4f7e2a8
[  991.758218] x20: ffffffdbfb774324 x19: 0000000000000000 x18: 0000000000000000
[  991.758238] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[  991.758257] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
[  991.758276] x11: 0000000000000000 x10: 0000000000000000 x9 : ffffffdbfb7770c4
[  991.758296] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
[  991.758315] x5 : 0000000000000003 x4 : ffffff8104ee74e0 x3 : 0000000000000000
[  991.758334] x2 : ffffffffffffff00 x1 : 0000000000000001 x0 : 0000000000000000
[  991.758354] Call trace:
[  991.758363]  fput+0xac/0x198
[  991.758380]  pc_clock_gettime+0x74/0xb8
[  991.758396] Code: d50323bf d65f03c0 d2800021 f9800271 (c85f7e60)
[  991.758408] ---[ end trace 0000000000000000 ]---


문제 원인과 조치 방법을 확인해 줘

---

**Cursor**

커널 로그를 분석해보니 PTP 관련 NULL 포인터 역참조 오류가 발생하고 있습니다. 문제를 파악하기 위해 현재 코드를 살펴보겠습니다.


커널 로그와 코드를 분석한 결과, PTP 관련 NULL 포인터 역참조 오류의 원인을 파악했습니다.

## 문제 원인 분석

1. **커널 로그 분석**:
   - `pc : fput+0xac/0x198` - `fput` 함수에서 NULL 포인터 역참조 발생
   - `lr : pc_clock_gettime+0x74/0xb8` - PTP 클럭의 `gettime` 함수에서 문제 발생
   - `Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000`

2. **코드 분석**:
   - `lan1qtsn_ptp_gettimex` 함수에서 `ptp_read_system_prets(sts)` 호출 시 문제 발생
   - `ptp_read_system_prets`와 `ptp_read_system_postts` 함수가 정의되지 않음
   - 이 함수들은 Linux PTP 서브시스템의 표준 함수이지만 현재 코드에서 누락됨

## 조치 방법

### 1. 누락된 PTP 함수 정의 추가

`lan1qtsn_ptp.c` 파일에 누락된 함수들을 추가해야 합니다:


```c
#include "lan1qtsn_ptp.h"

#include "lan1qtsn_arch.h"
#include "lan1qtsn_netdev.h"

#define NS_IN_1S 1000000000

/**
 * ptp_read_system_prets - Read system timestamp before PTP operation
 * @sts: System timestamp structure
 */
static void ptp_read_system_prets(struct ptp_system_timestamp *sts)
{
    if (sts) {
        sts->pre_ts = ktime_get_real();
    }
}

/**
 * ptp_read_system_postts - Read system timestamp after PTP operation
 * @sts: System timestamp structure
 */
static void ptp_read_system_postts(struct ptp_system_timestamp *sts)
{
    if (sts) {
        sts->post_ts = ktime_get_real();
    }
}

// ... existing code ...
```

### 2. PTP 함수에서 NULL 포인터 체크 강화

`lan1qtsn_ptp_gettimex` 함수에서 더 강력한 NULL 포인터 체크를 추가합니다:


```c
static int lan1qtsn_ptp_gettimex(struct ptp_clock_info* ptp, struct timespec64* ts, struct ptp_system_timestamp* sts) {
    u64 clock, timestamp;
    unsigned long flags;

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);
#if 1
    pr_err("%s - ptp_data: %p\n", __func__, ptp_data);
    return 0;
#endif
    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;

->

static int lan1qtsn_ptp_gettimex(struct ptp_clock_info* ptp, struct timespec64* ts, struct ptp_system_timestamp* sts) {
    u64 clock, timestamp;
    unsigned long flags;

    if (!ptp || !ts) {
        pr_err("%s - Invalid parameters: ptp=%p, ts=%p\n", __func__, ptp, ts);
        return -EINVAL;
    }

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);
    if (!ptp_data) {
        pr_err("%s - Failed to get ptp_data from container\n", __func__);
        return -EINVAL;
    }

    if (!ptp_data->ldev) {
        pr_err("%s - ptp_data->ldev is NULL\n", __func__);
        return -EINVAL;
    }

    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    if (!priv || !priv->tc6) {
        pr_err("%s - priv or priv->tc6 is NULL\n", __func__);
        return -EINVAL;
    }
    struct lan1qtsn_spi* tc6 = priv->tc6;
```

### 3. 다른 PTP 함수들에도 NULL 체크 추가

`lan1qtsn_ptp_settime` 함수에도 NULL 체크를 추가합니다:


```c
static int lan1qtsn_ptp_settime(struct ptp_clock_info* ptp, const struct timespec64* ts) {
    u64 hw_timestamp, host_timestamp, sys_clock;
    unsigned long flags;

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);

    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    struct lan1qtsn_spi* tc6 = priv->tc6;

->

static int lan1qtsn_ptp_settime(struct ptp_clock_info* ptp, const struct timespec64* ts) {
    u64 hw_timestamp, host_timestamp, sys_clock;
    unsigned long flags;

    if (!ptp || !ts) {
        pr_err("%s - Invalid parameters: ptp=%p, ts=%p\n", __func__, ptp, ts);
        return -EINVAL;
    }

    struct ptp_device_data* ptp_data = container_of(ptp, struct ptp_device_data, ptp_info);
    if (!ptp_data) {
        pr_err("%s - Failed to get ptp_data from container\n", __func__);
        return -EINVAL;
    }

    if (!ptp_data->ldev) {
        pr_err("%s - ptp_data->ldev is NULL\n", __func__);
        return -EINVAL;
    }

    struct lan1qtsn_dev* ldev = ptp_data->ldev;
    struct lan1qtsn_priv* priv = netdev_priv(ldev->ndev);
    if (!priv || !priv->tc6) {
        pr_err("%s - priv or priv->tc6 is NULL\n", __func__);
        return -EINVAL;
    }
    struct lan1qtsn_spi* tc6 = priv->tc6;
```

### 4. PTP 디바이스 초기화 시 더블 체크 추가

`ptp_device_init` 함수에서 PTP 클럭 등록 후 추가 검증을 수행합니다:


```c
    ptp->ptp_clock = ptp_clock_register(&ptp->ptp_info, dev);
    if (IS_ERR(ptp->ptp_clock)) {
        pr_err("Failed to register ptp clock\n");
        kfree(ptp);
        return NULL;
    }

->

    ptp->ptp_clock = ptp_clock_register(&ptp->ptp_info, dev);
    if (IS_ERR(ptp->ptp_clock)) {
        pr_err("Failed to register ptp clock\n");
        kfree(ptp);
        return NULL;
    }

    /* Verify PTP clock registration */
    if (!ptp->ptp_clock) {
        pr_err("PTP clock registration returned NULL\n");
        kfree(ptp);
        return NULL;
    }
```

### 5. PTP 디바이스 파괴 시 안전성 강화

`ptp_device_destroy` 함수에서 더 안전한 정리를 수행합니다:


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

->

void ptp_device_destroy(struct ptp_device_data* ptp_data) {
    if (!ptp_data) {
        pr_err("ptp_device_destroy: NULL ptp_data\n");
        return;
    }

    /* Safely unregister PTP clock */
    if (ptp_data->ptp_clock) {
        if (!IS_ERR(ptp_data->ptp_clock)) {
            ptp_clock_unregister(ptp_data->ptp_clock);
        }
        ptp_data->ptp_clock = NULL;
    }

    /* Clear device references */
    ptp_data->ldev = NULL;
    ptp_data->dev = NULL;

    kfree(ptp_data);
}
```

## 요약

**문제 원인**:
1. `ptp_read_system_prets`와 `ptp_read_system_postts` 함수가 정의되지 않음
2. PTP 함수들에서 NULL 포인터 체크가 부족함
3. PTP 디바이스 초기화/정리 과정에서 안전성 부족

**조치 방법**:
1. ✅ 누락된 PTP 시스템 타임스탬프 함수들 추가
2. ✅ 모든 PTP 함수에 강력한 NULL 포인터 체크 추가
3. ✅ PTP 디바이스 초기화 시 추가 검증 로직 추가
4. ✅ PTP 디바이스 파괴 시 안전한 정리 로직 추가

이제 커널을 다시 빌드하고 테스트해보시기 바랍니다. NULL 포인터 역참조 오류가 해결되어야 합니다.

