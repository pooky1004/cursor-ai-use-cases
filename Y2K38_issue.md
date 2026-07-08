# Y2K38 Issue — Application-Level Fix (ELDK 3.1.1 / PPC32)

**환경:** 32비트 Linux, PowerPC, ELDK 3.1.1  
**제약:** `time_t`가 32비트 signed이며, 64비트로 확장하는 툴체인/glibc 옵션 없음  
**목표:** Application(ABI + 라이브러리) 레벨에서 2038년 문제를 우회하고, 이벤트 로그 데몬(A/B)이 정상 동작하도록 조치  
**프로젝트 경로:** `C:\Users\user\projects\y2k38-abi`  
**작성일:** 2026-07-08  
**최종 갱신:** 2026-07-08 (교차 빌드·커널 offset 보정·배포 스크립트 반영)

### 관련 문서

| 문서 | 내용 |
|------|------|
| [`docs/ABI_DESIGN.md`](docs/ABI_DESIGN.md) | ABI 설계·툴체인 통합 상세 |
| [`docs/CROSS_BUILD.md`](docs/CROSS_BUILD.md) | ELDK 교차 빌드·stage·deploy |
| [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md) | wrap 복구·`/etc/y2k38_offset`·calibrate |
| [`README.md`](README.md) | 빠른 시작 |

---

## 1. 문제 정의

### 1.1 기술적 원인

Unix epoch 초를 담는 전통적 `time_t`가 **signed 32-bit**이면 표현 가능 최댓값은 다음과 같다.

| 값 | 의미 |
|----|------|
| `0x7FFFFFFF` (`2147483647`) | 2038-01-19 03:14:07 UTC |
| `0x80000000` (`2147483648`) | 다음 초. signed 32-bit에서는 **음수**로 해석됨 |

이후 `time()`, `gettimeofday()`, `localtime()`, `ctime()`, `timer` 관련 API, `future - now` 형태의 **시간차 연산**, 파일에 `%ld`로 남기는 **벽시계 로그**가 깨진다.

ELDK 3.1.1 / 구형 glibc에는 현대 glibc의 `_TIME_BITS=64` 같은 옵션이 없고, 커널/libc ABI의 `time_t`를 바꾸려면 **전면 재빌드·ABI 파탄**이 난다. 따라서 본 작업은 libc `time_t`를 교체하지 않고 **병행 ABI**를 둔다.

### 1.2 본 과제에서 깨지는 데몬 시나리오

| 구성 | 역할 | Y2K38 실패 모드 |
|------|------|-----------------|
| **Daemon A** | 이벤트 + 시간 정보를 로그에 저장 | `time_t` / `%ld`로 기록 → 2038 이후·먼 미래 시각이 **잘리거나 음수**로 저장 |
| **Daemon B** | 불규칙 미래 이벤트 시각과 현재 시각의 **차이**를 주기 계산 후 파일 저장 | `time_t` 차분 → **오버플로/잘못된 delta**. A가 이미 망가진 로그를 읽으면 이중 실패 |

추가로, 로그 포맷을 64-bit로 고쳐도 **커널 벽시계가 wrap**되면 `now` 자체가 틀어져 Daemon B의 delta가 의미를 잃는다. 이를 위해 **kernel offset 보정**이 필요하다.

---

## 2. 처리 방침 (설계 결정)

1. **libc `time_t`는 유지**한다. (ELDK 재빌드 최소화)
2. 새 ABI 타입 **`y2k38_time_t` = signed 64-bit (`int64_t`)** 를 정의한다.
3. 시간 API·로그·IPC·파일 포맷은 모두 `y2k38_*` 경계로 옮긴다.
4. 공유 라이브러리 **`liby2k38safe`** 로 ABI를 구현·배포한다. (저장/연산 + **kernel offset 복구**)
5. Daemon A/B는 해당 라이브러리만 사용하고, 기동 시 `/etc/y2k38_offset`(또는 `--offset-file`)을 로드한다.
6. 툴체인 통합은 **sysroot에 헤더/라이브러리 설치 + specs/환경·교차 빌드 스크립트**를 1순위로 한다. GCC `time_t` 패치는 하지 않는다(스케치만 문서화).
7. 보드 배포는 `stage` 트리 + `deploy-board.sh`로 표준화한다. 데몬/툴은 기본 **정적 링크**(`LINK_STATIC=1`).

