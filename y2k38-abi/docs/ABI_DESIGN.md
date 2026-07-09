# Y2K38 ABI Design & ELDK 3.1.1 Toolchain Integration

Target: **32-bit PowerPC Linux**, **ELDK 3.1.1**, `time_t` = `long` = **32-bit signed**, no glibc option to widen `time_t`.

Problem epoch: **2038-01-19 03:14:07 UTC** = `0x7FFFFFFF`. One second later, signed 32-bit `time_t` becomes negative (`0x80000000`) and libc time APIs misbehave (localtime, mktime, timers, `ctime`, timeout math).

This document defines an **application / userspace ABI** that introduces `y2k38_time_t` (64-bit) without requiring a 64-bit kernel `time_t` or a full glibc rebuild with incompatible `time_t`. Kernel `time_t`-based syscalls remain 32-bit on this platform; the ABI wraps and extends them at the **userspace library** boundary.

---

## 1. ABI summary

### 1.1 Core types (new ABI)

| Symbol | Width | Meaning |
|--------|-------|---------|
| `y2k38_time_t` | **signed 64-bit** | Seconds since Unix epoch (UTC), range ≫ 2038 |
| `y2k38_suseconds_t` | signed 32-bit | Microseconds [0, 999999] |
| `struct y2k38_timeval` | 64 + 32 (+ pad) | Absolute time with sub-second |
| `struct y2k38_timespec` | 64 + 32 (+ pad) | Absolute time with nanoseconds |
| `struct y2k38_tm` | same fields as `struct tm` | Broken-down calendar time |

ABI rule: **all APIs that accept or return wall-clock time across shared-library / daemon boundaries use `y2k38_*` types**, never raw `time_t`.

### 1.2 Why a separate type (not just `int64_t`)

1. **Semantic clarity** — review, static analysis, and ABI docs know the unit is “epoch seconds”.
2. **Stable layout** — `struct y2k38_timeval` packing can be fixed and versioned independent of host `timeval`.
3. **Name mangling / headers** — allows `#include <y2k38/time.h>` without colliding with `<time.h>`.
4. **Future kernel widen** — if a later kernel exports 64-bit time syscalls, only the library backend changes; app ABI stays.

### 1.3 ABI versioning

```c
#define Y2K38_ABI_VERSION_MAJOR 1
#define Y2K38_ABI_VERSION_MINOR 0
```

Shared library soname: `liby2k38safe.so.1`

Binary modules (daemons, plugins) compiled against major 1 must not link against major 2 without recompile.

### 1.4 Calling convention & packing (PPC32 EABI)

- Scalar `y2k38_time_t` (`long long`): passed/returned per **PowerPC EABI** — typically in `r3`/`r4` register pair (or stack for some edges). ELDK GCC already does this for `long long`.
- Structures: `#pragma pack` **not** used; use natural alignment:
  - `y2k38_time_t` aligned to 8 bytes
  - Following 32-bit fields keep 4-byte alignment; pad to 8 where the next `y2k38_time_t` appears
- Documented layout (little-endian and big-endian PPC both):

```
struct y2k38_timeval {
    y2k38_time_t  tv_sec;   /* offset 0, size 8 */
    int32_t       tv_usec;  /* offset 8, size 4 */
    int32_t       __pad;    /* offset 12, size 4  — keep sizeof == 16 */
};
```

`sizeof(struct y2k38_timeval) == 16`, `sizeof(y2k38_time_t) == 8` on this ABI. Verify with `scripts/abi_check.c`.

### 1.5 Forbidden patterns across the ABI boundary

```c
/* WRONG — 32-bit time_t stores */
time_t t = time(NULL);
fprintf(f, "%ld\n", (long)t);

/* WRONG — signed overflow in delta */
time_t future = ...;
time_t now = time(NULL);
long delta = future - now;   /* undefined / wrap after 2038 */

/* RIGHT */
y2k38_time_t t = y2k38_time(NULL);
y2k38_time_t delta = y2k38_difftime_sec(future, now);
```

Serialization: always write **decimal ASCII** of the full 64-bit value, or a fixed **8-byte big-endian** blob with an explicit format tag (`Y2K38T\0\1`). Never `fwrite(&time_t, 4, 1, fp)` for wall clock.

