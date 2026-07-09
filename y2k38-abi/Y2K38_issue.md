# Y2K38 Issue — 프로젝트 생성부터 결과물까지

**환경:** 32비트 Linux, PowerPC, ELDK 3.1.1  
**제약:** `time_t`가 32비트 signed이며, 64비트로 확장하는 툴체인/glibc 옵션 없음  
**목표:** Application(ABI + 라이브러리) 레벨에서 2038년 문제를 우회하고, 이벤트 로그 데몬(A/B)이 정상 동작하도록 조치  
**프로젝트 경로:** `C:\Users\user\projects\y2k38-abi`  
**작성일:** 2026-07-08  
**최종 갱신:** 2026-07-09

### 관련 문서

| 문서 | 내용 |
|------|------|
| [`docs/ABI_DESIGN.md`](docs/ABI_DESIGN.md) | ABI 설계·툴체인 통합 상세 |
| [`docs/CROSS_BUILD.md`](docs/CROSS_BUILD.md) | ELDK 교차 빌드·stage·deploy |
| [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md) | wrap 복구·`/etc/y2k38_offset`·calibrate |
| [`README.md`](README.md) | 빠른 시작 |

---

## 0. 과제 배경 (프로젝트 착수 이유)

### 0.1 요구 사항

32비트 PPC 리눅스 보드(ELDK 3.1.1)에서 `time_t`를 64비트로 바꿀 수 없는 상황에서, Application 레벨로 2038년 문제를 해결해야 한다.

구체적 요구:

1. **새 ABI** — `y2k38_time_t`(64-bit)를 정의하고 ELDK 툴체인에 추가
2. **라이브러리** — 시간 저장·연산·로그가 Y2K38에서 깨지지 않도록 `liby2k38safe` 작성
3. **데몬 A** — 이벤트 + 시간 정보를 포함한 로그 저장
4. **데몬 B** — 불규칙 미래 이벤트 시각과 현재 시각의 차이를 주기 계산 후 파일 저장
5. **예제** — 문제 발생 코드(broken) / 라이브러리 적용 코드(fixed)를 C로 작성
6. **보드 대응** — 교차 빌드·커널 offset 보정 시나리오까지 정리

### 0.2 핵심 문제

| 시점 | 현상 |
|------|------|
| `time_t` = `0x7FFFFFFF` | 2038-01-19 03:14:07 UTC (마지막 정상 초) |
| `time_t` = `0x80000000` | 다음 초. signed 32-bit에서 **음수**로 해석 |
| 이후 | `time()`, `gettimeofday()`, `future - now`, `%ld` 로그 등 전면 붕괴 |

ELDK 3.1.1에는 glibc `_TIME_BITS=64` 옵션이 없으므로, **libc `time_t`는 그대로 두고** 병행 ABI를 도입하는 것이 현실적이다.

---

## 1. 프로젝트 생성

### 1.1 저장소·디렉터리 초기화

```
C:\Users\user\projects\y2k38-abi\
```

수행 내용:

1. 프로젝트 루트 디렉터리 생성
2. `git init` — 로컬 Git 저장소 초기화
3. 아래 디렉터리 트리 구성

```
y2k38-abi/
├── docs/                    # 설계·운영 문서
├── include/y2k38/           # 공개 ABI 헤더
├── lib/                     # liby2k38safe 소스
├── examples/broken/         # Y2K38 실패 시연
├── examples/fixed/          # 라이브러리 적용 예제
├── daemons/daemon_a/        # 이벤트 로그 데몬
├── daemons/daemon_b/        # delta 계산 데몬
├── tools/                   # y2k38_offsetctl
├── scripts/                 # 빌드·배포·검증 스크립트
├── toolchain/abi/           # GCC specs
├── toolchain/patches/       # GCC 패치 스케치
├── Makefile
├── README.md
└── Y2K38_issue.md           # 본 문서
```

### 1.2 개발 단계 타임라인

