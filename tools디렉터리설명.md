`tools/` 디렉터리에는 현재 **파일 1개**만 있습니다.

```
tools/
└── y2k38_offsetctl.c    → 빌드 시 /usr/bin/y2k38_offsetctl
```

보드·개발 호스트에서 **커널 32-bit 시계와 실제 UTC 사이의 OFFSET**을 조회·설정·교정하는 **CLI 유틸리티**입니다. `liby2k38safe`의 offset API를 사람이 쓰기 쉬운 명령으로 감싼 도구입니다.

---

## 1. 왜 필요한가?

32-bit ELDK 환경에서는:

```
real_utc = kernel_raw + OFFSET
```

- **kernel_raw**: `gettimeofday().tv_sec` (32-bit, 2038 이후 wrap)
- **OFFSET**: `/etc/y2k38_offset`에 저장하는 보정값

데몬·앱은 `y2k38_time()`만 호출하면 되지만, **OFFSET을 맞추는 작업**은 운영자·NTP 에이전트가 해야 합니다. 그때 쓰는 도구가 `y2k38_offsetctl`입니다.

| 역할 | 담당 |
|------|------|
| 데몬 A/B | `y2k38_time()`, auto-wrap — **읽기·자동 보정** |
| **y2k38_offsetctl** | OFFSET **수동 설정·교정·확인** |

---

## 2. 빌드·설치

```bash
make tools
# → tools/y2k38_offsetctl

make install / make stage
# → /usr/bin/y2k38_offsetctl (보드 배포)
```

- `liby2k38safe.a`에 **정적 링크** (`LINK_STATIC=1` 기본)
- 보드에 `.so` 없이 바이너리만 복사해도 동작

---

## 3. OFFSET 파일 경로

`resolve_path()` 우선순위:

1. `--file PATH` (CLI)
2. 환경 변수 `Y2K38_KERNEL_OFFSET_FILE`
3. 기본값 `/etc/y2k38_offset`

파일 형식 (`lib/y2k38_offset.c`가 쓰는 형식):

```
# y2k38 kernel clock offset (seconds)
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 4294967296
```

---

## 4. 서브커맨드 상세

### 4.1 `show` — 현재 상태 조회

```bash
y2k38_offsetctl show
y2k38_offsetctl show --file /etc/y2k38_offset
```

**동작 (`cmd_show`):**

1. `y2k38_clock_apply_offset_default(path)` — offset 파일 로드 (없으면 OFFSET=0)
2. `y2k38_time_kernel_raw()` — offset **미적용** kernel 초
3. `y2k38_time()` — offset **적용** 복구 UTC
4. ISO-8601 문자열 출력

**출력 예:**

```
offset_file = /etc/y2k38_offset
offset      = 4294967296
kernel_raw  = -2147483548
utc_now     = 2147483748 (2038-01-19T03:15:48Z)
```

| 필드 | 의미 |
|------|------|
| `offset` | 현재 적용 중인 OFFSET (int64) |
| `kernel_raw` | libc가 주는 raw 초 (wrap 시 음수 가능) |
| `utc_now` | 앱이 써야 할 실제 UTC (`raw + offset`) |

보드 점검·데몬 기동 전 확인용입니다.

---

### 4.2 `set` — OFFSET 직접 설정

```bash
y2k38_offsetctl set 4294967296
y2k38_offsetctl set 0 --file /etc/y2k38_offset
```

**동작 (`cmd_set`):**

1. `y2k38_clock_save_offset_file(path, offset)` — 파일 기록
2. `y2k38_clock_set_kernel_offset(offset)` — **현재 프로세스**에 즉시 반영

이미 알고 있는 OFFSET 값을 넣을 때 사용합니다.  
**다른 데몬**은 파일을 다시 읽거나 재기동해야 같은 OFFSET을 씁니다.

---

### 4.3 `set-u32-wrap` — 32-bit wrap 1회분 OFFSET

```bash
y2k38_offsetctl set-u32-wrap 1
# OFFSET = 1 × 2^32 = 4294967296

y2k38_offsetctl set-u32-wrap 2
# OFFSET = 2 × 2^32
```

**동작:**

```c
y2k38_clock_u32_wrap_offset(n)  // n * 4294967296LL
→ cmd_set()
```

커널 시계가 signed 32-bit로 **정확히 한 바퀴** 돌았다고 가정할 때 씁니다.  
GPS 없이 “wrap 1회” 모델만 맞을 때의 빠른 복구입니다.

---

### 4.4 `calibrate` — 신뢰 UTC로 OFFSET 계산 (권장)

```bash
y2k38_offsetctl calibrate 2147483748
y2k38_offsetctl calibrate 2147483748 --file /etc/y2k38_offset
```

**동작 (`cmd_calibrate`):**

```
raw    = y2k38_time_kernel_raw(NULL)     # 지금 kernel raw
offset = true_utc - raw                  # y2k38_clock_compute_offset
→ 파일 저장 + 메모리 반영
```

**출력 예:**

```
true_utc   = 2147483748
kernel_raw = -2147483548
offset     = 4294967296
wrote OFFSET 4294967296 to /etc/y2k38_offset
```

**신뢰 UTC(`true_utc`) 출처 예:**

- GPS 모듈 epoch
- NTP 동기화된 호스트의 `date +%s`
- 운영자가 아는 정확 시각