---

## 2. Mapping to kernel / libc on 32-bit ELDK

On classic 32-bit PowerPC Linux (2.4/2.6 era) and ELDK userland:

| Need | Reality | Strategy |
|------|---------|----------|
| `time()` | returns 32-bit `time_t` | Call syscall / libc; **sign-extend** to `y2k38_time_t` while wall clock &lt; 2^31. After overflow, host RTC/kernel may already be wrong — use `CLOCK_REALTIME` via `gettimeofday` then extend, or an external **trusted clock** (NTP file, RTC drivers that expose 64-bit, GPS). |
| `gettimeofday` | `timeval.tv_sec` is 32-bit | Same: widen to `y2k38_timeval`. |
| Timestamps stored in files | app-controlled | Use `y2k38_time_t` formatting APIs. |
| `select` / `poll` timeouts | relative ms/us often `int` | Prefer relative timeouts from `y2k38` deltas clamped safely. |
| `timer_settime` absolute | often 32-bit | Convert relative fires from 64-bit schedule using “now + delta” with clamp. |

**Important:** Pure userspace cannot fix a kernel that returns wrapped `time_t` after 2038. This project assumes one of:

1. Device stays within signed 32-bit RTC and you need correct **arithmetic / storage** of *designed* future event times (schedules past 2038) **before and after** wrap for *app data*, or
2. You use **offset recovery** so UTC is reconstructed in userspace:  
   `real_utc = sign_extend(kernel_time_t) + OFFSET`  
   (see `docs/KERNEL_OFFSET.md`, tool `y2k38_offsetctl`, file `/etc/y2k38_offset`).

`liby2k38safe` implements both:

- **Widen mode** (default): sign-extend libc `time_t` / `gettimeofday` when value is still valid (`OFFSET=0`).
- **Epoch offset mode**: `y2k38_clock_load_offset_file()` / `y2k38_clock_apply_offset_default()`.

Board packaging and ELDK steps: `docs/CROSS_BUILD.md`.

Daemons use **storage and arithmetic only on `y2k38_time_t`**, and load the same offset file at startup.

---

## 3. Adding the ABI to the ELDK 3.1.1 toolchain

There is **no** stock ELDK switch `"--with-y2k38-time"`. You add a **userspace ABI package** that the cross compiler finds via headers and libraries, optionally with a dedicated **sysroot variant**.

### 3.1 Recommended approach (practical for ELDK)

Do **not** rewrite GCC’s `time_t`. Keep libc ABI intact. Add a parallel ABI:

```
$SYSROOT/usr/include/y2k38/...
$SYSROOT/usr/lib/liby2k38safe.so*
$SYSROOT/usr/lib/liby2k38safe.a
```

Compiler flags for Y2K38-safe apps:

```bash
ppc_82xx-gcc -I$SYSROOT/usr/include \
  -o daemon_a daemon_a.c -L$SYSROOT/usr/lib -ly2k38safe
```

Optional convenience wrapper (install as `ppc_82xx-y2k38-gcc`):

```bash
#!/bin/sh
exec ppc_82xx-gcc -isystem "$ELDK_Y2K38_INCLUDE" "$@" -L"$ELDK_Y2K38_LIB" -ly2k38safe
```

This is the **lowest-risk** integration: no glibc rebuild, no GCC patch, works with ELDK 3.1.1 today.

### 3.2 Deeper integration: custom “ABI flavor” / sysroot

If you want a toolchain profile named like a multilib:

1. **Clone sysroot**

   ```bash
   cp -a /opt/eldk/ppc_82xx /opt/eldk/ppc_82xx_y2k38
   ```

2. **Install ABI headers & library** into the clone (`make install PREFIX=/opt/eldk/ppc_82xx_y2k38/usr`).

3. **Spec file** (`toolchain/abi/y2k38.specs`) so GCC always searches the right paths:

   ```
   *cpp_unique_options:
   + -isystem /opt/eldk/ppc_82xx_y2k38/usr/include

   *link:
   + -L/opt/eldk/ppc_82xx_y2k38/usr/lib -ly2k38safe
   ```

4. Invoke:

   ```bash
   ppc_82xx-gcc -specs=/opt/eldk/usr/lib/y2k38.specs ...
   ```

