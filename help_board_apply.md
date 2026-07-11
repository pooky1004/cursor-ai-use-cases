`--help` / `-h` 지원을 추가했습니다. 보드에서 구동하기 위한 조치도 아래에 정리했습니다.

---

## 1. `--help` 사용법

```bash
y2k38_offsetctl --help
y2k38_offsetctl -h
y2k38_offsetctl help
y2k38_offsetctl          # 인자 없이 실행해도 동일 도움말
```

도움말은 **stdout**으로 출력되며, 종료 코드는 **0**입니다.

출력 예:

```
y2k38_offsetctl — Y2K38 kernel offset and wall-clock tool

Usage:
  y2k38_offsetctl show [--file PATH]
  y2k38_offsetctl get-time [--file PATH]
  y2k38_offsetctl set-time <YYYY-MM-DD:hh:mm:ss> [--file PATH] [--notify]
  ...

Options:
  --file PATH       offset file (default: /etc/y2k38_offset or
                    $Y2K38_KERNEL_OFFSET_FILE)
  --notify          send SIGHUP to daemon_a/daemon_b after change
  ...

Formula:  real_utc = sign_extend(kernel_time_t) + OFFSET
```

---

## 2. Embedded Linux 보드에서 구동하기 — 전체 흐름

```mermaid
flowchart LR
    subgraph host ["개발 호스트 (ELDK)"]
        A[cross-build] --> B[stage]
        B --> C[deploy-board.sh]
    end
    subgraph board ["PPC32 보드"]
        D[/usr/bin/y2k38_offsetctl]
        E[/etc/y2k38_offset]
        F[daemon_a / daemon_b]
        G[board-init-y2k38.sh]
        C --> D
        C --> E
        G --> D
        G --> F
        E --> D
        E --> F
    end
```

---

## 3. 단계별 보드 조치

### 3.1 개발 호스트 — 교차 빌드

보드 아키텍처에 맞게 ELDK로 빌드합니다 (예: `ppc_82xx`).

```bash
cd /path/to/y2k38-abi
export ELDK_ARCH=ppc_82xx
export ELDK_ROOT=/opt/eldk
./scripts/cross-build-eldk.sh stage
```

생성물 (`staging/`):

| 경로 | 설명 |
|------|------|
| `staging/usr/bin/y2k38_offsetctl` | OFFSET·시계 관리 CLI (**정적 링크**) |
| `staging/usr/bin/daemon_a`, `daemon_b` | 참조 데몬 (정적 링크) |
| `staging/usr/lib/liby2k38safe.so*` | 자체 앱용 공유 라이브러리 |
| `staging/etc/y2k38_offset` | OFFSET placeholder |
| `staging/usr/share/y2k38/board-init-y2k38.sh` | 부팅 스크립트 |

`y2k38_offsetctl`은 기본 **`LINK_STATIC=1`** 이라 보드에 `.so` 없이도 동작합니다.

---

### 3.2 보드 — 파일 배포

```bash
# 호스트에서
./scripts/deploy-board.sh root@192.168.1.10:/
```

수동 배포 예:

```bash
scp staging/usr/bin/y2k38_offsetctl root@BOARD:/usr/bin/
scp staging/usr/bin/daemon_a staging/usr/bin/daemon_b root@BOARD:/usr/bin/
scp staging/etc/y2k38_offset root@BOARD:/etc/y2k38_offset
chmod +x /usr/bin/y2k38_offsetctl /usr/bin/daemon_a /usr/bin/daemon_b
```

보드에서 ELF 확인:

```bash
file /usr/bin/y2k38_offsetctl
# ELF 32-bit MSB executable, PowerPC ...

ldd /usr/bin/y2k38_offsetctl
# not a dynamic executable  (정적 링크 시)
```

---

### 3.3 보드 — 필수 시스템 요건

| 항목 | 필요 이유 |
|------|-----------|
| **32-bit `time_t` Linux** | Y2K38 대상 환경 (ELDK PPC32) |
| **`gettimeofday(2)`** | kernel raw 시계 읽기 |
| **`settimeofday(2)` 또는 `date`** | `set-time` 시 커널 시계 설정 (root) |
| **쓰기 가능 `/etc`** | `/etc/y2k38_offset` 저장 |
| **root 권한** | 시계 설정·`/etc` 쓰기 |
| **`killall`** (선택) | `--notify` 시 데몬 SIGHUP |

`set-time` 동작 순서:

1. `settimeofday(int32 잔여 초)` 시도  
2. 실패 시 `date -s @<kernel_sec>`  
3. (가능하면) `hwclock -w`로 RTC 동기화  
4. `/etc/y2k38_offset`에 OFFSET 저장  

---

### 3.4 보드 — OFFSET 파일 준비

기본 경로: **`/etc/y2k38_offset`**

```
# y2k38 kernel clock offset (seconds)
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 0
```

| 상황 | 조치 |
|------|------|
| **pre-2038, 커널 정상** | `OFFSET 0` 유지 또는 파일 생략 |
| **2038 wrap 이후** | `set-time` 또는 `calibrate` 실행 |
| **GPS/NTP 있음** | `calibrate <epoch>` 권장 |
| **wrap 1회만 가정** | `set-u32-wrap 1` |

환경 변수로 경로 변경:

```bash
export Y2K38_KERNEL_OFFSET_FILE=/custom/path/offset
```

---

### 3.5 보드 — 최초 시계 설정 (운영)

