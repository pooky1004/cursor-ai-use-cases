반영했습니다. 데몬 A/B의 각 스레드가 wrap을 감지하면 다른 프로세스에 SIGHUP을 보내고, 다른 데몬이 보낸 SIGHUP도 처리합니다.

## 동작

```
[자신 스레드] y2k38_time / poll_wrap
        │
        ├─ wrap 감지 → OFFSET += 2^32 (파일 저장)
        │            → on_local_wrap → SIGHUP-list notify (본인 제외)
        │
        └─ peer SIGHUP → session reload OFFSET
                       → on_peer_sighup 로그
```

## 데몬 A / B 공통

| 항목 | 내용 |
|------|------|
| **송신** | `y2k38_clock_on_wrap_callback` → `y2k38_sighup_list_notify(getpid())` |
| **수신** | session SIGHUP 핸들러 → OFFSET reload → `on_peer_sighup` |
| **스레드** | `thread_service_clock()` — T1/T2/T3 및 sleep 루프에서 wrap·SIGHUP poll |

wrap notify는 wrap count 기준으로 **한 번만** 보내 중복 SIGHUP을 막습니다.

## 로그 예

```
daemon_a: WRAP count=1 offset=4294967296 — SIGHUP peers
daemon_a: SIGHUP sent to 2 process(es)
daemon_b: received SIGHUP — reloaded offset=4294967296
```

## 라이브러리 추가 API

```c
y2k38_session_on_sighup_callback(fn, user);  /* peer SIGHUP 후 콜백 */
y2k38_session_poll_sighup();                 /* 스레드에서 명시적 drain */
```