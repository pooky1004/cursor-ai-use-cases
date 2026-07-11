`lib/` 디렉터리는 **`liby2k38safe`** 라이브러리의 소스 3개로 구성됩니다. Makefile이 이들을 묶어 `liby2k38safe.a`(정적)와 `liby2k38safe.so`(동적)를 만듭니다.

```
lib/
├── y2k38_time.c      ← 핵심 (시계·연산·변환·대기·auto-wrap)
├── y2k38_offset.c    ← OFFSET 파일·계산 유틸
└── y2k38_eventlog.c  ← Daemon A/B용 EVENT 로그 I/O
```

공개 API는 `include/y2k38/*.h`에, 구현은 위 `.c` 파일에 있습니다.

---

## 전체 역할 분담

```
                    ┌─────────────────────┐
                    │   y2k38_time.c      │
                    │  (시계의 중심)       │
                    └──────────┬──────────┘
                               │
              ┌────────────────┼────────────────┐
              ▼                ▼                ▼
    gettimeofday()      civil time 변환    sleep_until
    auto-wrap 감지      format/parse       difftime
              │
              ▼
    ┌─────────────────────┐
    │  y2k38_offset.c     │  ← OFFSET 파일 load/save/compute
    └─────────────────────┘
              │
              ▼
    ┌─────────────────────┐
    │ y2k38_eventlog.c    │  ← EVENT 라인 (when = epoch64)
    └─────────────────────┘
```

| 파일 | 한 줄 요약 |
|------|-----------|
| `y2k38_time.c` | 32-bit kernel 시계를 64-bit UTC로 복구·연산·대기 |
| `y2k38_offset.c` | `/etc/y2k38_offset` 영속화·교정 계산 |
| `y2k38_eventlog.c` | 데몬용 `EVENT id epoch64 msg` 읽기/쓰기 |

---

## 1. `y2k38_time.c` (약 556줄) — 핵심 엔진

가장 큰 파일입니다. **시계 읽기**, **wrap 자동 감지**, **64-bit 연산**, **달력 변환**, **상대/절대 대기**를 담당합니다.

### 1.1 내부 전역 상태

```c
static y2k38_time_t g_kernel_offset;     // real_utc = raw + offset
static int          g_auto_wrap;         // wrap 자동 감지 on/off
static y2k38_time_t g_last_kernel_raw;   // 이전 raw (wrap 비교용)
static unsigned     g_wrap_count;        // wrap 감지 횟수
static pthread_mutex_t g_clk_mu;         // 시계 상태 보호
// mock (테스트용)
static int g_mock_enabled;
static y2k38_time_t g_mock_now;
static int g_mock_kernel_enabled;
static int32_t g_mock_kernel_sec;
```

핵심 공식:

```
real_utc = kernel_raw + g_kernel_offset
```

`kernel_raw`는 `gettimeofday().tv_sec`를 int64로 넓힌 값입니다.

### 1.2 시계 읽기 계층

| 함수 | 역할 |
|------|------|
| `read_kernel_sec_raw()` | libc `gettimeofday` / `time()` → raw 초 (내부) |
| `y2k38_time_kernel_raw()` | offset **미적용** raw (calibrate용) |
| `y2k38_time()` | **복구된 UTC** = raw + offset. auto-wrap 검사 포함 |
| `y2k38_gettimeofday()` | `struct y2k38_timeval` (64-bit sec + usec) |

`y2k38_time()` 호출 흐름:

```
pthread_mutex_lock
  → read_kernel_sec_raw()
  → detect_and_apply_wrap_locked()   // auto_wrap 켜져 있으면
  → now = raw + g_kernel_offset
pthread_mutex_unlock
```

### 1.3 Auto-wrap (장기 실행 데몬용)

| 함수 | 역할 |
|------|------|
| `y2k38_clock_set_auto_wrap(on, path)` | wrap 감지 활성화 + OFFSET 저장 경로 |
| `detect_and_apply_wrap_locked()` | raw가 ~2³¹ 초 이상 **뒤로 점프**하면 OFFSET += 2³² |
| `y2k38_clock_poll_wrap()` | `y2k38_time()` 없이 wrap만 수동 검사 |
| `y2k38_clock_on_wrap_callback()` | wrap 발생 시 콜백 등록 |
| `y2k38_clock_get_wrap_count()` | 감지된 wrap 횟수 |

감지 조건 (내부):

```
raw < last_raw  AND  (last_raw - raw) >= 2^31
→ OFFSET += 4294967296
→ /etc/y2k38_offset 저장 (path 지정 시)
```

2038 **이전에 시작**해 **이후에도 계속** 도는 데몬이 재시작 없이 UTC를 유지하게 하는 핵심 로직입니다.

### 1.4 Mock (테스트·데모 전용)

| 함수 | 역할 |
|------|------|
| `y2k38_clock_set_mock(1, epoch)` | `y2k38_time()`이 항상 그 UTC 반환 |
| `y2k38_clock_set_mock_kernel(1, sec32)` | kernel raw만 32-bit 값으로 에뮬레이션 |

