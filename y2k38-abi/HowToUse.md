# HowToUse — Y2K38 ABI 배치·설치·사용 가이드

ELDK 3.1.1 / 32-bit PowerPC 환경에서 **헤더**, **동적/정적 라이브러리**, **기타 ABI 파일**을 어디에 두고 어떻게 쓰는지 정리한 문서입니다.

**프로젝트:** `y2k38-abi`  
**관련 문서:** [`docs/CROSS_BUILD.md`](docs/CROSS_BUILD.md), [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md), [`docs/ABI_DESIGN.md`](docs/ABI_DESIGN.md), [`Y2K38_issue.md`](Y2K38_issue.md)  
**최초 작성:** 2026-07-09  
**추가 갱신:** 2026-07-11 — 멀티스레드·sleep·장기 실행 auto-wrap·시간 설정·예제

---

## 1. 두 가지 설치 대상

ABI 파일은 **역할에 따라 두 곳**에 나뉩니다.

| 대상 | 경로 예 | 용도 |
|------|---------|------|
| **개발 호스트 (ELDK sysroot)** | `/opt/eldk/ppc_82xx/usr/` | 크로스 **컴파일·링크** |
| **타깃 보드 (실제 리눅스)** | `/usr/`, `/etc/` | **실행** (바이너리, `.so`, 설정) |

```
┌─────────────────────────────────────────────────────────────┐
│  개발 호스트 (ELDK sysroot)                                  │
│  예: /opt/eldk/ppc_82xx/usr/                                 │
├─────────────────────────────────────────────────────────────┤
│  include/y2k38/*.h     ← 컴파일 시 #include                  │
│  lib/liby2k38safe.a    ← 정적 링크                          │
│  lib/liby2k38safe.so*  ← 동적 링크 빌드용                    │
│  lib/gcc-specs/y2k38.specs  ← GCC 자동 -I/-L (선택)         │
└─────────────────────────────────────────────────────────────┘
                          │ deploy (scp/rsync)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  타깃 보드                                                   │
├─────────────────────────────────────────────────────────────┤
│  /usr/lib/liby2k38safe.so*  ← 동적 링크 앱 런타임 필수        │
│  /usr/bin/daemon_a, daemon_b, y2k38_offsetctl                │
│  /etc/y2k38_offset         ← 커널 시계 wrap 보정 (선택)       │
└─────────────────────────────────────────────────────────────┘
```

**요약**

- **헤더** → sysroot에만 두면 됨 (보드에서 컴파일하지 않는 한 보드 불필요)
- **동적 `.so`** → sysroot(빌드용) + **보드 `/usr/lib`**(실행용)
- **정적 `.a`** → sysroot만 (보드에는 실행파일에 이미 포함)

---

## 2. 헤더 파일

### 2.1 설치 위치 (sysroot)

| 소스 (프로젝트) | 설치 경로 (sysroot) | 역할 |
|-----------------|---------------------|------|
| `include/y2k38/types.h` | `$SYSROOT/usr/include/y2k38/types.h` | `y2k38_time_t`, 구조체 |
| `include/y2k38/time.h` | `$SYSROOT/usr/include/y2k38/time.h` | clock, diff, offset API |
| `include/y2k38/eventlog.h` | `$SYSROOT/usr/include/y2k38/eventlog.h` | `EVENT` 로그 I/O |
| `include/y2k38.h` | `$SYSROOT/usr/include/y2k38.h` | 통합 include |

`$SYSROOT` 예: `/opt/eldk/ppc_82xx`

### 2.2 설치 후 디렉터리 구조

```
/opt/eldk/ppc_82xx/usr/include/
├── y2k38.h                    # #include <y2k38.h>
└── y2k38/
    ├── types.h                # #include <y2k38/types.h>
    ├── time.h                 # #include <y2k38/time.h>
    └── eventlog.h             # #include <y2k38/eventlog.h>
```

`y2k38/` 서브디렉터리를 쓰는 이유:

- 시스템 `<time.h>`와 이름 충돌 방지
- `#include <y2k38/time.h>`로 Y2K38 ABI임을 명확히 구분

### 2.3 소스에서 include

```c
/* 방법 1: 개별 헤더 */
#include <y2k38/types.h>
#include <y2k38/time.h>

/* 방법 2: 통합 헤더 */
#include <y2k38.h>
```

### 2.4 컴파일러가 헤더를 찾게 하는 방법

**방법 A — `-I` 플래그**

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -I$SYSROOT/usr/include \
  myapp.c -o myapp
```

**방법 B — `-isystem` (권장, specs/환경 스크립트에서 사용)**

```bash
-isystem $SYSROOT/usr/include
```

**방법 C — 환경 변수 `CPATH`**

```bash
export CPATH=$SYSROOT/usr/include:$CPATH
```

`scripts/environment-setup-y2k38.sh`가 `CPATH`를 설정합니다.

### 2.5 보드에 헤더가 필요한가?

| 상황 | 보드에 헤더 필요? |
|------|-------------------|
| ELDK 교차 빌드만 사용 | **불필요** |
| 보드에서 네이티브 gcc로 빌드 | 필요 (`/usr/include/y2k38/`) |

---

## 3. 라이브러리 (정적 / 동적)

### 3.1 설치 위치 (sysroot)

```
$SYSROOT/usr/lib/
├── liby2k38safe.a              # 정적 라이브러리
├── liby2k38safe.so.1.0.0       # 공유 라이브러리 본체
├── liby2k38safe.so.1  →  liby2k38safe.so.1.0.0   # soname
└── liby2k38safe.so    →  liby2k38safe.so.1       # 링커용
```

`make install` / `make install-sysroot`가 위 구조로 복사·심볼릭 링크를 만듭니다.

### 3.2 soname 3단계 구조

| 파일 | 용도 |
|------|------|
| `liby2k38safe.so.1.0.0` | 실제 바이너리 (버전 1.0.0) |
| `liby2k38safe.so.1` | **soname** — ABI major. 런타임 로더가 이 이름으로 로드 |
| `liby2k38safe.so` | **링크 시** `-ly2k38safe`가 가리키는 이름 |

ABI major가 바뀌면 soname을 `liby2k38safe.so.2`로 올립니다.

### 3.3 정적 vs 동적 링크

| | 정적 (`liby2k38safe.a`) | 동적 (`-ly2k38safe`) |
|--|-------------------------|----------------------|
| 링크 | `... liby2k38safe.a` | `-L.../lib -ly2k38safe` |
| 보드에 `.so` 필요? | **아니오** | **예** |
| 이 프로젝트 기본 | `LINK_STATIC=1` (데몬/툴) | 별도 앱이 동적 링크 시 |
| 장점 | 배포 단순, `LD_LIBRARY_PATH` 불필요 | `.so`만 갱신해 여러 앱 업데이트 |

**정적 링크 예**

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -I$SYSROOT/usr/include \
  myapp.c $SYSROOT/usr/lib/liby2k38safe.a \
  -o myapp
```

**동적 링크 예**

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -I$SYSROOT/usr/include \
  -L$SYSROOT/usr/lib \
  myapp.c -ly2k38safe \
  -Wl,-rpath,/usr/lib \
  -o myapp
```

`-Wl,-rpath,/usr/lib` — 보드에서 `/usr/lib`의 `.so`를 찾게 함.

### 3.4 보드에 동적 라이브러리 넣기

동적 링크 앱을 보드에서 실행하려면:

```bash
# 보드에 복사
/usr/lib/liby2k38safe.so.1.0.0
/usr/lib/liby2k38safe.so.1  →  liby2k38safe.so.1.0.0
# (선택) /usr/lib/liby2k38safe.so → liby2k38safe.so.1
```

설치 후 확인:

```bash
ldconfig                    # 지원 시
ldd /usr/bin/myapp          # liby2k38safe.so.1 => /usr/lib/...
```

---

## 4. 기타 ABI 관련 파일

### 4.1 GCC specs (개발 호스트 전용)

| 항목 | 경로 |
|------|------|
| 소스 | `toolchain/abi/y2k38.specs` |
| 설치 | `$SYSROOT/usr/lib/gcc-specs/y2k38.specs` |

내용 (경로는 실제 SYSROOT에 맞게 수정):

```
*cpp_unique_options:
+ -isystem /opt/eldk/ppc_82xx/usr/include

