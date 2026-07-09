# Y2K38 ABI 파일 배치 가이드

헤더, 동적/정적 라이브러리, GCC specs, 런타임 설정 파일을
**어디에 두고**, **어떻게 쓰는지**를 정리한다.

대상: ELDK 3.1.1 / 32-bit PowerPC (`ppc_82xx` 등)

---

## 0. 두 장소만 기억하면 된다

ABI 관련 파일은 **역할이 다른 두 곳**에 나뉜다.

| 장소 | 경로 예 | 언제 쓰나 |
|------|---------|-----------|
| **① 개발 호스트 (ELDK sysroot)** | `/opt/eldk/ppc_82xx/usr/` | 크로스 **컴파일·링크** |
| **② 타깃 보드 (실제 루트FS)** | `/usr/`, `/etc/` | **실행** |

```
[개발 PC]  헤더 + .a + .so + specs  →  빌드에 사용
                │
                │  scp / rsync / make stage
                ▼
[보드]     .so + 바이너리 + /etc/y2k38_offset  →  실행에 사용
```

- **헤더**는 보통 보드에 안 넣는다. (보드에서 컴파일하지 않으므로)
- **동적 `.so`** 는 동적 링크한 앱을 보드에서 돌릴 때 **반드시** `/usr/lib`에 둔다.
- **정적 링크**한 데몬만 쓰면 보드에 `.so`가 없어도 된다.

---

## 1. 헤더 파일

### 1.1 어디에 넣는가 (개발 호스트 sysroot)

| 프로젝트 소스 | 설치 경로 |
|---------------|-----------|
| `include/y2k38/types.h` | `$SYSROOT/usr/include/y2k38/types.h` |
| `include/y2k38/time.h` | `$SYSROOT/usr/include/y2k38/time.h` |
| `include/y2k38/eventlog.h` | `$SYSROOT/usr/include/y2k38/eventlog.h` |
| `include/y2k38.h` | `$SYSROOT/usr/include/y2k38.h` |

`$SYSROOT` 예: `/opt/eldk/ppc_82xx`

설치 후 트리:

```
/opt/eldk/ppc_82xx/usr/include/
├── y2k38.h                 ← #include <y2k38.h>
└── y2k38/
    ├── types.h             ← #include <y2k38/types.h>
    ├── time.h              ← #include <y2k38/time.h>
    └── eventlog.h          ← #include <y2k38/eventlog.h>
```

`y2k38/` 서브디렉터리를 쓰는 이유:

1. 시스템 `<time.h>`와 이름 충돌 방지
2. `#include <y2k38/...>` 로 “Y2K38 ABI”임이 코드에서 바로 보임

### 1.2 수동으로 넣는 방법

```bash
SYSROOT=/opt/eldk/ppc_82xx

mkdir -p $SYSROOT/usr/include/y2k38
cp include/y2k38/*.h $SYSROOT/usr/include/y2k38/
cp include/y2k38.h   $SYSROOT/usr/include/
```

### 1.3 자동으로 넣는 방법 (권장)

```bash
export CROSS=ppc_82xx-
export SYSROOT=/opt/eldk/ppc_82xx
make install-sysroot CROSS=$CROSS SYSROOT=$SYSROOT
```

`install-sysroot`가 헤더 + lib + specs + bin을 한꺼번에 넣는다.

### 1.4 컴파일러가 헤더를 찾게 하는 방법

소스:

```c
#include <y2k38/time.h>
#include <y2k38/types.h>
/* 또는 */
#include <y2k38.h>
```

컴파일 플래그 (셋 중 하나):

```bash
# A) -I
ppc_82xx-gcc --sysroot=$SYSROOT -I$SYSROOT/usr/include ...

# B) -isystem (권장: 시스템 헤더처럼 취급)
ppc_82xx-gcc --sysroot=$SYSROOT -isystem $SYSROOT/usr/include ...

# C) 환경 변수
export CPATH=$SYSROOT/usr/include:$CPATH
```

`scripts/environment-setup-y2k38.sh`를 source 하면 `CPATH`와 `y2k38_gcc` 래퍼가 설정된다.

### 1.5 보드에는?

| 상황 | 보드 `/usr/include/y2k38/` 필요? |
|------|----------------------------------|
| 호스트에서만 교차 빌드 (일반) | **불필요** |
| 보드에서 네이티브 gcc로 빌드 | **필요** |

---

## 2. 동적 라이브러리 (`.so`)

### 2.1 어디에 넣는가

#### (A) 개발 호스트 — 링크용

```
$SYSROOT/usr/lib/
├── liby2k38safe.so.1.0.0     ← 실제 공유 라이브러리 본체
├── liby2k38safe.so.1  →  .so.1.0.0   ← soname (런타임이 찾는 이름)
└── liby2k38safe.so    →  .so.1       ← -ly2k38safe 가 찾는 이름
```

#### (B) 타깃 보드 — 실행용 (동적 링크 앱일 때 필수)

```
/usr/lib/
├── liby2k38safe.so.1.0.0
└── liby2k38safe.so.1  →  liby2k38safe.so.1.0.0
```