#### A. 달력 형식으로 설정 (권장)

```bash
y2k38_offsetctl set-time 2038-01-19:03:15:48 \
  --file /etc/y2k38_offset \
  --notify
```

#### B. epoch로 OFFSET만 교정 (커널 시계는 그대로)

```bash
y2k38_offsetctl calibrate 2147483748 \
  --file /etc/y2k38_offset \
  --notify
```

#### C. 확인

```bash
y2k38_offsetctl show
y2k38_offsetctl get-time
# → 2038-01-19:03:15:48
```

`show` 출력 예:

```
offset_file = /etc/y2k38_offset
offset      = 4294967296
kernel_raw  = -2147483548
utc_now     = 2038-01-19:03:15:48
epoch       = 2147483748 (2038-01-19T03:15:48Z)
```

---

### 3.6 보드 — 데몬·다중 프로세스 통일

모든 y2k38 프로세스가 **같은 OFFSET 파일**을 써야 시계가 일치합니다.

```bash
# 데몬 기동 (동일 offset 파일)
daemon_a /var/log/y2k38_events.log \
  --offset-file /etc/y2k38_offset \
  --auto-wrap &

daemon_b /var/log/y2k38_events.log /var/run/deltas.out 10 \
  --offset-file /etc/y2k38_offset &
```

OFFSET 변경 후 동기화:

```bash
y2k38_offsetctl set-time 2100-01-01:00:00:00 --notify
# 또는
y2k38_offsetctl sync --notify
```

| 메커니즘 | 대상 |
|----------|------|
| `--notify` → SIGHUP | `daemon_a`, `daemon_b` 즉시 reload |
| mtime poll | `y2k38_time()` 호출 시 자동 reload |
| `reload` / `sync` | 현재 프로세스만 강제 reload |

---

### 3.7 보드 — 부팅 시 자동 적용

`board-init-y2k38.sh`를 보드에 설치:

```bash
scp staging/usr/share/y2k38/board-init-y2k38.sh root@BOARD:/usr/share/y2k38/
chmod +x /usr/share/y2k38/board-init-y2k38.sh
```

`/etc/rc.local` 또는 init 스크립트에 추가:

```sh
# /etc/rc.local (예)
export Y2K38_KERNEL_OFFSET_FILE=/etc/y2k38_offset
export Y2K38_START_DAEMONS=1
/usr/share/y2k38/board-init-y2k38.sh
```

부팅 시:

1. `y2k38_offsetctl show` — OFFSET·UTC 확인  
2. `reload` — OFFSET 메모리 반영  
3. (옵션) `daemon_a` / `daemon_b` 기동  

---

### 3.8 보드 — 자체 앱 연동

y2k38를 쓰는 앱은:

```c
#include <y2k38/time.h>

y2k38_clock_apply_offset_default(NULL);  /* /etc/y2k38_offset 로드 */
y2k38_time_t now = y2k38_time(NULL);     /* 복구된 UTC */
```

동적 링크 시 보드에 라이브러리 필요:

```bash
scp staging/usr/lib/liby2k38safe.so.1.0.0 root@BOARD:/usr/lib/
ssh root@BOARD 'cd /usr/lib && ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1'
ssh root@BOARD 'cd /usr/lib && ln -sf liby2k38safe.so.1 liby2k38safe.so'
```

`y2k38_offsetctl`·데몬은 정적 링크라 `.so` 없이 동작합니다.

---

## 4. 보드 체크리스트

### 배포 직후

- [ ] `/usr/bin/y2k38_offsetctl` 실행 가능 (`file`로 PPC32 ELF 확인)
- [ ] `/etc/y2k38_offset` 존재·읽기 가능
- [ ] `y2k38_offsetctl --help` 정상 출력
- [ ] `y2k38_offsetctl show` — `utc_now`가 실제 시계와 일치

### 2038 대비

- [ ] pre-2038: `OFFSET 0` 또는 파일 없음
- [ ] 2038 통과 예상: `daemon_a --auto-wrap` 사용
- [ ] wrap 후: `set-time` 또는 `calibrate` 실행
- [ ] 모든 데몬·앱이 **동일** `/etc/y2k38_offset` 사용

### 권한·도구

- [ ] `set-time`은 **root**로 실행
- [ ] `date` 명령 존재 (settimeofday 실패 시 fallback)
- [ ] (선택) `hwclock` — RTC 백업
- [ ] (선택) `killall` — `--notify`용

---

## 5. 시나리오별 요약

| 시나리오 | 보드 조치 |
|----------|-----------|
| **신규 설치 (pre-2038)** | 바이너리 배포 → `show` 확인 → 데몬 기동 |
| **2038 이후 최초 기동** | `set-time YYYY-MM-DD:hh:mm:ss --notify` |
| **RTC 교체·시계 점프** | `calibrate` 또는 `set-time` 재실행 |
| **OFFSET만 수정** | `set` / `calibrate` + `--notify` |
| **데몬 재시작 없이 동기화** | `sync --notify` |

---

## 6. 한 줄 요약

**보드에서 할 일:** ELDK로 빌드한 `y2k38_offsetctl`을 `/usr/bin`에 배포하고, `/etc/y2k38_offset`을 준비한 뒤, root로 `set-time` 또는 `calibrate`로 시계를 맞추고, 모든 y2k38 프로세스가 그 파일을 공유하게 하면 됩니다. `y2k38_offsetctl --help`로 명령 전체를 확인할 수 있습니다.