---

## 3. 처리 과정

### 3.1 프로젝트 골격 생성

- 루트 저장소 초기화 (`C:\Users\user\projects\y2k38-abi`)
- 디렉터리 구성:
  - `docs/` — ABI, 교차 빌드, kernel offset
  - `include/y2k38/`, `lib/` — ABI 헤더 + `liby2k38safe`
  - `examples/broken`, `examples/fixed` — 문제/해결·offset 데모
  - `daemons/daemon_a`, `daemons/daemon_b`
  - `tools/` — `y2k38_offsetctl`
  - `toolchain/abi`, `toolchain/patches`, `scripts/`

### 3.2 ABI 설계

| 심볼 | 폭 | 용도 |
|------|----|------|
| `y2k38_time_t` | 8바이트 signed | epoch 초 |
| `struct y2k38_timeval` | 16바이트 | 초+usec (+pad), PPC32 EABI 정렬 |
| `struct y2k38_timespec` | 16바이트 | 초+nsec |
| `struct y2k38_tm` | `struct tm` 유사 | 달력 분해 |
| ABI version | major 1 / minor 0 | soname `liby2k38safe.so.1` |

**경계 규칙:** 프로세스·파일·데몬 간에 벽시계를 넘길 때 `time_t` 금지. 직렬화는 **64비트 십진 ASCII**.

**벽시계 복구식 (kernel wrap 시):**

```
real_utc = sign_extend(kernel_time_t) + OFFSET
```

### 3.3 툴체인(ELDK) 추가 절차

#### 권장: Sysroot ABI 패키지 + 교차 빌드 스크립트

```bash
export ELDK_ARCH=ppc_82xx          # 보드에 맞게 (ppc_8xx 등)
export ELDK_ROOT=/opt/eldk
export CROSS=${ELDK_ARCH}-
export SYSROOT=${ELDK_ROOT}/${ELDK_ARCH}

./scripts/cross-build-eldk.sh                 # 빌드
./scripts/cross-build-eldk.sh install-sysroot # $SYSROOT/usr 에 ABI 설치
# 또는:
make install-sysroot CROSS=$CROSS SYSROOT=$SYSROOT
```

설치 위치:

```
$SYSROOT/usr/include/y2k38/*.h
$SYSROOT/usr/include/y2k38.h
$SYSROOT/usr/lib/liby2k38safe.a
$SYSROOT/usr/lib/liby2k38safe.so*
$SYSROOT/usr/lib/gcc-specs/y2k38.specs
```

앱 링크 예:

```bash
. scripts/environment-setup-y2k38.sh
y2k38_gcc daemon_a.c -ly2k38safe -o daemon_a
# 또는
ppc_82xx-gcc --sysroot=$SYSROOT \
  -specs=$SYSROOT/usr/lib/gcc-specs/y2k38.specs \
  -I$SYSROOT/usr/include daemon_a.c \
  -L$SYSROOT/usr/lib -ly2k38safe -o daemon_a
```

보조물:

| 경로 | 역할 |
|------|------|
| `scripts/cross-build-eldk.sh` | `all` / `stage` / `tarball` / `install-sysroot` |
| `scripts/deploy-board.sh` | staging → 보드 scp/rsync |
| `scripts/board-init-y2k38.sh` | 부팅 시 offset 적용·데몬 기동 |
| `scripts/environment-setup-y2k38.sh` | `CROSS`/`SYSROOT`/`y2k38_gcc` |
| `toolchain/abi/y2k38.specs` | GCC include/lib 경로 |

#### 선택: Sysroot flavor 복제

```bash
cp -a /opt/eldk/ppc_82xx /opt/eldk/ppc_82xx_y2k38
SYSROOT=/opt/eldk/ppc_82xx_y2k38 ./scripts/cross-build-eldk.sh install-sysroot
```

#### 비권장: GCC 매크로 패치

- `toolchain/patches/gcc-eldk-y2k38-define.patch` — `__Y2K38_ABI__` define **스케치**
- `time_t` 자체를 바꾸지 않음 → 실무는 sysroot 설치로 충분

### 3.4 라이브러리 `liby2k38safe` 구현