보드에는 `.so` (링커용 링크)까지는 없어도 된다.  
런타임이 찾는 것은 **soname** `liby2k38safe.so.1` 이다.

### 2.2 soname 3단 구조가 필요한 이유

| 파일 | 누가 쓰나 | 의미 |
|------|-----------|------|
| `liby2k38safe.so.1.0.0` | 디스크上的 실제 파일 | 패치 버전 (1.0.0) |
| `liby2k38safe.so.1` | 실행 시 동적 링커 | ABI major 1 |
| `liby2k38safe.so` | 빌드 시 `-ly2k38safe` | 개발용 별칭 |

ABI major가 바뀌면 soname을 `.so.2`로 올리고, 옛 앱은 `.so.1`을 계속 쓴다.

### 2.3 수동 설치 (sysroot)

```bash
SYSROOT=/opt/eldk/ppc_82xx
# 먼저 교차 빌드
make cross CROSS=ppc_82xx- SYSROOT=$SYSROOT

cp lib/liby2k38safe.so $SYSROOT/usr/lib/liby2k38safe.so.1.0.0
cd $SYSROOT/usr/lib
ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1
ln -sf liby2k38safe.so.1     liby2k38safe.so
```

### 2.4 보드에 동적 lib 배포

```bash
scp lib/liby2k38safe.so root@BOARD:/usr/lib/liby2k38safe.so.1.0.0
ssh root@BOARD 'cd /usr/lib && \
  ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1 && \
  (command -v ldconfig >/dev/null && ldconfig || true)'
```

확인:

```bash
# 보드에서
ls -la /usr/lib/liby2k38safe*
ldd /usr/bin/myapp | grep y2k38
# liby2k38safe.so.1 => /usr/lib/liby2k38safe.so.1
```

### 2.5 동적 링크 빌드 예

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -isystem $SYSROOT/usr/include \
  -L$SYSROOT/usr/lib \
  myapp.c -ly2k38safe \
  -Wl,-rpath,/usr/lib \
  -o myapp
```

`-Wl,-rpath,/usr/lib` : 보드에서 `/usr/lib`를 기본 검색 경로로 박아 둔다.

### 2.6 정적 라이브러리 (`.a`) — 같이 두는 위치

```
$SYSROOT/usr/lib/liby2k38safe.a
```

| | 정적 `.a` | 동적 `.so` |
|--|-----------|------------|
| 보드에 lib 파일 필요? | **아니오** (실행파일에 포함) | **예** |
| 이 프로젝트 데몬 기본 | `LINK_STATIC=1` | — |
| 배포 | 바이너리만 복사 | 바이너리 + `.so.1*` |

정적 링크 예:

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -isystem $SYSROOT/usr/include \
  myapp.c $SYSROOT/usr/lib/liby2k38safe.a \
  -o myapp
```

---

## 3. 기타 ABI 관련 파일

### 3.1 GCC specs (호스트 전용)

| 항목 | 경로 |
|------|------|
| 소스 | `toolchain/abi/y2k38.specs` |
| 설치 | `$SYSROOT/usr/lib/gcc-specs/y2k38.specs` |

역할: `-isystem`(헤더), `-L`(lib) 경로를 GCC에 자동 주입.

```
*cpp_unique_options:
+ -isystem /opt/eldk/ppc_82xx/usr/include

*link:
+ -L/opt/eldk/ppc_82xx/usr/lib
```

**주의:** install 전에 specs 안의 절대 경로를 실제 `SYSROOT`에 맞게 고친다.
(`ppc_8xx`면 `/opt/eldk/ppc_8xx/...`)

사용:

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -specs=$SYSROOT/usr/lib/gcc-specs/y2k38.specs \
  myapp.c -ly2k38safe -o myapp
```

보드에는 **넣지 않는다**.

### 3.2 환경 스크립트 (호스트)

| 파일 | 하는 일 |
|------|---------|
| `scripts/environment-setup-y2k38.sh` | `CROSS`, `SYSROOT`, `CPATH`, `LIBRARY_PATH`, `y2k38_gcc` |

```bash
. scripts/environment-setup-y2k38.sh
y2k38_gcc myapp.c -ly2k38safe -o myapp
```

고정 설치 경로는 없다. 프로젝트 또는 `/opt/eldk` 아래에 두고 source 한다.

### 3.3 실행 바이너리 (보드 `/usr/bin`)

| 빌드 산출물 | 보드 경로 |
|-------------|-----------|
| `daemon_a` | `/usr/bin/daemon_a` |
| `daemon_b` | `/usr/bin/daemon_b` |
| `y2k38_offsetctl` | `/usr/bin/y2k38_offsetctl` |

`LINK_STATIC=1`이면 이 파일들만 복사해도 동작한다 (`.so` 불필요).

### 3.4 커널 offset 설정 (보드 `/etc`)

| 파일 | 경로 | 용도 |
|------|------|------|
| offset | `/etc/y2k38_offset` | wrap 후 UTC 복구 |

형식:

```
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 0
```

교정:

```bash
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
```

경로 변경:

```bash
export Y2K38_KERNEL_OFFSET_FILE=/custom/path/offset
```

### 3.5 부팅 훅 (보드, 선택)

| 소스 | 보드 권장 경로 |
|------|----------------|
| `scripts/board-init-y2k38.sh` | `/usr/share/y2k38/board-init-y2k38.sh` |

`make stage` 시 `staging/usr/share/y2k38/`에 들어간다.
`/etc/rc.local` 등에서 호출하거나 `Y2K38_START_DAEMONS=1`로 데몬을 띄운다.

---

## 4. 한눈에 보는 최종 레이아웃

### 개발 호스트 (`$SYSROOT = /opt/eldk/ppc_82xx`)

```
$SYSROOT/usr/
├── include/
│   ├── y2k38.h
│   └── y2k38/
│       ├── types.h
│       ├── time.h
│       └── eventlog.h
├── lib/
│   ├── liby2k38safe.a
│   ├── liby2k38safe.so.1.0.0
│   ├── liby2k38safe.so.1 → so.1.0.0
│   ├── liby2k38safe.so   → so.1
│   └── gcc-specs/
│       └── y2k38.specs
└── bin/
    ├── daemon_a
    ├── daemon_b
    └── y2k38_offsetctl