| 단계 | 작업 | 산출물 |
|------|------|--------|
| **1** | 프로젝트 골격·ABI 설계 | `docs/ABI_DESIGN.md`, `include/y2k38/*.h` |
| **2** | `liby2k38safe` 구현 | `lib/y2k38_time.c`, `y2k38_eventlog.c` |
| **3** | broken / fixed 예제 | `examples/broken/`, `examples/fixed/` |
| **4** | Daemon A / B | `daemons/daemon_a/`, `daemon_b/` |
| **5** | Makefile·호스트 빌드 검증 | WSL gcc로 `make check` 통과 |
| **6** | 커널 offset 보정 | `lib/y2k38_offset.c`, `y2k38_offsetctl`, `kernel_offset_demo` |
| **7** | 교차 빌드·배포 | `cross-build-eldk.sh`, `deploy-board.sh`, `make stage` |
| **8** | 문서 통합 | `Y2K38_issue.md`, `CROSS_BUILD.md`, `KERNEL_OFFSET.md` |

---

## 2. 설계 방침

1. **libc `time_t`는 유지** — ELDK/glibc 재빌드 최소화
2. **`y2k38_time_t` = signed 64-bit** — 프로세스·파일·데몬 경계의 벽시계 타입
3. **`liby2k38safe`** — 64-bit 저장/연산 + kernel offset 복구
4. **툴체인 통합** — sysroot에 헤더/lib 설치, GCC specs, 교차 빌드 스크립트
5. **데몬** — `EVENT`/`DELTA` 로그는 64-bit 십진 ASCII, `time_t`/`%ld` 금지
6. **보드 배포** — `make stage` + `deploy-board.sh`, 데몬 기본 정적 링크

### 2.1 ABI 핵심 타입

| 심볼 | 폭 | 용도 |
|------|----|------|
| `y2k38_time_t` | 8바이트 signed | epoch 초 |
| `struct y2k38_timeval` | 16바이트 | 초+usec (+pad), PPC32 EABI 정렬 |
| `struct y2k38_timespec` | 16바이트 | 초+nsec |
| `struct y2k38_tm` | `struct tm` 유사 | 달력 분해 |
| soname | `liby2k38safe.so.1` | ABI major 1 |

### 2.2 커널 wrap 복구식

```
real_utc = sign_extend(kernel_time_t) + OFFSET
```

- 기본 offset 파일: `/etc/y2k38_offset`
- 환경 변수: `Y2K38_KERNEL_OFFSET_FILE`

---

## 3. 처리 과정 (구현 상세)

### 3.1 ABI 헤더 (`include/y2k38/`)

| 파일 | 내용 |
|------|------|
| `types.h` | `y2k38_time_t`, `y2k38_timeval`, `Y2K38_TIME_T32_MAX` 등 |
| `time.h` | clock, diff, format, parse, offset API |
| `eventlog.h` | `struct y2k38_event`, `EVENT` 라인 I/O |
| `y2k38.h` | 위 헤더 통합 include |

### 3.2 라이브러리 `liby2k38safe` (`lib/`)

| 파일 | 내용 |
|------|------|
| `y2k38_time.c` | `y2k38_time()`, raw/recover clock, mock, civil time 변환 |
| `y2k38_eventlog.c` | `EVENT <id> <epoch64> <msg>` append/parse |
| `y2k38_offset.c` | offset compute/load/save, `2^32` wrap, default path |

주요 API:

| API | 역할 |
|-----|------|
| `y2k38_time()` / `y2k38_gettimeofday()` | 복구된 UTC (`raw + offset`) |
| `y2k38_time_kernel_raw()` | offset 미적용 kernel 초 |
| `y2k38_difftime_sec()` | 64-bit 차분 |
| `y2k38_format_epoch()` / `y2k38_parse_epoch()` | 직렬화 |
| `y2k38_gmtime_r()` / `y2k38_timegm()` | 32-bit libc `gmtime` 비의존 |
| `y2k38_clock_compute_offset()` | `true_utc - kernel_raw` |
| `y2k38_clock_u32_wrap_offset(n)` | `n * 2^32` |
| `y2k38_clock_load/save_offset_file()` | `/etc/y2k38_offset` 영속화 |
| `y2k38_clock_apply_offset_default()` | path / env / 기본 경로 로드 |
| `y2k38_clock_set_mock()` / `set_mock_kernel()` | 테스트용 mock |