| 파일 | 내용 |
|------|------|
| `include/y2k38/types.h` | `y2k38_time_t` 및 구조체 |
| `include/y2k38/time.h` | clock / diff / format / parse / offset API |
| `include/y2k38/eventlog.h` | Daemon용 `EVENT` 라인 I/O |
| `lib/y2k38_time.c` | raw/recover clock, mock, civil time, `y2k38_difftime_sec` |
| `lib/y2k38_offset.c` | offset compute/load/save/`2^32` wrap / default path |
| `lib/y2k38_eventlog.c` | `EVENT <id> <epoch64> <msg>` append/parse |

주요 API:

| API | 역할 |
|-----|------|
| `y2k38_time()` / `y2k38_gettimeofday()` | 복구된 UTC (`raw + offset`) |
| `y2k38_time_kernel_raw()` | offset 미적용 kernel 초 |
| `y2k38_difftime_sec()` / `y2k38_timeval_diff()` | 64-bit 차분 |
| `y2k38_format_epoch()` / `y2k38_parse_epoch()` / `y2k38_format_iso8601_utc()` | 직렬화 |
| `y2k38_gmtime_r()` / `y2k38_timegm()` | 32-bit libc `gmtime` 비의존 civil 변환 |
| `y2k38_clock_set/get_kernel_offset()` | in-process OFFSET |
| `y2k38_clock_compute_offset(true, raw)` | `true - raw` |
| `y2k38_clock_u32_wrap_offset(n)` | `n * 2^32` |
| `y2k38_clock_load/save_offset_file()` | `/etc/y2k38_offset` 영속화 |
| `y2k38_clock_apply_offset_default()` | path / `Y2K38_KERNEL_OFFSET_FILE` / 기본 경로 |
| `y2k38_clock_set_mock()` | 절대 UTC mock (테스트) |
| `y2k38_clock_set_mock_kernel()` | signed 32-bit kernel residual mock |

기본 offset 경로: `/etc/y2k38_offset`  
환경 변수: `Y2K38_KERNEL_OFFSET_FILE`

### 3.5 Daemon 구현

공통 옵션:

- `--offset-file PATH` — kernel offset 로드
- `--no-offset-file` — 자동 로드 생략
- `--mock-now EPOCH` — 절대 UTC mock (kernel+offset 무시)
- `--mock-kernel SEC` — wrap된 kernel 초 에뮬레이션
- `--once`

#### Daemon A (`daemons/daemon_a/daemon_a.c`)

- 로그: `EVENT <id> <64bit-epoch> <msg>`
- `--once` 시드: `2147483648`(overflow+1s), `4102444800`(2100-01-01), now+120s
- stdin: `EVENT` / `SCHED` 명령

#### Daemon B (`daemons/daemon_b/daemon_b.c`)

- `delta = y2k38_difftime_sec(when, now)`
- 출력: `DELTA <id> <seconds> <when> <now> <msg>`
- `HB`(heartbeat)는 미래 이벤트에서 제외
- 주기 `period_sec` 또는 `--once`

### 3.6 보드 유틸·예제

| 경로 | 목적 |
|------|------|
| `tools/y2k38_offsetctl.c` | `show` / `set` / `set-u32-wrap` / `calibrate` |
| `examples/broken/y2k38_broken.c` | 32-bit `time_t` 에뮬레이션 → 잘림·delta 실패 |
| `examples/broken/broken_daemon_pipeline.c` | A→B 파이프라인 실패 요약 |
| `examples/fixed/y2k38_fixed.c` | 동일 시나리오를 라이브러리로 통과 |
| `examples/fixed/kernel_offset_demo.c` | wrap → OFFSET 파일 → UTC 복구 데모 |

> 현대 64-bit 호스트는 `sizeof(time_t)==8`이라 broken 예제는 **의도적으로 `int32_t` 저장을 시뮬레이트**한다. 실제 ELDK PPC32에서는 캐스트만으로 동일 실패가 난다.

### 3.7 빌드·검증 (호스트)

