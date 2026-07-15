# 발표 자료 — y2k38_offsetctl 도구

**대상:** `tools/y2k38_offsetctl`  
**분량:** PPT 3~4장  
**역할:** OFFSET·시스템 시계 운영 / peer 통지

---

## Slide 1 — 개요: 운영자용 컨트롤 패널

### 한 줄

> 보드에서 **OFFSET를 보고·고치고·신뢰 UTC로 맞춘 뒤**, y2k38 프로세스들에게 **SIGHUP으로 동기화**하는 CLI.

### Check 데몬과의 역할 분담

| | Check 데몬 | offsetctl |
|--|------------|----------|
| 성격 | 상시·자동 | 수동·일회 |
| wrap | 주기 poll | — |
| 시계 설정 | — | `set-time` |
| 교정 | auto `+= 2^32` | `calibrate` / `set` / `set-u32-wrap` |
| 통지 | wrap 시 SIGHUP | `--notify` / `sync` / `notify` |

```
운영자 ──► y2k38_offsetctl ──► /etc/y2k38_offset
                      │
                      └── SIGHUP ──► Check / Daemon A / B / …
```

---

## Slide 2 — 기능 (명령 맵)

### 조회

```bash
y2k38_offsetctl show [--file PATH]
```

출력 예: `offset`, `kernel_raw`, `utc_now` (ISO)

### OFFSET 쓰기

| 명령 | 의미 |
|------|------|
| `set <offset>` | OFFSET를 직접 지정 |
| `set-u32-wrap <n>` | `OFFSET = n × 2³²` |
| `calibrate <true_utc_epoch>` | `OFFSET = true_utc − kernel_raw` |

공통: `--file PATH`, `--notify` (저장 후 SIGHUP 리스트 통지)

### 시스템 시각

```bash
y2k38_offsetctl set-time YYYY-MM-DD:hh:mm:ss [--file PATH]
```

1. 커널/시스템 시계를 32-bit residue에 맞게 설정 (`settimeofday` 등)
2. OFFSET 파일 갱신
3. **항상** SIGHUP 리스트 notify (부팅·현장 시각 맞춤용)

### 동기화

| 명령 | 동작 |
|------|------|
| `sync` | 로컬 OFFSET reload + 전원에게 SIGHUP |
| `notify` | SIGHUP만 (리스트의 다른 pid, 자기 제외) |

### 기본 경로

| 항목 | 기본값 | 환경 변수 |
|------|--------|-----------|
| OFFSET | `/etc/y2k38_offset` | `Y2K38_KERNEL_OFFSET_FILE` |
| SIGHUP 리스트 | `/var/run/y2k38_sighup.list` | `Y2K38_SIGHUP_LIST_FILE` |

---

## Slide 3 — 연동

### 라이브러리

offsetctl도 **`liby2k38safe` 링크**:

- `y2k38_clock_compute_offset` / `save_offset_file`
- `y2k38_clock_set_system_utc` / `kernel_sec_for_utc`
- `y2k38_sighup_list_notify`

### Check · 앱 데몬

```
[offsetctl set-time / calibrate --notify]
        │
        ▼
 /etc/y2k38_offset 갱신
        │
        ▼
 SIGHUP → session_poll → 각 프로세스 OFFSET 재적용
        │
        ▼
 이후 y2k38_time() 이 동일한 UTC 공유
```

### 현장 시나리오

| 상황 | 권장 명령 |
|------|-----------|
| 보드 최초 시각 설정 | `set-time 2038-…` (또는 현재 신뢰 시각) |
| GPS/NTP로 epoch만 알 때 | `calibrate <epoch> --notify` |
| wrap 모델을 알 때 | `set-u32-wrap 1 --notify` |
| 파일만 바꿨을 때 | `sync` |

---

## Slide 4 — 사용 예 · 발표 요약 (선택)

### 복사용 예

```bash
# 상태 확인
y2k38_offsetctl show

# 신뢰 UTC(epoch)로 교정 + 전원 통지
y2k38_offsetctl calibrate 2147483748 --notify

# 벽시계 문자열로 시스템+OFFSET+통지
y2k38_offsetctl set-time 2038-01-19:03:15:48

# 전원에게만 다시 알림
y2k38_offsetctl notify
```

### 발표 요약

1. offsetctl = **운영자 도구**, Check = **자동 감시**
2. 모든 변경의 공통 키는 **`/etc/y2k38_offset`**
3. 프로세스 일치는 **`SIGHUP` 리스트** (`liby2k38safe` session)
4. `set-time`은 현장 투입·부팅 스크립트에 넣기 쉬운 API