```

### 타깃 보드

```
/usr/bin/daemon_a
/usr/bin/daemon_b
/usr/bin/y2k38_offsetctl
/usr/lib/liby2k38safe.so.1.0.0     # 동적 링크 앱일 때만
/usr/lib/liby2k38safe.so.1 → ...
/etc/y2k38_offset
/usr/share/y2k38/board-init-y2k38.sh   # 선택
```

---

## 5. 권장 설치 절차 (복사용)

```bash
# --- 호스트 ---
export ELDK_ARCH=ppc_82xx
export ELDK_ROOT=/opt/eldk
export CROSS=${ELDK_ARCH}-
export SYSROOT=${ELDK_ROOT}/${ELDK_ARCH}
export PATH=${ELDK_ROOT}/usr/bin:$PATH

cd /path/to/y2k38-abi
# (필요 시) toolchain/abi/y2k38.specs 경로를 SYSROOT에 맞게 수정
make install-sysroot CROSS=$CROSS SYSROOT=$SYSROOT

. scripts/environment-setup-y2k38.sh
y2k38_gcc myapp.c -ly2k38safe -o myapp

# --- 보드 배포 ---
./scripts/cross-build-eldk.sh stage
./scripts/deploy-board.sh root@BOARD:/
```

---

## 6. 체크리스트

### 호스트 (컴파일·링크)

- [ ] `$SYSROOT/usr/include/y2k38/{types,time,eventlog}.h`
- [ ] `$SYSROOT/usr/include/y2k38.h`
- [ ] `$SYSROOT/usr/lib/liby2k38safe.a`
- [ ] `$SYSROOT/usr/lib/liby2k38safe.so.1.0.0` + `.so.1` + `.so`
- [ ] (선택) `$SYSROOT/usr/lib/gcc-specs/y2k38.specs`

### 보드 (실행)

- [ ] `/usr/bin/daemon_a`, `daemon_b`, `y2k38_offsetctl`
- [ ] (동적 링크 시) `/usr/lib/liby2k38safe.so.1*`
- [ ] (선택) `/etc/y2k38_offset` 교정
- [ ] `ldd` / `y2k38_offsetctl show` 확인

---

## 7. 자주 하는 실수

| 증상 | 원인 | 해결 |
|------|------|------|
| `y2k38/time.h: No such file` | 헤더 미설치 또는 `-I` 누락 | `install-sysroot` + `-isystem $SYSROOT/usr/include` |
| `undefined reference to y2k38_time` | `-ly2k38safe` 누락 | 링크 줄 끝에 `-ly2k38safe` 또는 `.a` |
| `liby2k38safe.so.1: not found` | 보드에 `.so` 없음 | `/usr/lib`에 soname 링크까지 복사, 또는 정적 링크 |
| specs가 안 먹음 | specs 안 경로 ≠ 실제 SYSROOT | `y2k38.specs` 절대 경로 수정 |
| 헤더만 보드에 넣고 lib 없음 | 역할 혼동 | 헤더는 호스트, `.so`는 보드 |

---

## 8. 한 줄 요약

| 종류 | 호스트 (sysroot) | 보드 |
|------|------------------|------|
| **헤더** | `$SYSROOT/usr/include/y2k38/` | 보통 불필요 |
| **정적 `.a`** | `$SYSROOT/usr/lib/` | 불필요 |
| **동적 `.so`** | `$SYSROOT/usr/lib/` (링크용) | **`/usr/lib/`** (실행용) |
| **specs** | `$SYSROOT/usr/lib/gcc-specs/` | 불필요 |
| **바이너리** | `$SYSROOT/usr/bin/` (선택) | **`/usr/bin/`** |
| **offset** | — | **`/etc/y2k38_offset`** |

**핵심:** 헤더는 크로스 빌드용으로 **sysroot include**에, 동적 라이브러리는 링크용으로 **sysroot lib** + 실행용으로 **보드 `/usr/lib`**에 둔다. 나머지는 specs(호스트), 바이너리·offset(보드)이다.