### 3.3 예제 코드 (`examples/`)

#### Broken — 문제 발생 시연

| 파일 | 시연 내용 |
|------|-----------|
| `broken/y2k38_broken.c` | `int32_t`로 ELDK 32-bit `time_t` 에뮬레이션 → 잘림·delta 실패 |
| `broken/broken_daemon_pipeline.c` | Daemon A→B 파이프라인 실패 요약 |

**전형적 실패 패턴:**

```c
/* WRONG: 32-bit time_t에 post-2038 시각 저장 */
time_t stored = (time_t)4102444800LL;   /* → -192522496 */

/* WRONG: 32-bit 차분 */
long delta = (long)(future - now);      /* → 거대 음수 */
```

#### Fixed — 라이브러리 적용

| 파일 | 시연 내용 |
|------|-----------|
| `fixed/y2k38_fixed.c` | 동일 시나리오를 `liby2k38safe`로 통과 |
| `fixed/kernel_offset_demo.c` | kernel wrap → OFFSET 파일 → UTC 복구 |

**올바른 패턴:**

```c
y2k38_time_t when = 4102444800LL;
y2k38_time_t delta = y2k38_difftime_sec(future, now);
y2k38_event_append(fp, &ev);   /* EVENT id epoch64 msg */
```

### 3.4 데몬 (`daemons/`)

#### Daemon A — 이벤트 로그

- **역할:** 이벤트 + 64-bit epoch를 로그에 append
- **포맷:** `EVENT <id> <epoch64> <msg>`
- **시드 이벤트:** `2147483648`(overflow+1s), `4102444800`(2100), now+120s

#### Daemon B — delta 계산

- **역할:** A의 로그를 읽어 `delta = y2k38_difftime_sec(when, now)` 계산
- **포맷:** `DELTA <id> <seconds> <when> <now> <msg>`
- **주기:** `period_sec` (기본 5초) 또는 `--once`

공통 옵션:

```
--offset-file PATH    # kernel offset 로드
--no-offset-file      # 자동 로드 생략
--mock-now EPOCH      # 절대 UTC mock
--mock-kernel SEC     # wrap된 kernel 초 mock
--once
```

### 3.5 보드 유틸 (`tools/`)

**`y2k38_offsetctl`** — offset 관리 CLI

```bash
y2k38_offsetctl show [--file PATH]
y2k38_offsetctl set <offset> [--file PATH]
y2k38_offsetctl set-u32-wrap <count> [--file PATH]
y2k38_offsetctl calibrate <true_utc_epoch> [--file PATH] [--mock-kernel SEC]
```

### 3.6 툴체인 통합 (`toolchain/`, `scripts/`)

| 경로 | 역할 |
|------|------|
| `toolchain/abi/y2k38.specs` | GCC `-specs=` include/lib 경로 |
| `toolchain/patches/gcc-eldk-y2k38-define.patch` | `__Y2K38_ABI__` define 스케치 (비권장) |
| `scripts/cross-build-eldk.sh` | `all` / `stage` / `tarball` / `install-sysroot` |
| `scripts/deploy-board.sh` | staging → 보드 scp/rsync |
| `scripts/board-init-y2k38.sh` | 부팅 시 offset 적용·데몬 기동 |
| `scripts/environment-setup-y2k38.sh` | `CROSS`/`SYSROOT`/`y2k38_gcc` |
| `scripts/abi_check.c` | ABI 레이아웃 검증 |

#### ELDK sysroot 설치 위치

```
$SYSROOT/usr/include/y2k38/*.h
$SYSROOT/usr/include/y2k38.h
$SYSROOT/usr/lib/liby2k38safe.a
$SYSROOT/usr/lib/liby2k38safe.so*
$SYSROOT/usr/lib/gcc-specs/y2k38.specs
```

---

## 4. 빌드·검증 과정

### 4.1 호스트 빌드 (WSL / Linux)

