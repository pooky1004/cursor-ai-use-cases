# Kernel clock offset recovery (Y2K38)

## Why offset is needed

`liby2k38safe` stores and computes times in **64-bit** (`y2k38_time_t`). That alone
fixes durable logs and deltas **for application data**.

It does **not** invent a correct wall clock if the **kernel/RTC** already returns a
wrapped signed 32-bit `time_t`. In that case:

```
real_utc = sign_extend(kernel_time_t) + OFFSET
```

`OFFSET` is persisted (default `/etc/y2k38_offset`) and loaded by daemons at start.

## File format

```
# y2k38 kernel clock offset (seconds)
# real_utc = sign_extend(kernel_time_t) + OFFSET
OFFSET 4294967296
```

A single decimal line is also accepted.

Environment override: `Y2K38_KERNEL_OFFSET_FILE=/path/to/file`

## API (liby2k38safe)

| Function | Role |
|----------|------|
| `y2k38_time()` | recovered UTC (`raw + offset`) |
| `y2k38_time_kernel_raw()` | widened kernel seconds, **no** offset |
| `y2k38_clock_set_kernel_offset` / `get` | in-process offset |
| `y2k38_clock_compute_offset(true, raw)` | `true - raw` |
| `y2k38_clock_u32_wrap_offset(n)` | `n * 2^32` |
| `y2k38_clock_load/save_offset_file` | persist |
| `y2k38_clock_apply_offset_default` | load path / env / `/etc/y2k38_offset` |
| `y2k38_clock_set_mock_kernel` | tests only |

## Scenarios

### A. Pre-2038, kernel still correct

Leave `OFFSET 0` (or omit file). Apps only need 64-bit **storage/math** for future
schedules past 2038.

### B. Single unsigned wrap (common mental model)

If the hardware second counter is treated as uint32 and has wrapped once since
the Unix epoch interpretation, a starting offset of `2^32 = 4294967296` maps
the residual back toward correct UTC **when** that model matches the board.

```bash
y2k38_offsetctl set-u32-wrap 1 --file /etc/y2k38_offset
```

Prefer **calibrate** (scenario C) when a trusted absolute time is available —
it does not assume the wrap model.

**Set post-2038 wall clock** (scenario E — sets kernel + OFFSET file):

```bash
# root required: calendar UTC + /etc/y2k38_offset
y2k38_offsetctl set-time 2038-01-19:03:15:48 --file /etc/y2k38_offset --notify
y2k38_offsetctl get-time
y2k38_offsetctl show
```

`set-time` writes the int32 kernel residue via `settimeofday`/`date`, saves
`OFFSET = target - kernel_raw`, and optionally sends SIGHUP to daemons.
All y2k38 processes sharing `/etc/y2k38_offset` reload on the next
`y2k38_time()` (mtime poll) or on SIGHUP.

### C. Calibrate from trusted UTC (recommended on board)

Trusted source examples: GPS UART, NTP on a gateway, staging host `date +%s`
at install time, lab clock.

```bash
# Example: trusted UTC seconds = 2147483748
# Kernel currently reports wrapped signed residual; offsetctl reads kernel.
y2k38_offsetctl calibrate 2147483748 --file /etc/y2k38_offset
y2k38_offsetctl show
```

Host demo with emulated kernel residual:

```bash
./tools/y2k38_offsetctl calibrate 2147483748 \
  --file /tmp/y2k38_off.conf \
  --mock-kernel -2147483548
./examples/fixed/kernel_offset_demo
```

### D. Daemons after wrap

```bash
# both daemons load the same offset file
daemon_a /var/log/y2k38_events.log --offset-file /etc/y2k38_offset
daemon_b /var/log/y2k38_events.log /var/log/y2k38_deltas.out 5 \
  --offset-file /etc/y2k38_offset
```

`--mock-kernel` is for lab only. `--mock-now` forces absolute UTC and **skips**
kernel+offset (unit tests / dry runs).

Use `--no-offset-file` when intentionally ignoring `/etc/y2k38_offset`.

## Operational checklist

1. Cross-build & deploy (`docs/CROSS_BUILD.md`)
2. Confirm `y2k38_offsetctl show` — `utc_now` matches trusted clock
3. If not: `calibrate` and write `/etc/y2k38_offset`
4. Start A/B (or `board-init-y2k38.sh` with `Y2K38_START_DAEMONS=1`)
5. Confirm Daemon B deltas for `FUT2038` / far schedules are positive & sane
6. Re-calibrate after RTC battery events or known time jumps

## Limits

- Offset is a **userspace policy**. It does not patch the kernel timestore.
- `select`/`nanosleep` absolute kernel timers may still be 32-bit; prefer
  relative waits derived from `y2k38_difftime_sec` with clamping.
- If multiple processes disagree on OFFSET, clocks diverge — keep **one file**,
  root-owned, updated by a single calibrator (`y2k38_offsetctl` or NTP agent).