```bash
make
make check
make check-offset
./examples/broken/y2k38_broken
./examples/fixed/y2k38_fixed
./examples/fixed/kernel_offset_demo

# 데몬 (pre-wrap mock)
./daemons/daemon_a/daemon_a /tmp/events.log \
  --mock-now 2147483600 --once --no-offset-file
./daemons/daemon_b/daemon_b /tmp/events.log /tmp/deltas.out \
  --mock-now 2147483600 --once --no-offset-file

# 데몬 (post-wrap: offset 복구)
./tools/y2k38_offsetctl calibrate 2147483748 \
  --file /tmp/y2k38_off.conf --mock-kernel -2147483548
./daemons/daemon_a/daemon_a /tmp/ev_wrap.log \
  --offset-file /tmp/y2k38_off.conf --mock-kernel -2147483548 --once
./daemons/daemon_b/daemon_b /tmp/ev_wrap.log /tmp/d_wrap.out \
  --offset-file /tmp/y2k38_off.conf --mock-kernel -2147483548 --once
```

본 작업 환경에서는 **WSL(Ubuntu gcc 9.4)** 로 호스트 빌드·`check`·`check-offset`·daemon offset 연동·`make stage`까지 검증했다.  
실제 `ppc_82xx-` 교차 빌드는 ELDK가 설치된 빌드 호스트에서 `./scripts/cross-build-eldk.sh`로 수행한다.

### 3.8 보드 교차 빌드·배포 과정

```bash
export ELDK_ARCH=ppc_82xx ELDK_ROOT=/opt/eldk
./scripts/cross-build-eldk.sh stage
./scripts/deploy-board.sh root@BOARD:/

# 보드에서 (커널이 wrap된 경우)
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
y2k38_offsetctl show
daemon_a /var/log/y2k38_events.log --offset-file /etc/y2k38_offset &
daemon_b /var/log/y2k38_events.log /var/log/y2k38_deltas.out 5 \
  --offset-file /etc/y2k38_offset &
```

- `LINK_STATIC=1`(기본): 데몬/툴에 `liby2k38safe.a` 내장 → 보드 `LD_LIBRARY_PATH` 불필요
- `make stage`: `staging/usr/{bin,lib,include}`, `staging/etc/y2k38_offset`
- 부팅 훅: `scripts/board-init-y2k38.sh` (`Y2K38_START_DAEMONS=1`)

---

## 4. 결과물

### 4.1 디렉터리 트리 (요약)

```
y2k38-abi/
├── Y2K38_issue.md              ← 본 문서
├── README.md
├── Makefile                    # host / cross / stage / check-offset
├── docs/
│   ├── ABI_DESIGN.md
│   ├── CROSS_BUILD.md
│   └── KERNEL_OFFSET.md
├── include/y2k38/              # types.h, time.h, eventlog.h
├── include/y2k38.h
├── lib/                        # time, eventlog, offset → liby2k38safe
├── tools/y2k38_offsetctl.c
├── examples/broken/
├── examples/fixed/             # y2k38_fixed, kernel_offset_demo
├── daemons/daemon_a|daemon_b/
├── scripts/
│   ├── cross-build-eldk.sh
│   ├── deploy-board.sh
│   ├── board-init-y2k38.sh
│   ├── environment-setup-y2k38.sh
│   └── abi_check.c
└── toolchain/
    ├── abi/                    # y2k38.specs, README
    └── patches/                # GCC define sketch
```

### 4.2 Broken 예제 실행 결과 (발췌)

```
=== Scenario 1: store post-2038 time in 32-bit time_t ===
true future (int64) = 4102444800
stored as time32_t  = -192522496
FAIL: truncated/wrapped — classic Y2K38 storage bug

=== Scenario 2: delta near Y2K38 boundary ===
wanted future      = 2147484147
stored future32    = -2147483149
delta (future-now) = -4294966696  (wanted 600)
FAIL: delta wrong due to 32-bit wrap / truncation

=== Scenario 3: daemon A/B pipeline with time32_t ===
intended epoch     = 2147483657
logged time32      = -2147483639
daemon_b delta     = -4294967236  (wanted 60)
FAIL: Daemon B would persist a bogus DELTA after Y2K38
```

**해석:** Daemon A가 미래 시각을 32-bit에 넣으면 음수/잘림이 되고, Daemon B의 `future - now`는 수백억 단위의 잘못된 값이 된다.

### 4.3 Fixed 예제 실행 결과 (발췌)