*link:
+ -L/opt/eldk/ppc_82xx/usr/lib
```

사용:

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -specs=$SYSROOT/usr/lib/gcc-specs/y2k38.specs \
  myapp.c -ly2k38safe -o myapp
```

역할: 매 빌드마다 `-I`/`-L`을 수동으로 넣지 않아도 됨. **보드에는 불필요.**

### 4.2 환경 스크립트 (개발 호스트)

| 파일 | 역할 |
|------|------|
| `scripts/environment-setup-y2k38.sh` | `CROSS`, `SYSROOT`, `CPATH`, `LIBRARY_PATH`, `y2k38_gcc` |

```bash
. scripts/environment-setup-y2k38.sh
y2k38_gcc myapp.c -ly2k38safe -o myapp
```

### 4.3 실행 바이너리 (보드 `/usr/bin`)

| 빌드물 | 보드 경로 | 용도 |
|--------|-----------|------|
| `daemons/daemon_a/daemon_a` | `/usr/bin/daemon_a` | 이벤트 로그 (`--auto-wrap` 지원) |
| `daemons/daemon_b/daemon_b` | `/usr/bin/daemon_b` | delta 계산 |
| `tools/y2k38_offsetctl` | `/usr/bin/y2k38_offsetctl` | offset 관리 |

정적 링크(`LINK_STATIC=1`)면 **`.so` 없이** 바이너리만 복사해도 동작합니다.

### 4.4 런타임 설정 (보드 `/etc`)

| 파일 | 경로 | 용도 |
|------|------|------|
| offset 설정 | `/etc/y2k38_offset` | 커널 `time_t` wrap 보정 |

형식:

```
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 0
```

wrap 발생 후 교정:

```bash
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
y2k38_offsetctl show
```

환경 변수로 경로 변경:

```bash
export Y2K38_KERNEL_OFFSET_FILE=/custom/path/offset
```

데몬 기동 시:

```bash
daemon_a /var/log/events.log --offset-file /etc/y2k38_offset --auto-wrap
daemon_b /var/log/events.log /var/log/deltas.out 5 --offset-file /etc/y2k38_offset
```

### 4.5 부팅 스크립트 (보드, 선택)

| 소스 | 권장 보드 경로 |
|------|----------------|
| `scripts/board-init-y2k38.sh` | `/usr/share/y2k38/board-init-y2k38.sh` |

`/etc/rc.local` 등에서 source:

```bash
export Y2K38_START_DAEMONS=1
. /usr/share/y2k38/board-init-y2k38.sh
```

---

## 5. 자동 설치 (권장)

### 5.1 사전 준비

```bash
export ELDK_ARCH=ppc_82xx          # 보드에 맞게: ppc_8xx, ppc_4xx
export ELDK_ROOT=/opt/eldk
export CROSS=${ELDK_ARCH}-
export SYSROOT=${ELDK_ROOT}/${ELDK_ARCH}
export PATH=${ELDK_ROOT}/usr/bin:$PATH

# 컴파일러 확인
which ${CROSS}gcc
```

### 5.2 sysroot에 ABI 설치 (개발 호스트)

**중요:** `toolchain/abi/y2k38.specs` 안의 경로를 실제 `SYSROOT`에 맞게 수정한 뒤 실행합니다.

```bash
cd /path/to/y2k38-abi
make install-sysroot CROSS=$CROSS SYSROOT=$SYSROOT
```

또는:

```bash
./scripts/cross-build-eldk.sh install-sysroot
```

설치되는 항목:

```
$SYSROOT/usr/include/y2k38/*.h
$SYSROOT/usr/include/y2k38.h
$SYSROOT/usr/lib/liby2k38safe.a
$SYSROOT/usr/lib/liby2k38safe.so*
$SYSROOT/usr/lib/gcc-specs/y2k38.specs
$SYSROOT/usr/bin/daemon_a, daemon_b, y2k38_offsetctl
```

### 5.3 보드 배포용 staging

```bash
./scripts/cross-build-eldk.sh stage
```

생성 트리:

```
staging/
├── etc/y2k38_offset
└── usr/
    ├── bin/
    │   ├── daemon_a
    │   ├── daemon_b
    │   ├── y2k38_offsetctl
    │   ├── y2k38_fixed
    │   ├── kernel_offset_demo
    │   ├── y2k38_threads_demo
    │   └── y2k38_autowrap_demo
    ├── include/y2k38/...
    ├── lib/
    │   ├── liby2k38safe.a
    │   └── liby2k38safe.so*
    └── share/y2k38/
        └── board-init-y2k38.sh
```

보드로 전송:

```bash
./scripts/deploy-board.sh root@192.168.0.10:/
```

---

## 6. 수동 설치

Makefile 없이 직접 복사할 때:

### 6.1 sysroot — 헤더

```bash
SYSROOT=/opt/eldk/ppc_82xx
mkdir -p $SYSROOT/usr/include/y2k38
cp include/y2k38/*.h $SYSROOT/usr/include/y2k38/
cp include/y2k38.h $SYSROOT/usr/include/
```

### 6.2 sysroot — 라이브러리

```bash
# 먼저 교차 빌드
make cross CROSS=ppc_82xx- SYSROOT=$SYSROOT

cd $SYSROOT/usr/lib
cp /path/to/y2k38-abi/lib/liby2k38safe.a .
cp /path/to/y2k38-abi/lib/liby2k38safe.so liby2k38safe.so.1.0.0
ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1
ln -sf liby2k38safe.so.1 liby2k38safe.so
```

### 6.3 sysroot — GCC specs

```bash
mkdir -p $SYSROOT/usr/lib/gcc-specs
# 경로 수정 후
cp toolchain/abi/y2k38.specs $SYSROOT/usr/lib/gcc-specs/
```

### 6.4 보드 — 동적 라이브러리만

```bash
scp lib/liby2k38safe.so board:/usr/lib/liby2k38safe.so.1.0.0
ssh board 'cd /usr/lib && ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1'
```

### 6.5 보드 — 바이너리 (정적 링크)

```bash
scp daemons/daemon_a/daemon_a board:/usr/bin/
scp daemons/daemon_b/daemon_b board:/usr/bin/
scp tools/y2k38_offsetctl board:/usr/bin/
chmod +x board:/usr/bin/daemon_a board:/usr/bin/daemon_b board:/usr/bin/y2k38_offsetctl
```

---

## 7. 애플리케이션 작성·빌드 예

### 7.1 소스 예

```c
#include <y2k38/time.h>
#include <y2k38/types.h>

int main(void)
{
    y2k38_time_t now = y2k38_time(NULL);
    y2k38_time_t future = 4102444800LL;   /* 2100-01-01 UTC */
    y2k38_time_t delta = y2k38_difftime_sec(future, now);
    return 0;
}
```

**금지:** 영속 로그·IPC·데몬 경계에 `time_t` / `%ld` 벽시계 저장.

### 7.2 빌드 (환경 스크립트 사용)

```bash
. scripts/environment-setup-y2k38.sh
y2k38_gcc myapp.c -ly2k38safe -o myapp
```

### 7.3 빌드 (플래그 직접 지정)

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -specs=$SYSROOT/usr/lib/gcc-specs/y2k38.specs \
  -I$SYSROOT/usr/include \
  -L$SYSROOT/usr/lib \
  myapp.c -ly2k38safe -Wl,-rpath,/usr/lib \
  -o myapp
