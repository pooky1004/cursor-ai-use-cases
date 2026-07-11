구현을 완료했습니다. `y2k38_offsetctl`로 **2038 이후 시각 설정**과 **전 프로세스 OFFSET 통일**이 가능합니다.

## 핵심 아이디어

32-bit 커널에는 `settimeofday`로 **int32에 들어가는 잔여 초**만 설정하고, 나머지는 `/etc/y2k38_offset`의 **OFFSET**으로 보정합니다.

```
target_utc = 2147483748  (2038-01-19 03:15:48)
kernel_set = (int32_t)target  →  -2147483548   ← settimeofday/date
OFFSET     = target - kernel  →   4294967296   ← /etc/y2k38_offset
y2k38_time() = kernel + OFFSET → 2147483748     ← 앱이 보는 UTC
```

---

## 새 명령어 (`y2k38_offsetctl`)

| 명령 | 역할 |
|------|------|
| **`set-time <epoch>`** | `settimeofday(2)` → 실패 시 `date -s @<kernel_sec>` → OFFSET 저장 → (선택) 데몬 알림 |
| **`reload`** | 현재 프로세스가 `/etc/y2k38_offset` 다시 읽기 |
| **`sync`** | `reload` + `--notify`와 동일한 알림 |
| **`notify`** | `daemon_a`/`daemon_b`에 SIGHUP 전송 |
| **`--notify`** | `set` / `calibrate` / `set-time` 후 데몬에 SIGHUP |

### 보드에서 2038 이후 시각 설정 (root)

```bash
# GPS/NTP에서 받은 int64 epoch
y2k38_offsetctl set-time 2147483748 \
  --file /etc/y2k38_offset \
  --notify

y2k38_offsetctl show
```

### wrap만 보정 (커널 시계는 그대로)

```bash
y2k38_offsetctl calibrate 2147483748 \
  --file /etc/y2k38_offset \
  --notify
```

---

## 다중 프로세스 시간 통일

세 가지 메커니즘이 함께 동작합니다.

1. **공유 파일** — `/etc/y2k38_offset` (또는 `$Y2K38_KERNEL_OFFSET_FILE`)
2. **mtime 자동 재로드** — `y2k38_time()` / `y2k38_gettimeofday()` 호출 시 파일 변경 감지 후 OFFSET 재적용
3. **SIGHUP** — `daemon_a` / `daemon_b`가 SIGHUP 수신 시 `reload`

```
y2k38_offsetctl set-time ... --notify
        │
        ├─► /etc/y2k38_offset (atomic write: .tmp → rename)
        │
        ├─► SIGHUP → daemon_a / daemon_b (즉시 reload)
        │
        └─► 다른 y2k38 앱 → 다음 y2k38_time() 시 mtime poll로 reload
```

앱/데몬은 기동 시 `y2k38_clock_apply_offset_default()`만 호출하면 자동으로 공유 경로가 등록됩니다.

---

## 라이브러리 API 추가

| API | 설명 |
|-----|------|
| `y2k38_clock_kernel_sec_for_utc()` | settimeofday용 int32 잔여 초 |
| `y2k38_clock_set_system_utc()` | 커널 시계 설정 + in-process OFFSET |
| `y2k38_clock_set_system_and_offset()` | 위 + 파일 저장 + 공유 reload 등록 |
| `y2k38_clock_reload_offset_default()` | 파일에서 OFFSET 강제 재로드 |
| `y2k38_clock_resolve_offset_path()` | 경로 해석 (CLI → env → 기본값) |

---

## 변경된 파일

- `lib/y2k38_offset.c` — atomic 저장, mtime poll, `set_system_and_offset`
- `lib/y2k38_time.c` — `settimeofday`/`date`/`hwclock`, mtime poll 연동
- `tools/y2k38_offsetctl.c` — `set-time`, `reload`, `sync`, `notify`
- `daemons/daemon_a/daemon_a.c`, `daemon_b/daemon_b.c` — SIGHUP 핸들러
- `docs/KERNEL_OFFSET.md`, `scripts/deploy-board.sh`, `scripts/board-init-y2k38.sh`

`make check` / `make check-offset` 통과했습니다.

**참고:** `set-time`은 **root 권한**이 필요합니다 (`settimeofday` 또는 `date`). 보드에서 테스트할 때는 위 명령을 root로 실행하세요.
 