```
sizeof(y2k38_time_t)=8

=== Scenario 1 ===
stored (int64) = 4102444800
ISO-8601 UTC   = 2100-01-01T00:00:00Z
OK: past signed 32-bit time_t max, still representable

=== Scenario 2 ===
delta = 600 (expected 600)
OK: 64-bit delta correct across the 2038 boundary

=== Scenario 3 ===
logged when = 2147483657
delta       = 60 (expected 60)
OK: durable log + delta survive the Y2K38 boundary
```

### 4.4 Daemon A / B 연동 (pre-wrap mock)

`--mock-now 2147483600` (overflow 48초 전), `--no-offset-file`:

**events.log**

```
EVENT FUT2038 2147483648 first-second-after-time_t-overflow
EVENT FAR2100 4102444800 century-maintenance-window
EVENT NEAR 2147483720 relative-plus-120s
```

**deltas.out**

```
# recomputed at 2147483600
DELTA FUT2038 48 2147483648 2147483600 first-second-after-time_t-overflow
DELTA FAR2100 1954961200 4102444800 2147483600 century-maintenance-window
DELTA NEAR 120 2147483720 2147483600 relative-plus-120s
```

| 이벤트 | 기대 | 결과 |
|--------|------|------|
| FUT2038 | 48초 후 | **48** OK |
| FAR2100 | 2100까지 초 차이 | **1954961200** OK |
| NEAR | 120초 후 | **120** OK |

ABI layout (`make check`):

```
sizeof(y2k38_time_t)=8 (expect 8)
sizeof(struct y2k38_timeval)=16 (expect 16)
sizeof(struct y2k38_timespec)=16 (expect 16)
ABI CHECK OK
```

### 4.5 Kernel offset 복구 (`make check-offset`)

```
true UTC          = 2147483748
kernel as int32   = -2147483548  (emulated wrap)
computed offset   = 4294967296
u32 wrap x1       = 4294967296
recovered UTC     = 2147483748 (2038-01-19T03:15:48Z)
OK: wrap recovered via OFFSET file
delta to 2100-01-01 = 1954961052 s
OK: post-wrap delta arithmetic
```

`y2k38_offsetctl show` (동일 mock):

```
offset_file = /tmp/y2k38_off.conf
offset      = 4294967296
kernel_raw  = -2147483548
utc_now     = 2147483748 (2038-01-19T03:15:48Z)
```

### 4.6 Daemon A / B + offset (post-wrap)

`--offset-file` + `--mock-kernel -2147483548` (복구된 now = `2147483748`):

**events**

```
EVENT FUT2038 2147483648 ...
EVENT FAR2100 4102444800 ...
EVENT NEAR 2147483868 relative-plus-120s
```

**deltas**

```
# recomputed at 2147483748
DELTA FUT2038 -100 ...          # 이미 100초 지난 overflow 시점 이벤트
DELTA FAR2100 1954961052 ...    # 2100까지 양수 유지
DELTA NEAR 120 ...
```

| 항목 | 의미 |
|------|------|
| FUT2038 = **-100** | wrap 복구 후 “지금”이 overflow+100s이므로, overflow 시점은 **과거** — 정상 |
| FAR2100 | 양수 거대 delta — **32-bit라면 붕괴했을 값**이 유지됨 |
| NEAR = 120 | now+120 스케줄 정상 |

### 4.7 Staging 배포 트리 (`make stage`)

```
staging/etc/y2k38_offset
staging/usr/bin/daemon_a
staging/usr/bin/daemon_b
staging/usr/bin/y2k38_offsetctl
staging/usr/bin/kernel_offset_demo
staging/usr/bin/y2k38_fixed
staging/usr/include/y2k38/*.h
staging/usr/lib/liby2k38safe.a
staging/usr/lib/liby2k38safe.so*
staging/usr/share/y2k38/board-init-y2k38.sh
```

---

## 5. 교차 빌드 (보드 / ELDK) 요약

상세: [`docs/CROSS_BUILD.md`](docs/CROSS_BUILD.md)

```bash
export ELDK_ARCH=ppc_82xx
export ELDK_ROOT=/opt/eldk
./scripts/cross-build-eldk.sh              # PPC32 바이너리
./scripts/cross-build-eldk.sh stage        # ./staging
./scripts/cross-build-eldk.sh tarball      # y2k38-abi-ppc_82xx.tar.gz
./scripts/cross-build-eldk.sh install-sysroot
./scripts/deploy-board.sh root@BOARD:/
```

