# Y2K38 Application-Level Fix for 32-bit PPC (ELDK 3.1.1)

Application-level workaround for the Year 2038 problem when `time_t` is locked to 32-bit signed on PowerPC Linux with ELDK 3.1.1 (no glibc `_TIME_BITS=64` option).

## Goals

1. Define a **new ABI** with `y2k38_time_t` (64-bit signed integer) and matching time helpers.
2. Integrate that ABI into the **ELDK/GCC toolchain** (sysroot + specs + cross scripts).
3. Ship **`liby2k38safe`** (storage/math + **kernel offset recovery**).
4. **Daemon A / Daemon B** using the library.
5. **Broken vs fixed** examples, plus kernel-offset demo / `y2k38_offsetctl`.

## Docs

| Doc | Topic |
|-----|--------|
| [`docs/ABI_DESIGN.md`](docs/ABI_DESIGN.md) | ABI + toolchain design |
| [`docs/CROSS_BUILD.md`](docs/CROSS_BUILD.md) | ELDK cross-build / stage / deploy |
| [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md) | Wrap recovery, calibrate, `/etc/y2k38_offset` |
| [`Y2K38_issue.md`](Y2K38_issue.md) | Full issue write-up (KO) |

## Layout

```
include/y2k38/   lib/          liby2k38safe
daemons/         tools/        daemon_a, daemon_b, y2k38_offsetctl
examples/broken  examples/fixed
scripts/         cross-build-eldk.sh, deploy-board.sh, board-init-y2k38.sh
toolchain/abi/   y2k38.specs
```

## Quick build (host)

```bash
make
make check
make check-offset
./examples/broken/y2k38_broken
./examples/fixed/y2k38_fixed
./examples/fixed/kernel_offset_demo
```

Daemons (demo):

```bash
./daemons/daemon_a/daemon_a /tmp/events.log --mock-now 2147483600 --once --no-offset-file
./daemons/daemon_b/daemon_b /tmp/events.log /tmp/deltas.out --mock-now 2147483600 --once --no-offset-file
```

## Cross-build (ELDK 3.1.1)

```bash
export ELDK_ARCH=ppc_82xx ELDK_ROOT=/opt/eldk
. scripts/environment-setup-y2k38.sh   # optional
./scripts/cross-build-eldk.sh          # or: stage | tarball | install-sysroot
./scripts/deploy-board.sh root@board:/
```

See [`docs/CROSS_BUILD.md`](docs/CROSS_BUILD.md).

## Kernel offset (post-wrap wall clock)

```bash
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset
daemon_a /var/log/y2k38_events.log --offset-file /etc/y2k38_offset
```

See [`docs/KERNEL_OFFSET.md`](docs/KERNEL_OFFSET.md).