```

---

## 8. 검증

### 8.1 sysroot 파일 존재

```bash
ls -la $SYSROOT/usr/include/y2k38/
ls -la $SYSROOT/usr/lib/liby2k38safe*
cat $SYSROOT/usr/lib/gcc-specs/y2k38.specs
```

### 8.2 ABI 레이아웃 (타깃으로 컴파일)

```bash
${CROSS}gcc --sysroot=$SYSROOT \
  -I$SYSROOT/usr/include \
  scripts/abi_check.c \
  -L$SYSROOT/usr/lib -ly2k38safe \
  -o /tmp/abi_check_ppc
```

기대 출력:

```
sizeof(y2k38_time_t)=8 (expect 8)
sizeof(struct y2k38_timeval)=16 (expect 16)
sizeof(struct y2k38_timespec)=16 (expect 16)
ABI CHECK OK
```

### 8.3 ELF 아키텍처

```bash
${CROSS}readelf -h daemons/daemon_a/daemon_a
# Class: ELF32, Machine: PowerPC
```

### 8.4 보드 동적 링크

```bash
ldd /usr/bin/myapp | grep y2k38
# liby2k38safe.so.1 => /usr/lib/liby2k38safe.so.1
```

---

## 9. 체크리스트

### 개발 호스트 (sysroot)

- [ ] `$SYSROOT/usr/include/y2k38/types.h`
- [ ] `$SYSROOT/usr/include/y2k38/time.h`
- [ ] `$SYSROOT/usr/include/y2k38/eventlog.h`
- [ ] `$SYSROOT/usr/include/y2k38.h`
- [ ] `$SYSROOT/usr/lib/liby2k38safe.a`
- [ ] `$SYSROOT/usr/lib/liby2k38safe.so.1.0.0` (+ `.so.1`, `.so` 링크)
- [ ] (선택) `$SYSROOT/usr/lib/gcc-specs/y2k38.specs`

### 타깃 보드

**정적 링크 앱만 사용 시**

- [ ] `/usr/bin/daemon_a`, `daemon_b`, `y2k38_offsetctl`
- [ ] (선택) `/etc/y2k38_offset`

**동적 링크 앱 사용 시 추가**

- [ ] `/usr/lib/liby2k38safe.so.1.0.0`
- [ ] `/usr/lib/liby2k38safe.so.1` → 위 파일
- [ ] `ldd`로 의존성 확인

---

## 10. 자주 하는 실수

| 증상 | 원인 | 해결 |
|------|------|------|
| `y2k38/time.h: No such file` | sysroot 미설치 또는 `-I` 누락 | `make install-sysroot` 후 `-I$SYSROOT/usr/include` |
| `undefined reference to y2k38_time` | `-ly2k38safe` 누락 | 링크 줄 끝에 `-ly2k38safe` 또는 `.a` 직접 지정 |
| 보드 `liby2k38safe.so.1: not found` | 동적 링크인데 `.so` 미배포 | `/usr/lib`에 `.so*` 복사 또는 정적 링크 |
| specs 적용 안 됨 | specs 내 경로가 다른 아키텍처 | `y2k38.specs` 경로를 실제 SYSROOT로 수정 |
| `sizeof(y2k38_time_t)!=8` | 호스트 gcc로 abi_check 빌드 | **반드시** `${CROSS}gcc`로 빌드 |

---

## 11. 위치 요약표

| 종류 | 개발 호스트 (sysroot) | 타깃 보드 |
|------|------------------------|-----------|
| **헤더** | `$SYSROOT/usr/include/y2k38/` | 보통 불필요 |
| **정적 lib** | `$SYSROOT/usr/lib/liby2k38safe.a` | 불필요 (실행파일에 포함) |
| **동적 lib** | `$SYSROOT/usr/lib/liby2k38safe.so*` | **`/usr/lib/liby2k38safe.so*`** |
| **GCC specs** | `$SYSROOT/usr/lib/gcc-specs/y2k38.specs` | 불필요 |
| **바이너리** | `$SYSROOT/usr/bin/` (선택) | **`/usr/bin/`** |
| **offset 설정** | — | **`/etc/y2k38_offset`** |
| **부팅 스크립트** | — | `/usr/share/y2k38/board-init-y2k38.sh` (선택) |

---

## 12. 최소 명령 요약

```bash
# 1. 환경
export ELDK_ARCH=ppc_82xx ELDK_ROOT=/opt/eldk
export CROSS=${ELDK_ARCH}- SYSROOT=${ELDK_ROOT}/${ELDK_ARCH}
export PATH=${ELDK_ROOT}/usr/bin:$PATH