Make 등가:

```bash
make cross CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx LINK_STATIC=1
make stage CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
make install-sysroot CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
```

보드 ELF 확인:

```bash
ppc_82xx-readelf -h daemons/daemon_a/daemon_a
# Machine: PowerPC, Class: ELF32
```

---

## 6. 커널 offset 보정 시나리오 요약

상세: [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md)

| 시나리오 | 조치 |
|----------|------|
| **A. Pre-2038, 커널 정상** | `OFFSET 0` (또는 파일 없음). 앱은 64-bit **저장/연산**만으로 미래 스케줄 보존 |
| **B. uint32 wrap 1회 가정** | `y2k38_offsetctl set-u32-wrap 1` → `OFFSET=4294967296` |
| **C. 신뢰 UTC로 교정 (권장)** | `y2k38_offsetctl calibrate <true_utc>` (GPS/NTP/호스트 `date +%s`) |
| **D. 데몬 기동** | A/B 모두 동일 `--offset-file` (또는 env / 기본 `/etc/y2k38_offset`) |

offset 파일 형식:

```
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 4294967296
```

**한계:** Offset은 유저스페이스 정책이다. 커널 timestore를 고치지 않으며, 프로세스 간 OFFSET 불일치가 없도록 **단일 파일·단일 교정자**를 유지한다. 절대 시각 커널 타이머는 상대 대기(`y2k38_difftime_sec` + clamp)를 권장한다.

---

## 7. 애플리케이션 준수 체크리스트

- [ ] 영속 로그·IPC에 `time_t` / `struct timeval.tv_sec` 벽시계 저장 금지
- [ ] 교차 프로세스 필드는 `y2k38_time_t` / `int64_t`
- [ ] 차분은 `y2k38_difftime_sec` (또는 동등한 int64 연산)
- [ ] 스케줄 파싱은 `y2k38_parse_epoch` / `strtoll`
- [ ] `-ly2k38safe`(또는 정적 `liby2k38safe.a`), soname major 일치
- [ ] 단위 테스트: `now = INT32_MAX - 10`, `future = INT32_MAX + 100`
- [ ] wrap 테스트: `--mock-kernel` + offset calibrate
- [ ] 보드: `/etc/y2k38_offset` 교정 후 `y2k38_offsetctl show`로 신뢰 시계와 일치 확인
- [ ] Daemon A/B가 동일 offset 파일을 쓰는지 확인

---

## 8. 한계 및 후속 작업

1. Offset은 **유저스페이스 정책** — 커널/RTC 하드웨어 버그는 별도(GPS/NTP/배터리) 영역이다.
2. `y2k38_localtime_r`는 현재 **UTC와 동일** 취급. TZ가 필요하면 플러그인 추가.
3. `timer_settime` 등 절대 커널 타이머는 여전히 32-bit일 수 있음 → **상대 대기 + clamp**.
4. 레거시 4바이트 time 필드 로그는 **포맷 버전 bump**로 마이그레이션.
5. 실보드에서 `ppc_82xx-`로 빌드한 ELF/`readelf`·실시계 calibrate를 한 번 더 확인.

---

## 9. 요약

| 항목 | 내용 |
|------|------|
| 문제 | 32-bit `time_t`로 인한 2038 이후 시각 저장·시간차 붕괴 (Daemon A/B) + 커널 wrap 시 `now` 왜곡 |
| 접근 | 병행 ABI `y2k38_time_t` + `liby2k38safe` (libc `time_t` 비변경) |
| 툴체인 | ELDK sysroot 설치, specs, `cross-build-eldk.sh` / stage / deploy |
| 시계 wrap | `/etc/y2k38_offset` + `y2k38_offsetctl calibrate` |
| 데몬 | `--offset-file` / `--mock-kernel` / 64-bit `EVENT`·`DELTA` |
| 검증 | broken 실패, fixed·daemon·`check-offset`·stage 정상 (WSL host) |

**실무 한 줄:** 벽시계는 항상 `y2k38_time_t`로 다루고, 보드 시계가 wrap되면 **한곳의 OFFSET 파일**로 보정한 뒤 Daemon A/B를 기동한다.
