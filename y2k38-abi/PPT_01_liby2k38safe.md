# 발표 자료 — liby2k38safe 라이브러리

**대상:** ELDK 3.1.1 / 32-bit PPC Linux  
**분량:** PPT 3~4장  
**산출물:** `liby2k38safe.a` / `liby2k38safe.so.1`

---

## Slide 1 — 개요: 왜 라이브러리가 필요한가

### 문제

| 항목 | 내용 |
|------|------|
| 플랫폼 | 32-bit `time_t` (signed), ELDK에 64-bit `time_t` 옵션 없음 |
| 한계 시각 | 2038-01-19 03:14:07 UTC (`0x7FFFFFFF`) |
| 실패 모드 | 로그 잘림, `future − now` 오버플로, wrap 후 `now` 왜곡 |

### 접근

- libc / 커널의 `time_t`는 **그대로 유지**
- Application 레벨 **병행 ABI**: `y2k38_time_t` (signed 64-bit)
- 공유 구현체: **`liby2k38safe`**

### 한 줄

> 벽시계 저장·연산·프로세스 연동은 전부 `y2k38_*`로, libc `time()`은 래핑만 한다.

```
┌─────────────┐     y2k38_time_t (64-bit)      ┌──────────────┐
│  Daemon A/B │◄──────────────────────────────►│ liby2k38safe │
│ Check/ctl   │   OFFSET + SIGHUP session      │  (userspace) │
└─────────────┘                                └──────┬───────┘
                                                      │ widen / offset
                                               ┌──────▼───────┐
                                               │ libc time_t  │
                                               │ (32-bit)     │
                                               └──────────────┘
```

---

## Slide 2 — 기능 (코어 API)

### 타입·포맷

| 심볼 | 의미 |
|------|------|
| `y2k38_time_t` | epoch 초 (int64), ABI 경계 전용 |
| `struct y2k38_timeval` | 16바이트 (PPC32 EABI) |
| 로그 | `EVENT <id> <epoch64> <msg>` (십진 ASCII) |

### 시계

| API | 역할 |
|-----|------|
| `y2k38_time()` | 복구된 UTC = `sign_extend(kernel) + OFFSET` |
| `y2k38_time_kernel_raw()` | OFFSET 미적용 raw |
| `y2k38_difftime_sec()` | 64-bit 차분 |
| `y2k38_format_*` / `parse_epoch` | 직렬화 |
| `y2k38_gmtime_r` / `timegm` | civil time (32-bit libc 비의존) |

### OFFSET / wrap

| API | 역할 |
|-----|------|
| `y2k38_clock_load/save_offset_file` | `/etc/y2k38_offset` |
| `y2k38_clock_set_auto_wrap` | 장기 실행 시 wrap 자동 감지 → `OFFSET += 2^32` |
| `y2k38_clock_poll_wrap` | 수동 wrap 검사 |
| `y2k38_clock_seconds_until_wrap` | 다음 wrap까지 초 (Check 데몬 주기용) |
| `y2k38_clock_on_wrap_callback` | wrap 시 사용자 콜백 |

### 세션 / 연동

| API | 역할 |
|-----|------|
| `y2k38_session_init(name)` | OFFSET 로드, SIGHUP 등록, 구독 리스트 등록, auto-wrap |
| `y2k38_session_poll_sighup` | peer 통지 후 OFFSET 재로드 |
| `y2k38_sighup_list_notify` | 다른 프로세스에 SIGHUP |

```
real_utc = sign_extend(kernel_time_t) + OFFSET
```

---

## Slide 3 — 연동 (다른 구성 요소와의 관계)

### 누가 링크하는가

```
                    ┌──────────────────┐
                    │  liby2k38safe    │
                    └────────┬─────────┘
           ┌─────────────────┼─────────────────┐
           ▼                 ▼                 ▼
   daemon_y2k38_check   daemon_a / b     y2k38_offsetctl
   (wrap 감시·통지)     (앱 데몬)         (운영 툴)
```

### 데이터·시그널 흐름

1. **기동:** 각 프로세스 `y2k38_session_init()` → `/etc/y2k38_offset` 로드, `/var/run/y2k38_sighup.list`에 등록
2. **평시:** 앱은 `y2k38_time()`만 사용 (저장·delta·sleep)
3. **wrap:** Check(또는 auto-wrap)가 OFFSET `+= 2^32` 저장 → `SIGHUP` → peer가 OFFSET 재로드
4. **수동 교정:** `y2k38_offsetctl set-time / calibrate` → 파일 갱신 → `notify/sync`로 SIGHUP

### 앱 코드 패턴

```c
#include <y2k38/time.h>

y2k38_session_init("my_app");
y2k38_time_t now = y2k38_time(NULL);
y2k38_time_t delta = y2k38_difftime_sec(future, now);
/* 금지: time_t / %ld 로 벽시계 로그 */
```

링크: `-ly2k38safe` 또는 `liby2k38safe.a` 정적 링크

---

## Slide 4 — 정리 / 발표 포인트 (선택)

### 키 메시지

1. **ABI는 userspace** — 커널/glibc `time_t` 비변경 → ELDK 재빌드 최소
2. **단일 진실:** `/etc/y2k38_offset` + SIGHUP 리스트로 프로세스 간 UTC 일치
3. **장기 실행:** auto-wrap으로 재시작 없이 2038 통과 가능

### 산출물 위치

| 파일 | 역할 |
|------|------|
| `include/y2k38/*.h` | 공개 헤더 |
| `lib/liby2k38safe.a` / `.so` | 정적·동적 라이브러리 |
| soname | `liby2k38safe.so.1` |

### 다음 장과의 연결

→ **Y2K38 Check 데몬**: wrap을 누가 감시하고 누구에게 알리는가  
→ **y2k38_offsetctl**: 운영자가 OFFSET·시계를 어떻게 교정하는가
