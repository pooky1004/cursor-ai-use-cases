# Board cross-build (ELDK 3.1.1 / PPC32)

## Prerequisites

- ELDK 3.1.1 (or compatible) installed, e.g. `/opt/eldk`
- Target arch prefix on `PATH`, e.g. `ppc_82xx-gcc`
- Matching sysroot: `/opt/eldk/ppc_82xx` (or `ppc_8xx` / `ppc_4xx`)

```bash
# Typical ELDK setup (vendor-specific)
export PATH=/opt/eldk/usr/bin:$PATH
# or: . /opt/eldk/eldk_init ppc_82xx
```

## One-shot cross build

```bash
cd /path/to/y2k38-abi
export ELDK_ARCH=ppc_82xx
export ELDK_ROOT=/opt/eldk
./scripts/cross-build-eldk.sh          # build
./scripts/cross-build-eldk.sh stage    # $PWD/staging tree
./scripts/cross-build-eldk.sh tarball  # y2k38-abi-ppc_82xx.tar.gz
./scripts/cross-build-eldk.sh install-sysroot  # into $SYSROOT/usr
```

Equivalent Make:

```bash
make cross CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx LINK_STATIC=1
make stage CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
make install-sysroot CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
```

`LINK_STATIC=1` (default) embeds `liby2k38safe.a` into daemons/tools so the board
does not need `LD_LIBRARY_PATH` for those binaries. Shared library is still
built for other apps: `-ly2k38safe`.

## Deploy

```bash
./scripts/deploy-board.sh                 # print instructions
./scripts/deploy-board.sh root@192.168.0.10:/
```

On the board after first deploy:

```bash
# If wall clock is still correct (pre-wrap): leave OFFSET 0
y2k38_offsetctl show

# If kernel time_t has wrapped — calibrate from trusted UTC (NTP/GPS/host):
y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset

# Or known single unsigned wrap:
y2k38_offsetctl set-u32-wrap 1 --file /etc/y2k38_offset
```

Boot hook: `scripts/board-init-y2k38.sh` (see comments inside).

## Compiler wrapper (optional)

```bash
. scripts/environment-setup-y2k38.sh
ppc_82xx-gcc -specs=$SYSROOT/usr/lib/gcc-specs/y2k38.specs ...
```

## Verify ELF

```bash
ppc_82xx-readelf -h daemons/daemon_a/daemon_a
# Machine: PowerPC, Class: ELF32
ppc_82xx-nm daemons/daemon_a/daemon_a | grep y2k38_time
```
