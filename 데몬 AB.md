데몬 A/B를 3스레드 구조로 바꿨습니다.

## 공통 구조

| 스레드 | 동작 |
|--------|------|
| **T1** | 시작 시각 저장 → 2초마다 `start` / `now` / `delta` 출력 |
| **T2** | 난수(1~10)초 `y2k38_nsleep_relative` → 진입/기상/차이 출력 → 난수(1~5)초 대기 후 반복 |
| **T3** | ns 단위 `clock_gettime` 측정 루프 → 3초마다 iterations / measured_ns 리포트 |

종료는 `volatile g_run` 플래그로 `while (g_run)`을 제어합니다.

```bash
kill -TERM <pid>   # 또는 Ctrl-C / kill <pid>
```

`kill -9`(SIGKILL)는 커널이 강제 종료하므로 **플래그로 처리할 수 없습니다**. TERM/INT만 graceful stop이 가능합니다.

---

## Daemon A

```bash
daemon_a [/var/log/events.log] [--offset-file PATH] [--auto-wrap]
```

- T1이 선택적으로 이벤트 로그에 `T1` 기록
- 출력 태그: `[A-T1]`, `[A-T2]`, `[A-T3]`

## Daemon B

```bash
daemon_b [/var/log/events.log /var/run/deltas.out] [--offset-file PATH]
```

- A와 동일한 3스레드
- T1에서 (로그 경로가 있으면) DELTA 재계산도 수행
- 출력 태그: `[B-T1]`, `[B-T2]`, `[B-T3]`

---

## 출력 예 (Daemon A)

```
[A-T1] start=2026-07-14:12:48:44  now=2026-07-14:12:48:46  delta=2 s
[A-T2] sleep_enter=...  sleep_sec=8
[A-T2] sleep_wake=...  enter=...  delta=8 s (wanted 8)
[A-T3] report: iterations=539654  measured_ns=163037700  ...
```

시간은 모두 `y2k38_time()` / `y2k38_difftime_sec()` 기준이라 Y2K38 OFFSET·wrap과 맞춰져 있습니다.