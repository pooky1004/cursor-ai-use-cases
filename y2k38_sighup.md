요청하신 4가지를 반영했습니다. `make check` / `make check-offset` 통과했습니다.

## 동작 요약

### 1. API 호출 시 OFFSET 반영 확인 (ready flag)

`y2k38_time()` / `y2k38_gettimeofday()` 진입 시 `y2k38_session_ensure()`:

| 상태 | 동작 |
|------|------|
| **flag 반영됨** | 그대로 진행 |
| **미반영** | SIGHUP 핸들러 등록 → SIGHUP-list에 `이름 PID` 추가 → `/etc/y2k38_offset` 로드 → flag 설정 → auto-wrap ON |

이후 매 `y2k38_time()`에서 wrap 감지 시 OFFSET += `2^32`.

### 2. 프로세스 종료 시

`y2k38_session_exit()` (`atexit` 포함):

- SIGHUP 핸들러 해제
- SIGHUP-list에서 본인 PID 삭제

### 3. `y2k38_offsetctl set-time` / `sync`

- 시스템 시계 (`settimeofday` / `date`) + HW (`hwclock -w`)
- `/etc/y2k38_offset` 갱신
- **SIGHUP-list의 모든 PID에 SIGHUP** (본인 제외)
- `sync`도 동일하게 list 기반 notify

### 4. `daemon_y2k38_check` 추가

```bash
daemon_y2k38_check --offset-file /etc/y2k38_offset
```

- list에 자기 자신 등록 (`daemon_y2k38_check <pid>`)
- wrap까지 남은 시간에 따라 폴링 간격 조절 (멀면 드묾, 가까우면 자주)
- wrap 시 OFFSET 파일 갱신 + list에 SIGHUP (**본인 제외**)
- 자신이 SIGHUP 받으면 OFFSET reload

---

## 주요 경로 / API

| 항목 | 값 |
|------|-----|
| SIGHUP-list | `/var/run/y2k38_sighup.list` (`Y2K38_SIGHUP_LIST_FILE`) |
| Offset | `/etc/y2k38_offset` |

```c
y2k38_session_init("my_app");   /* preferred at main() */
y2k38_session_is_ready();
y2k38_session_exit();
y2k38_sighup_list_notify(getpid());  /* exclude self */
```

List 형식:

```
# y2k38 SIGHUP subscriber list — name pid
daemon_y2k38_check 1234
daemon_a 1235
daemon_b 1236
```

---

## 보드에서

```bash
# Check 데몬 먼저
daemon_y2k38_check --offset-file /etc/y2k38_offset &

daemon_a /var/log/events.log --offset-file /etc/y2k38_offset --auto-wrap &
daemon_b /var/log/events.log /var/run/deltas.out 10 --offset-file /etc/y2k38_offset &

# 시간 설정 → list 전원에 SIGHUP
y2k38_offsetctl set-time 2038-01-19:03:15:48 --file /etc/y2k38_offset
y2k38_offsetctl sync --file /etc/y2k38_offset
```

`board-init-y2k38.sh`도 Check 데몬을 먼저 기동하도록 수정했습니다.