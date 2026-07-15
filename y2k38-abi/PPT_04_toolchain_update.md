# 발표 자료 — 개발환경(툴체인) 갱신

**대상:** ELDK 3.1.1 + y2k38 ABI를 sysroot에 넣는 방법  
**분량:** PPT 2~3장

---

## Slide 1 — 무엇을 “갱신”하는가

### 하지 않는 것

- glibc `time_t`를 64-bit로 바꾸지 않음
- ELDK GCC 전체를 다시 빌드하지 않음 (일반 경로)

### 하는 것

ELDK **sysroot**에 Y2K38 **병행 ABI 패키지**를 설치한다.

```
$SYSROOT/usr/
├── include/y2k38/…     ← 헤더
├── lib/liby2k38safe.*  ← 정적/동적 라이브러리
├── lib/gcc-specs/y2k38.specs
└── bin/…               ← 참고용 교차 빌드 바이너리
```

예: `SYSROOT=/opt/eldk/ppc_82xx` (`ppc_8xx` / `ppc_4xx`는 보드에 맞게)

### 한 줄

> 툴체인 갱신 = **크로스 컴파일러가 찾는 include/lib에 y2k38을 심는 것**

---

## Slide 2 — 단계별 절차

### 1) ELDK 환경

```bash
export PATH=/opt/eldk/usr/bin:$PATH
# 또는: . /opt/eldk/eldk_init ppc_82xx

export ELDK_ARCH=ppc_82xx
export ELDK_ROOT=/opt/eldk
export CROSS=${ELDK_ARCH}-
export SYSROOT=${ELDK_ROOT}/${ELDK_ARCH}

which ppc_82xx-gcc   # 확인
```

### 2) (필요 시) specs 경로 수정

`toolchain/abi/y2k38.specs` 안의 `/opt/eldk/ppc_82xx/...` 를 실제 SYSROOT에 맞춤.

### 3) 한 방 설치

```bash
cd /path/to/y2k38-abi
./scripts/cross-build-eldk.sh install-sysroot
# 등가:
# make install-sysroot CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
```

설치 결과:

| 항목 | 경로 |
|------|------|
| 헤더 | `$SYSROOT/usr/include/y2k38/` |
| lib | `$SYSROOT/usr/lib/liby2k38safe.a` / `.so*` |
| specs | `$SYSROOT/usr/lib/gcc-specs/y2k38.specs` |

### 4) 개발 셸

```bash
. scripts/environment-setup-y2k38.sh
y2k38_gcc myapp.c -ly2k38safe -o myapp
```

또는:

```bash
ppc_82xx-gcc --sysroot=$SYSROOT \
  -specs=$SYSROOT/usr/lib/gcc-specs/y2k38.specs \
  myapp.c -ly2k38safe -o myapp
```

### 5) 검증

```bash
ls $SYSROOT/usr/include/y2k38/
${CROSS}readelf -h daemons/daemon_a/daemon_a   # ELF32 PowerPC
```

---

## Slide 3 — 운영·팀 관점 (선택)

### 갱신 전략

| 방식 | 설명 |
|------|------|
| **기존 sysroot에 install** | 단순, 전사 기본 툴체인에 ABI 추가 |
| **sysroot 복제 flavor** | `cp -a ppc_82xx ppc_82xx_y2k38` 후 복제본만 설치 → 레거시와 분리 |

### 앱 빌드 체크리스트

- [ ] `#include <y2k38/time.h>`
- [ ] `-ly2k38safe` 또는 정적 `.a`
- [ ] 벽시계에 `time_t` / `%ld` 사용 안 함
- [ ] 보드 배포용은 `./scripts/cross-build-eldk.sh stage`

### 발표 요약

1. 툴체인 갱신 ≠ 커널 재작성, = **sysroot ABI 패키지**
2. 명령 핵심: **`install-sysroot`**
3. 개발자는 **y2k38_gcc / -specs / -ly2k38safe** 로 사용