예제·단위 테스트에서 2038 경계를 시뮬레이션할 때 사용합니다.

### 1.5 64-bit 시간 연산

| 함수 | 역할 |
|------|------|
| `y2k38_difftime_sec(later, earlier)` | `later - earlier` (int64, overflow 없음) |
| `y2k38_timeval_diff()` | sub-second 포함 차분 |
| `y2k38_is_past_time_t32_max(t)` | `t > 2147483647` 여부 |

Daemon B의 delta 계산이 여기에 의존합니다.

### 1.6 달력 변환 (libc `gmtime` 비의존)

2038 이후 epoch도 변환하려면 32-bit `gmtime()`에 의존하면 안 됩니다.  
**Howard Hinnant** 스타일 civil 알고리즘을 자체 구현했습니다.

| 함수 | 역할 |
|------|------|
| `civil_from_days()` / `days_from_civil()` | 내부: epoch 일수 ↔ 연월일 |
| `y2k38_gmtime_r()` | epoch → `struct y2k38_tm` (UTC) |
| `y2k38_localtime_r()` | 현재는 UTC와 동일 (TZ 미구현) |
| `y2k38_timegm()` | `struct y2k38_tm` → epoch |

2100-01-01 같은 먼 미래도 ISO-8601 포맷이 가능합니다.

### 1.7 직렬화 (로그·파일용)

| 함수 | 역할 |
|------|------|
| `y2k38_format_epoch()` | `%lld` 십진 문자열 |
| `y2k38_format_iso8601_utc()` | `2038-01-19T03:14:08Z` 형식 |
| `y2k38_parse_epoch()` | `strtoll` 기반 64-bit 파싱 |
| `y2k38_fprint_epoch()` | `fprintf`로 epoch 출력 |

**절대 `%ld` + `time_t`로 로그에 쓰지 않습니다.**

### 1.8 대기 (Y2K38-safe sleep)

| 함수 | 역할 |
|------|------|
| `y2k38_nsleep_relative(sec, nsec)` | 상대 `nanosleep` (EINTR 재시도) |
| `y2k38_sleep_until(wake_utc)` | 절대 시각까지 **상대 sleep 루프** |
| `y2k38_sleep_set_max_chunk()` | 1회 sleep 상한 (기본 3600초) |

`y2k38_sleep_until` 내부:

```
loop:
  rem = wake_utc - y2k38_time()
  if rem <= 0: return
  nanosleep(min(rem, chunk))   // 항상 상대 대기
```

지금은 2038 전, 깨울 때는 2038 후인 경우에도 안전합니다.

### 1.9 OFFSET get/set (이 파일에 있는 부분)

| 함수 | 역할 |
|------|------|
| `y2k38_clock_set_kernel_offset()` | in-process OFFSET 설정 (mutex) |
| `y2k38_clock_get_kernel_offset()` | 현재 OFFSET 조회 |

파일 load/save는 `y2k38_offset.c`에 있습니다.

---

## 2. `y2k38_offset.c` (약 116줄) — OFFSET 영속화·계산

시계 **보정값**을 계산하고 `/etc/y2k38_offset` 같은 파일로 저장·로드합니다.  
`y2k38_time.c`의 auto-wrap이 wrap 감지 시 이 모듈의 `save`를 호출합니다.

### 2.1 OFFSET 계산

| 함수 | 수식/의미 |
|------|-----------|
| `y2k38_clock_compute_offset(true_utc, raw)` | `OFFSET = true_utc - kernel_raw` |
| `y2k38_clock_u32_wrap_offset(n)` | `n × 2³²` (4294967296 × n) |

GPS/NTP calibrate 예:

```c
off = y2k38_clock_compute_offset(gps_epoch, y2k38_time_kernel_raw(NULL));
y2k38_clock_set_kernel_offset(off);
y2k38_clock_save_offset_file("/etc/y2k38_offset", off);
```

### 2.2 파일 I/O

**파일 형식** (`/etc/y2k38_offset`):

```
# y2k38 kernel clock offset (seconds)
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 4294967296
```

| 함수 | 역할 |
|------|------|
| `y2k38_clock_load_offset_file(path)` | 파일 읽기 → `y2k38_clock_set_kernel_offset()` |
| `y2k38_clock_save_offset_file(path, off)` | OFFSET 파일 쓰기 |
| `y2k38_clock_apply_offset_default(path)` | env → 기본 경로 → load. 없으면 return 1 (OFFSET=0) |

경로 우선순위 (`apply_offset_default`):

1. 인자로 받은 `path`
2. 환경 변수 `Y2K38_KERNEL_OFFSET_FILE`
3. `/etc/y2k38_offset` (`Y2K38_OFFSET_PATH_DEFAULT`)

데몬 기동 시 한 번 호출하는 패턴:

```c
y2k38_clock_apply_offset_default(NULL);  // pre-2038이면 OFFSET 0
```

### 2.3 의존 관계

- `load` → `y2k38_parse_epoch()` (`y2k38_time.c`)
- `save` → `y2k38_fprint_epoch()` (`y2k38_time.c`)
- `load` → `y2k38_clock_set_kernel_offset()` (`y2k38_time.c`)

---

## 3. `y2k38_eventlog.c` (약 68줄) — EVENT 로그 포맷

Daemon A/B가 쓰는 **텍스트 로그 한 줄** 형식을 정의합니다.  
시간 필드는 반드시 **64-bit 십진 epoch**입니다.

### 3.1 데이터 구조 (`include/y2k38/eventlog.h`)

```c
struct y2k38_event {
    char         id[64];
    y2k38_time_t when;    // int64 epoch — 2038 이후 값 OK
    char         msg[256];
};
```

### 3.2 함수

| 함수 | 역할 |
|------|------|
| `y2k38_event_append(fp, ev)` | `EVENT <id> <epoch64> <msg>\n` 쓰기 + fflush |
| `y2k38_event_parse_line(line, ev)` | 한 줄 파싱. return: 1=성공, 0=빈줄/주석, -1=오류 |

**쓰기 예:**

```
EVENT FUT2038 2147483648 first-second-after-time_t-overflow
EVENT FAR2100 4102444800 century-maintenance-window
```

**잘못된 예 (이 라이브러리가 방지하는 것):**

```c
fprintf(fp, "EVENT E1 %ld msg\n", (long)time(NULL));  // 32-bit 잘림
```

### 3.3 파싱 규칙

- `#` 로 시작하는 줄, 빈 줄 → skip (return 0)
- `EVENT ` 접두어 필수
- epoch 필드는 `y2k38_parse_epoch()` → full int64
- id·msg는 공백 구분, 고정 크기 버퍼

Daemon B는 파싱한 `ev.when`과 `y2k38_time()`으로 `y2k38_difftime_sec()`을 계산합니다.

---

## 4. 파일 간 호출 관계

```
y2k38_eventlog.c
  └─ y2k38_fprint_epoch, y2k38_parse_epoch  (time.c)
  └─ y2k38_event_append / parse_line

y2k38_offset.c
  └─ y2k38_clock_set_kernel_offset       (time.c)
  └─ y2k38_parse_epoch, y2k38_fprint_epoch (time.c)

y2k38_time.c
  └─ y2k38_clock_save_offset_file      (offset.c)  ← auto-wrap 시
  └─ y2k38_clock_u32_wrap_offset       (offset.c)  ← wrap 시 +2^32
```

`time.c` ↔ `offset.c`는 **순환 참조**가 있지만, 둘 다 같은 `liby2k38safe`에 링크되므로 문제 없습니다.

---

## 5. 빌드 산출물

Makefile (`LIB_SRCS`):

```makefile
lib/y2k38_time.c
lib/y2k38_eventlog.c
lib/y2k38_offset.c
  → liby2k38safe.a   (정적)
  → liby2k38safe.so  (soname liby2k38safe.so.1)
```

| 산출물 | 용도 |
|--------|------|
| `liby2k38safe.a` | 데몬·툴 정적 링크 (기본 `LINK_STATIC=1`) |
| `liby2k38safe.so` | 동적 링크 앱 (`-ly2k38safe`) |

`y2k38_time.c`만 **pthread**에 의존합니다 (`g_clk_mu`). ELDK에서 `-pthread` 링크가 필요합니다.

---

## 6. 역할별 “어느 파일을 보면 되나”

| 하고 싶은 일 | 주로 보는 파일 |
|-------------|---------------|
| “지금” UTC 얻기 | `y2k38_time.c` → `y2k38_time()` |
| 2038 넘어서도 데몬 유지 | `y2k38_time.c` → auto-wrap |
| NTP/GPS로 시계 맞추기 | `y2k38_offset.c` → compute + save |
| 이벤트 로그 저장/읽기 | `y2k38_eventlog.c` |
| 미래 시각까지 sleep | `y2k38_time.c` → `y2k38_sleep_until()` |
| 2100년 ISO 문자열 | `y2k38_time.c` → `y2k38_gmtime_r` + format |

---

## 7. 한 줄 요약

| 파일 | 역할 |
|------|------|
| **`y2k38_time.c`** | 32-bit kernel → 64-bit UTC, wrap 자동 보정, 연산·변환·대기 |
| **`y2k38_offset.c`** | OFFSET 계산·`/etc/y2k38_offset` 파일 load/save |
| **`y2k38_eventlog.c`** | Daemon용 `EVENT id epoch64 msg` 로그 I/O |

세 파일이 합쳐져 **`liby2k38safe`**가 되며, 앱·데몬은 `time_t` 대신 이 라이브러리만 쓰면 Y2K38 저장·연산·장기 실행이 처리됩니다.