2038 이후에도 **int64 epoch**를 넘기면 됩니다.

---

### 4.5 `--mock-kernel` (테스트·호스트)

```bash
y2k38_offsetctl calibrate 2147483748 \
  --mock-kernel -2147483548 \
  --file /tmp/test_offset
```

kernel raw를 실제 `gettimeofday` 대신 **지정한 32-bit 값**으로 에뮬레이션합니다.  
보드 없이 wrap 복구 시나리오를 검증할 때 사용 (`make check-offset`에서도 사용).

---

## 5. 내부 구조 (소스 구성)

```
y2k38_offsetctl.c
├── resolve_path()      # offset 파일 경로 결정
├── usage()             # 도움말
├── cmd_show()          # show
├── cmd_set()           # set / set-u32-wrap / calibrate 공통 저장
├── cmd_calibrate()     # compute_offset + cmd_set
└── main()              # 인자 파싱, 서브커맨드 분기
```

**의존하는 라이브러리 API (`lib/`):**

| API | 출처 파일 |
|-----|-----------|
| `y2k38_clock_apply_offset_default` | `y2k38_offset.c` |
| `y2k38_clock_save_offset_file` | `y2k38_offset.c` |
| `y2k38_clock_set_kernel_offset` | `y2k38_time.c` |
| `y2k38_clock_compute_offset` | `y2k38_offset.c` |
| `y2k38_clock_u32_wrap_offset` | `y2k38_offset.c` |
| `y2k38_time_kernel_raw`, `y2k38_time` | `y2k38_time.c` |
| `y2k38_format_iso8601_utc` | `y2k38_time.c` |
| `y2k38_parse_epoch` | `y2k38_time.c` |
| `y2k38_clock_set_mock_kernel` | `y2k38_time.c` |

로직은 거의 없고, **라이브러리 호출 + printf** 수준입니다.

---

## 6. 운영 시나리오

### 시나리오 A — pre-2038, 커널 정상

```bash
y2k38_offsetctl show
# offset=0, utc_now ≈ 실제 시각
# 파일 없어도 OK
```

### 시나리오 B — 2038 wrap 직후 (GPS/NTP 있음)

```bash
TRUE=$(gps_read_epoch)   # 또는 NTP에서 받은 값
y2k38_offsetctl calibrate $TRUE --file /etc/y2k38_offset
y2k38_offsetctl show
# 데몬 재기동 또는 offset 파일 reload
```

### 시나리오 C — wrap 1회만 가정 (GPS 없음)

```bash
y2k38_offsetctl set-u32-wrap 1 --file /etc/y2k38_offset
```

### 시나리오 D — auto-wrap 데몬과 병행

```bash
# 데몬: --auto-wrap (wrap 시 OFFSET += 2^32 자동)
daemon_a /var/log/events.log --auto-wrap --offset-file /etc/y2k38_offset

# 운영자: 주기적 NTP 교정
y2k38_offsetctl calibrate $(ntp_epoch) 
```

auto-wrap은 **wrap 순간**을 잡고, calibrate는 **드리프트**를 잡습니다.

---

## 7. 다른 구성요소와의 관계

```
                    y2k38_offsetctl
                           │
           ┌───────────────┼───────────────┐
           ▼               ▼               ▼
   /etc/y2k38_offset   liby2k38safe    daemon_a/b
   (영속 저장)         (offset API)    (--offset-file 로드)
                           │
                    y2k38_time()
                    real = raw + OFFSET
```

| 구성요소 | OFFSET 쓰기 | OFFSET 읽기 |
|----------|-------------|-------------|
| **y2k38_offsetctl** | **주 담당** (set/calibrate) | show |
| daemon `--auto-wrap` | wrap 시 자동 + 파일 | 기동 시 load |
| daemon `--offset-file` | — | 기동 시 load |
| `y2k38_clock_apply_offset_default` | — | 앱 기동 시 |

---

## 8. 제한·주의

1. **`set`/`calibrate`는 이 프로세스 메모리에만 즉시 반영** — 다른 데몬은 파일 재로드 또는 재기동 필요.
2. **root 권한** — `/etc/y2k38_offset` 쓰기에 보통 필요.
3. **커널 시계 자체는 바꾸지 않음** — `settimeofday` 호출 없음. **앱이 보는 UTC만** OFFSET으로 맞춤.
4. **한 프로세스 CLI** — 장기 감시·자동 NTP는 별도 스크립트/데몬이 `calibrate`를 주기 호출하는 패턴이 적합.

---

## 9. 한 줄 요약

| 항목 | 내용 |
|------|------|
| **파일** | `tools/y2k38_offsetctl.c` 1개 |
| **역할** | `/etc/y2k38_offset` 조회·설정·GPS/NTP 기반 교정 |
| **명령** | `show` / `set` / `set-u32-wrap` / `calibrate` |
| **빌드물** | `/usr/bin/y2k38_offsetctl` (정적 링크) |
| **핵심** | `OFFSET = true_utc - kernel_raw` 를 파일로 남겨 모든 y2k38 앱이 같은 시계를 쓰게 함 |

`lib/`가 “시계 엔진”, `tools/y2k38_offsetctl`은 그 엔진의 **운영용 리모컨**이라고 보면 됩니다.