Or set `LIBRARY_PATH` / `CPATH` in `environment-setup-y2k38.sh` (see `scripts/environment-setup-y2k38.sh`).

### 3.3 Optional: patch GCC / newlib-style “builtin” (heavy)

Only if you must make `y2k38_time_t` a **compiler built-in typedef** visible without `-I`:

1. Add `gcc/config/rs6000/linux-y2k38.h` that defines:

   ```c
   #undef TARGET_OS_CPP_BUILTINS
   #define TARGET_OS_CPP_BUILTINS() do { \
     LINUX_TARGET_OS_CPP_BUILTINS(); \
     builtin_define ("__Y2K38_ABI__=1"); \
     builtin_define ("__SIZEOF_Y2K38_TIME_T__=8"); \
   } while (0)
   ```

2. Register a new target tarball or configure option `--enable-y2k38-abi` that installs a fixed `include/y2k38` into the toolchain `sys-include`.

3. Rebuild ELDK GCC — **high cost**, rarely needed if headers are installed in sysroot.

Sketch patch: `toolchain/patches/gcc-eldk-y2k38-define.patch`.

### 3.4 Optional: glibc “compat” symbols (not changing `time_t`)

You can add **new** ELF symbols next to glibc without replacing `time`:

```
y2k38_time
y2k38_gettimeofday
y2k38_gmtime_r
y2k38_localtime_r
y2k38_mktime
y2k38_strftime
y2k38_difftime_sec
```

Shipped in `liby2k38safe.so` is enough; injecting into `libc.so` requires a rebuilt glibc — avoid for ELDK stability.

### 3.5 Build & install into ELDK (step-by-step)

```bash
# On build host with ELDK installed
export CROSS=ppc_82xx-
export SYSROOT=/opt/eldk/ppc_82xx   # or ppc_8xx / ppc_4xx — match board

cd /path/to/y2k38-abi
make clean
make CROSS=${CROSS} SYSROOT=${SYSROOT} all

# Install ABI into sysroot
make CROSS=${CROSS} SYSROOT=${SYSROOT} PREFIX=${SYSROOT}/usr install

# Verify types on target or with qemu-user if available
${CROSS}gcc -I${SYSROOT}/usr/include scripts/abi_check.c \
  -L${SYSROOT}/usr/lib -ly2k38safe -o /tmp/abi_check
```

Target runtime: copy `liby2k38safe.so.1` to device `/usr/lib` (or link statically with `-static` / `liby2k38safe.a`).

### 3.6 ABI compliance checklist for application teams

- [ ] No `time_t` / `struct timeval.tv_sec` stored in durable logs for wall clock
- [ ] All cross-process message fields use `int64_t` / `y2k38_time_t`
- [ ] Subtraction via `y2k38_difftime_sec` / `y2k38_timeval_diff`
- [ ] Parsing of external schedules uses `y2k38_parse_iso8601` or `strtoll`
- [ ] Linked with `-ly2k38safe`, soname major matches
- [ ] Unit test runs simulated “now” of `0x7FFFFFFF - 10` and `0x7FFFFFFF + 100`

---

## 4. Interaction with Daemons A & B

**Daemon A** appends event lines: `EVENT <id> <y2k38_time_t> <msg>`  
If it used `%ld` of `time_t`, after 2038 logs show negative times and Daemon B’s delta becomes nonsense.

**Daemon B** reads those timestamps as `y2k38_time_t`, subtracts current `y2k38_time()`, writes `DELTA <id> <seconds>`.  
32-bit `future - now` overflows when both are near `INT32_MAX` or when `future` was stored truncated.

Both daemons link `liby2k38safe` only.

---

## 5. Test vectors

| Case | Input | Expected |
|------|-------|----------|
| Pre-2038 now | `0x7FFFFFFF - 3600` | Format / delta OK |
| Exact max | `0x7FFFFFFF` | Still representable as `y2k38_time_t` |
| Post wrap as schedule | `0x80000000LL` (= 2038-01-19 03:14:08) | Positive delta math vs earlier now |
| Far future | `4102444800` (2100-01-01) | Stored and diff correct as int64 |

Broken example intentionally casts far-future to `time_t` to show truncation/overflow.
