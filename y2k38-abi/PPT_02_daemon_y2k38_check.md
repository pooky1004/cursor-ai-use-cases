# 발표 자료 — Y2K38 Check 데몬

**대상:** `daemon_y2k38_check`  
**분량:** PPT 3~4장  
**역할:** wrap 감시 · OFFSET 갱신 · peer SIGHUP 통지

---

## Slide 1 — 개요: Check 데몬이 하는 일

### 한 줄

> 보드에서 **커널 32-bit wrap을 감시**하고, OFFSET를 맞춘 뒤 **다른 y2k38 프로세스에 SIGHUP**으로 알린다.

### 왜 별도 데몬인가

| 방식 | 한계 |
|------|------|
| 앱마다 auto-wrap만 | OFFSET 파일 경합, 통지 누락 가능 |
| **Check 데몬 중심** | wrap 담당을 한곳에, SIGHUP 리스트로 전파 |

### 관리하는 파일

| 경로 | 내용 |
|------|------|
| `/etc/y2k38_offset` | `OFFSET <int64>` |
| `/var/run/y2k38_sighup.list` | `process_name pid` 구독자 목록 |

```
              ┌─────────────────────┐
   kernel ──►│ daemon_y2k38_check  │──SIGHUP──► daemon_a
   time_t     │  poll / wrap / save │──────────► daemon_b
              │  OFFSET file        │──────────► 기타 y2k38 앱
              └─────────────────────┘
```

---

## Slide 2 — 기능

### 기동·옵션

```bash
daemon_y2k38_check [--offset-file PATH] [--list PATH] [--once] [--debug|-d]
```

| 옵션 | 의미 |
|------|------|
| `--offset-file` | OFFSET 경로 (기본 `/etc/y2k38_offset`) |
| `--list` | SIGHUP 리스트 (기본 `/var/run/y2k38_sighup.list`) |
| `--once` | 한 번 poll 후 종료 (테스트) |
| `--debug` / `-d` | 주기 상태 로그 (WRAP/SIGHUP는 **항상** 출력) |

### 핵심 동작

1. `y2k38_session_init("daemon_y2k38_check")` — 자기 등록 + OFFSET 로드 + auto-wrap
2. `y2k38_clock_poll_wrap()` — wrap 감지
3. wrap 시: OFFSET `+= 2^32` 저장 → `y2k38_sighup_list_notify(self)` (**자기 제외**)
4. peer SIGHUP 수신 시: OFFSET 재로드 (메시지 **항상** 출력)

### 적응형 체크 주기 (1~60초)

`y2k38_clock_seconds_until_wrap()` 기준:

| 다음 wrap까지 | poll 간격 |
|---------------|-----------|
| ≤ 30초 | **1초** (최소) |
| ≤ 2분 | 5초 |
| ≤ 10분 | 15초 |
| ≤ 1시간 | 30초 |
| 그 이상 (wrap **직후** ≈ 2³²초) | **60초** (최대) |

> wrap 직후 rem ≈ 68년 → 주기를 다시 늘림 (예전 버그: rem=0 → 1초 고정)

---

## Slide 3 — 연동

### 라이브러리와의 관계

Check 데몬은 **비즈니스 로직 없이** `liby2k38safe`만 사용:

- wrap 감지 / OFFSET 저장 → 라이브러리
- 구독 리스트 add/remove/notify → 라이브러리 session API
- “누구에게 알릴지”만 데몬이 오케스트레이션

### Daemon A / B와의 관계

```
[Check] wrap 발견
    │
    ├─ 저장: /etc/y2k38_offset
    └─ SIGHUP → [A][B] session handler
                    └─ OFFSET reload
                       (항상 로그: "received SIGHUP — reloaded offset=…")
```

A/B도 auto-wrap 가능하지만, **운영 권장:** Check가 주감시, A/B는 peer SIGHUP에 반응.

### offsetctl과의 관계

| 주체 | 역할 |
|------|------|
| Check | 자동·주기적 wrap 대응 |
| `y2k38_offsetctl` | 수동 set-time / calibrate / sync |
| 공통 | 같은 OFFSET 파일 + 같은 SIGHUP 리스트 |

---

## Slide 4 — 운영 포인트 (선택)

### 기동 예

```bash
daemon_y2k38_check --offset-file /etc/y2k38_offset
# 점검 시
daemon_y2k38_check --debug
```

### 항상 vs debug 로그

| 이벤트 | 출력 |
|--------|------|
| WRAP / SIGHUP 송수신 | **항상** |
| `utc=… next_poll=… until_wrap=…` | `--debug` 일 때만 |

### 발표 요약

1. Check = **시간 인프라 데몬** (앱 로직 없음)
2. 주기 = **다음 wrap까지 남은 시간**으로 1~60초 조절
3. 연동 축 = **OFFSET 파일 + SIGHUP 리스트 + liby2k38safe**
