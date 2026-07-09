# Toolchain ABI package notes (ELDK 3.1.1 / PPC32)

## Install layout inside ELDK sysroot

```
$SYSROOT/usr/include/y2k38/types.h
$SYSROOT/usr/include/y2k38/time.h
$SYSROOT/usr/include/y2k38/eventlog.h
$SYSROOT/usr/include/y2k38.h
$SYSROOT/usr/lib/liby2k38safe.a
$SYSROOT/usr/lib/liby2k38safe.so -> liby2k38safe.so.1
$SYSROOT/usr/lib/liby2k38safe.so.1 -> liby2k38safe.so.1.0.0
```

## What this ABI changes vs stock ELDK

| Item | Stock ELDK | Y2K38 ABI package |
|------|------------|-------------------|
| `time_t` | 32-bit signed | unchanged |
| Wall-clock in apps | `time_t` | `y2k38_time_t` (int64) |
| New symbols | — | `y2k38_*` in liby2k38safe |
| Compiler | stock | optional `-specs=y2k38.specs` |

Also ship env helpers that extend `CPATH` / `LIBRARY_PATH`.

## Steps

1. Build with `./scripts/cross-build-eldk.sh` or  
   `make CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx`
2. `make install-sysroot CROSS=... SYSROOT=...`  
   (headers, libs, `usr/lib/gcc-specs/y2k38.specs`)
3. Source `scripts/environment-setup-y2k38.sh`
4. Link apps with `-ly2k38safe` (or use staged static daemons)
5. On target: calibrate `/etc/y2k38_offset` if kernel clock wrapped  
   (`docs/KERNEL_OFFSET.md`)

See also `docs/CROSS_BUILD.md`.