```bash
cd y2k38-abi
make                    # lib + 예제 + 데몬 + offsetctl
make check              # ABI layout (8/16/16 bytes)
make check-offset       # kernel wrap 복구 데모
```

개별 실행:

```bash
./examples/broken/y2k38_broken
./examples/fixed/y2k38_fixed
./examples/fixed/kernel_offset_demo

# 데몬 (pre-wrap)
./daemons/daemon_a/daemon_a /tmp/events.log \
  --mock-now 2147483600 --once --no-offset-file
./daemons/daemon_b/daemon_b /tmp/events.log /tmp/deltas.out \
  --mock-now 2147483600 --once --no-offset-file

# 데몬 (post-wrap + offset)
./tools/y2k38_offsetctl calibrate 2147483748 \
  --file /tmp/y2k38_off.conf --mock-kernel -2147483548
./daemons/daemon_a/daemon_a /tmp/ev_wrap.log \
  --offset-file /tmp/y2k38_off.conf --mock-kernel -2147483548 --once
./daemons/daemon_b/daemon_b /tmp/ev_wrap.log /tmp/d_wrap.out \
  --offset-file /tmp/y2k38_off.conf --mock-kernel -2147483548 --once
```

검증 환경: **WSL Ubuntu gcc 9.4** — `make check`, `check-offset`, daemon 연동, `make stage` 통과.

### 4.2 보드 교차 빌드 (ELDK)

```bash
export ELDK_ARCH=ppc_82xx
export ELDK_ROOT=/opt/eldk
./scripts/cross-build-eldk.sh              # PPC32 바이너리
./scripts/cross-build-eldk.sh stage        # ./staging 트리
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

보드에서 offset 교정:

```bash
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
y2k38_offsetctl show
daemon_a /var/log/y2k38_events.log --offset-file /etc/y2k38_offset &
daemon_b /var/log/y2k38_events.log /var/log/y2k38_deltas.out 5 \
  --offset-file /etc/y2k38_offset &