# 2. sysroot 설치 (specs 경로 수정 후)
cd /path/to/y2k38-abi
make install-sysroot CROSS=$CROSS SYSROOT=$SYSROOT

# 3. 앱 빌드
. scripts/environment-setup-y2k38.sh
y2k38_gcc myapp.c -ly2k38safe -o myapp

# 4. 보드 배포
./scripts/cross-build-eldk.sh stage
./scripts/deploy-board.sh root@BOARD:/

# 5. 보드에서 offset 교정 (wrap 시)
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
```

---

**한 줄 요약:** 헤더·`.a`·`.so`(빌드용)는 **ELDK sysroot `$SYSROOT/usr/`** 에 두고, 보드에는 **동적 링크 시 `/usr/lib/liby2k38safe.so*`** 와 **실행 바이너리·`/etc/y2k38_offset`** 만 맞추면 됩니다.

---

# Part B — 런타임·동시성·장기 실행 (추가 가이드)

아래는 `HowToUse.md` 최초 작성 이후 추가된 API·예제·운영 패턴입니다.

## 13. 추가 API (`include/y2k38/time.h`)

| API | 역할 |
|-----|------|
| `y2k38_nsleep_relative(sec, nsec)` | 상대 `nanosleep` — Y2K38 안전 |
| `y2k38_sleep_until(wake_utc)` | 절대 시각까지 상대 sleep 루프로 대기 |
| `y2k38_sleep_set_max_chunk(sec)` | `sleep_until` 1회 chunk 상한 (기본 3600초) |
| `y2k38_clock_set_auto_wrap(on, path)` | 장기 실행 데몬: kernel wrap 자동 감지 |
| `y2k38_clock_poll_wrap()` | wrap 수동 재검사 (watchdog 스레드용) |
| `y2k38_clock_on_wrap_callback(fn, user)` | wrap 발생 시 콜백 |
| `y2k38_clock_get_wrap_count()` | 감지된 wrap 횟수 |

---

## 14. 여러 데몬·스레드가 동시에 시간을 쓸 때

### 14.1 원칙

| 항목 | 처리 |
|------|------|
| 저장·연산 | 모두 `y2k38_time_t` (64-bit). `time_t` 공유 금지 |
| offset | 전 프로세스가 **같은** `/etc/y2k38_offset` 사용 |
| offset 변경 | `y2k38_offsetctl` 또는 calibrate **한 곳**에서만 |
| `y2k38_time()` | Linux `gettimeofday` 기반 — 스레드 안전 |
| 프로세스 간 | `EVENT ... epoch64 ...` 로그 포맷 통일 |

### 14.2 주의

- 실행 중 `y2k38_clock_set_kernel_offset()`를 여러 스레드가 동시에 바꾸지 말 것.
- 기동 시 offset 로드 → 평소 읽기 전용. 변경은 calibrate 후 파일 갱신 + (필요 시) `y2k38_clock_load_offset_file()`.
- 여러 데몬이 동시에 wrap을 감지하면 OFFSET 파일이 경합할 수 있음 → **한 데몬이 wrap 담당**하거나 wrap 후 auto-wrap 끄기 권장.

### 14.3 3 스레드 예제

| 파일 | 스레드 역할 |
|------|-------------|
| `examples/fixed/y2k38_threads_demo.c` | ① logger: `y2k38_time()` ② scheduler: post-2038 delta ③ sleeper: `y2k38_sleep_until()` |

```bash
make examples/fixed/y2k38_threads_demo
./examples/fixed/y2k38_threads_demo --mock-now 2147483637 --wake-abs 2147483667 --ticks 35
```

---

## 15. `sleep` / `nanosleep` / `usleep`

### 15.1 안전 (상대 대기)

```c
sleep(5);
usleep(500000);
y2k38_nsleep_relative(5, 0);
```

32-bit `time_t`와 무관 — **몇 초 자다**는 의미만 전달.

### 15.2 위험 (절대 시각·32-bit)

| API | 이유 |
|-----|------|
| `timer_settime(..., TIMER_ABSTIME, &ts)` | `ts.tv_sec`가 32-bit |
| `pthread_cond_timedwait` + 32-bit `timespec` | 2038 이후 만료 시각 깨짐 |
| `select`/`poll` timeout을 `time_t`로 계산 | overflow |

### 15.3 지금은 2038 전, 깨울 때는 2038 후

```c
y2k38_time_t wake = Y2K38_OVERFLOW_EPOCH_SEC + 100;  /* int64로 보관 */
y2k38_sleep_until(wake);   /* 내부: y2k38_time() + 상대 nanosleep 루프 */
```

**금지:** `time_t wake = (time_t)wake_utc;` 후 절대 타이머 사용.

---

## 16. 장기 실행 데몬 (2038 전 시작 → 2038 후에도 계속)

### 16.1 문제

데몬이 재시작 없이 2038을 넘으면 커널 `time_t`가 wrap → `y2k38_time()`의 “지금”이 갑자기 과거로 튐.  
(미래 스케줄 `y2k38_time_t` 저장값 자체는 이미 안전.)

### 16.2 자동 wrap 복구

```c
real_utc = kernel_raw + OFFSET
/* wrap 시: OFFSET += 4294967296 (2^32) */
```

기동 코드:

```c
y2k38_clock_apply_offset_default("/etc/y2k38_offset");
y2k38_clock_set_auto_wrap(1, "/etc/y2k38_offset");
y2k38_clock_on_wrap_callback(on_wrap_log, NULL);