```

---

## 5. 결과물

### 5.1 최종 산출물 목록

| 구분 | 파일/디렉터리 | 설명 |
|------|---------------|------|
| **문서** | `Y2K38_issue.md` | 본 문서 (전체 이력) |
| | `docs/ABI_DESIGN.md` | ABI·툴체인 설계 |
| | `docs/CROSS_BUILD.md` | 교차 빌드 가이드 |
| | `docs/KERNEL_OFFSET.md` | offset 보정 가이드 |
| | `README.md` | 빠른 시작 |
| **ABI** | `include/y2k38/*.h` | 공개 헤더 |
| **라이브러리** | `lib/liby2k38safe.a`, `.so` | 정적/동적 라이브러리 |
| **예제** | `examples/broken/*` | Y2K38 실패 시연 |
| | `examples/fixed/*` | 라이브러리 적용·offset 데모 |
| **데몬** | `daemons/daemon_a/daemon_a` | 이벤트 로그 |
| | `daemons/daemon_b/daemon_b` | delta 계산 |
| **도구** | `tools/y2k38_offsetctl` | offset 관리 CLI |
| **스크립트** | `scripts/cross-build-eldk.sh` | ELDK 교차 빌드 |
| | `scripts/deploy-board.sh` | 보드 배포 |
| | `scripts/board-init-y2k38.sh` | 부팅 훅 |
| **툴체인** | `toolchain/abi/y2k38.specs` | GCC specs |
| **배포** | `staging/` (`make stage`) | 보드 배포용 트리 |

### 5.2 Broken 예제 실행 결과

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

**해석:** 32-bit `time_t`에 post-2038 시각을 넣으면 음수/잘림이 되고, Daemon B의 delta는 수백억 단위의 잘못된 값이 된다.

### 5.3 Fixed 예제 실행 결과

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

### 5.4 Daemon A / B 연동 (pre-wrap)

`--mock-now 2147483600` (overflow 48초 전):

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

### 5.5 Kernel offset 복구 (`make check-offset`)

```
true UTC          = 2147483748
kernel as int32   = -2147483548  (emulated wrap)
computed offset   = 4294967296
recovered UTC     = 2147483748 (2038-01-19T03:15:48Z)
OK: wrap recovered via OFFSET file
delta to 2100-01-01 = 1954961052 s
OK: post-wrap delta arithmetic
```

### 5.6 Daemon A / B + offset (post-wrap)

복구된 now = `2147483748` 기준:

```
DELTA FUT2038 -100 ...          # overflow 시점은 이미 100초 전 (정상)
DELTA FAR2100 1954961052 ...    # 2100까지 양수 유지
DELTA NEAR 120 ...
```

### 5.7 ABI layout (`make check`)

```
sizeof(y2k38_time_t)=8 (expect 8)
sizeof(struct y2k38_timeval)=16 (expect 16)
sizeof(struct y2k38_timespec)=16 (expect 16)
ABI CHECK OK
```

### 5.8 Staging 배포 트리 (`make stage`)

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

## 6. 커널 offset 보정 시나리오

상세: [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md)

| 시나리오 | 조치 |
|----------|------|
| **A. Pre-2038, 커널 정상** | `OFFSET 0` (또는 파일 없음). 64-bit 저장/연산만으로 미래 스케줄 보존 |
| **B. uint32 wrap 1회** | `y2k38_offsetctl set-u32-wrap 1` → `OFFSET=4294967296` |
| **C. 신뢰 UTC 교정 (권장)** | `y2k38_offsetctl calibrate <true_utc>` (GPS/NTP/호스트) |
| **D. 데몬 기동** | A/B 동일 `--offset-file` (또는 `/etc/y2k38_offset`) |

offset 파일 형식:

```
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 4294967296
```

---

## 7. 애플리케이션 준수 체크리스트

- [ ] 영속 로그·IPC에 `time_t` / `struct timeval.tv_sec` 벽시계 저장 금지
- [ ] 교차 프로세스 필드는 `y2k38_time_t` / `int64_t`
- [ ] 차분은 `y2k38_difftime_sec` (또는 동등한 int64 연산)
- [ ] 스케줄 파싱은 `y2k38_parse_epoch` / `strtoll`
- [ ] `-ly2k38safe`(또는 정적 `liby2k38safe.a`), soname major 일치
- [ ] 단위 테스트: `now = INT32_MAX - 10`, `future = INT32_MAX + 100`
- [ ] wrap 테스트: `--mock-kernel` + offset calibrate
- [ ] 보드: `/etc/y2k38_offset` 교정 후 `y2k38_offsetctl show` 확인
- [ ] Daemon A/B가 동일 offset 파일 사용

---

## 8. 한계 및 후속 작업

1. Offset은 **유저스페이스 정책** — 커널/RTC 하드웨어 버그는 별도(GPS/NTP/배터리) 영역
2. `y2k38_localtime_r`는 현재 **UTC와 동일** 취급. TZ 필요 시 플러그인 추가
3. `timer_settime` 등 절대 커널 타이머는 32-bit일 수 있음 → **상대 대기 + clamp**
4. 레거시 4바이트 time 필드 로그는 **포맷 버전 bump**로 마이그레이션
5. 실보드에서 `ppc_82xx-` ELF/`readelf`·실시계 calibrate 재확인

---

## 9. 요약

| 항목 | 내용 |
|------|------|
| **착수** | 32-bit PPC/ELDK에서 `time_t` 64-bit 확장 불가 → Application 레벨 병행 ABI |
| **생성** | `y2k38-abi` 프로젝트, Git 초기화, 디렉터리·문서·Makefile |
| **구현** | `y2k38_time_t` ABI, `liby2k38safe`, Daemon A/B, broken/fixed 예제 |
| **확장** | kernel offset 보정, `y2k38_offsetctl`, 교차 빌드·배포 스크립트 |
| **검증** | broken 실패 재현, fixed·daemon·`check-offset`·stage 정상 (WSL host) |
| **배포** | `make stage` → `deploy-board.sh` → 보드에서 `calibrate` |

**실무 한 줄:** 벽시계는 항상 `y2k38_time_t`로 다루고, 보드 시계가 wrap되면 **한곳의 OFFSET 파일**로 보정한 뒤 Daemon A/B를 기동한다.