for (;;) {
    y2k38_time_t now = y2k38_time(NULL);  /* 호출마다 wrap 검사 */
    /* ... */
    y2k38_nsleep_relative(5, 0);
}
```

데몬 CLI:

```bash
daemon_a /var/log/events.log --offset-file /etc/y2k38_offset --auto-wrap
```

### 16.3 예제

```bash
make examples/fixed/y2k38_autowrap_demo
./examples/fixed/y2k38_autowrap_demo
# WRAP detected → offset=4294967296 → UTC 연속 유지
```

### 16.4 wrap 후 auto-wrap 끄기 (성능·중복 방지)

```c
static void on_wrap(y2k38_time_t off, unsigned n, void *u) {
    (void)off; (void)u;
    if (n >= 1)
        y2k38_clock_set_auto_wrap(0, NULL);
}
```

### 16.5 watchdog (호출 빈도가 낮을 때)

```c
/* 30초마다 */
y2k38_clock_poll_wrap();
```

---

## 17. wrap 검사 성능

### 17.1 결론

**일반적으로 무시할 수준.** 비용 대부분은 `gettimeofday()`(또는 vdso)와 mutex.

| 경로 | 빈도 | 추가 비용 |
|------|------|-----------|
| `auto_wrap` 꺼짐 | 매 호출 | 플래그 확인만 |
| 정상 (시계 전진) | 매 호출 | 정수 비교 수 ns |
| wrap 감지 + 파일 저장 | **~68년 1회** | ms급 (한 번) |

### 17.2 부담 줄이기

1. wrap 1회 후 `y2k38_clock_set_auto_wrap(0, NULL)`
2. auto-wrap 끄고 `y2k38_clock_poll_wrap()`만 주기 호출
3. 로그용 `y2k38_time()` 호출 빈도 제한 (예: 1초 1회 캐시)

---

## 18. 시간 설정 방법

### 18.1 두 층

| 층 | 설명 |
|----|------|
| **A. 커널 벽시계** (`time_t` 32-bit) | 2038 이후 직접 설정 사실상 불가 |
| **B. y2k38 UTC 뷰** | `OFFSET`으로 `y2k38_time()` 결과 조정 — **권장** |

### 18.2 OFFSET으로 UTC 맞추기 (2038 이후에도 OK)

```bash
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
y2k38_offsetctl set <offset> --file /etc/y2k38_offset
y2k38_offsetctl set-u32-wrap 1 --file /etc/y2k38_offset
```

코드 (GPS/NTP):

```c
y2k38_time_t true_utc = gps_read_epoch();
y2k38_time_t raw      = y2k38_time_kernel_raw(NULL);
y2k38_time_t off      = y2k38_clock_compute_offset(true_utc, raw);
y2k38_clock_set_kernel_offset(off);
y2k38_clock_save_offset_file("/etc/y2k38_offset", off);
```

### 18.3 커널 `settimeofday` (제한)

```c
settimeofday(&tv, NULL);  /* tv.tv_sec는 32-bit — 2038 이후 설정 불가 */
```

2038 이후 벽시계는 **OFFSET 교정**으로만 맞춥니다.

### 18.4 테스트 mock (보드 시계 변경 아님)

```c
y2k38_clock_set_mock(1, epoch);
y2k38_clock_set_mock_kernel(1, kernel_sec32);
```

### 18.5 미래 이벤트 “시각” 설정 (스케줄)

```c
ev.when = 4102444800LL;  /* 2100-01-01 — y2k38_time_t */
y2k38_event_append(fp, &ev);
```

---

## 19. 추가 예제·빌드

| 예제 | 빌드 | 설명 |
|------|------|------|
| `y2k38_threads_demo` | `make examples/fixed/y2k38_threads_demo` | 3 스레드 동시 시간 API |
| `y2k38_autowrap_demo` | `make examples/fixed/y2k38_autowrap_demo` | wrap 자동 OFFSET 보정 |
| `kernel_offset_demo` | `make examples/fixed/kernel_offset_demo` | OFFSET 파일 수동 복구 |
| `y2k38_fixed` | `make examples/fixed/y2k38_fixed` | broken 시나리오 해결 |
| `y2k38_broken` | `make examples/broken/y2k38_broken` | 32-bit 실패 시연 |

검증:

```bash
make check
make check-offset
```

---

## 20. 장기 실행 데몬 체크리스트

- [ ] `time()` / `localtime()` 직접 호출 제거 → `y2k38_time()`만
- [ ] 로그·스케줄: `y2k38_time_t` / `EVENT id epoch64 msg`
- [ ] 기동: `y2k38_clock_apply_offset_default()`
- [ ] **`y2k38_clock_set_auto_wrap(1, "/etc/y2k38_offset")`** (2038 통과 예상 시)
- [ ] wrap 콜백에서 auto-wrap 끄기 또는 watchdog `poll_wrap`
- [ ] 대기: `y2k38_nsleep_relative` / `y2k38_sleep_until`
- [ ] (선택) NTP/GPS 주기 calibrate
- [ ] 여러 데몬: 동일 `/etc/y2k38_offset`

---

## 21. 추가 결과물 요약 (2026-07-11)

| 구분 | 추가·변경 |
|------|-----------|
| **라이브러리** | `y2k38_sleep_until`, `y2k38_nsleep_relative`, `y2k38_clock_set_auto_wrap`, `poll_wrap`, wrap callback |
| **데몬** | `daemon_a --auto-wrap` |
| **예제** | `y2k38_threads_demo.c`, `y2k38_autowrap_demo.c` |
| **문서** | 본 Part B, `Y2K38_issue.md` 장기 실행·교차 빌드 절 보강 |

**장기 실행 한 줄:** `y2k38_clock_set_auto_wrap(1, "/etc/y2k38_offset")` 켜고, 시간은 전부 `y2k38_*`만 쓰면 재시작 없이 2038을 넘어도 UTC가 연속됩니다.
