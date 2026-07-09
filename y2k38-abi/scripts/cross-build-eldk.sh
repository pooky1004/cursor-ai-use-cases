#!/bin/sh
# Cross-build Y2K38 ABI + daemons with ELDK 3.1.1 (or compatible) toolchain.
#
# Usage:
#   ./scripts/cross-build-eldk.sh
#   ELDK_ARCH=ppc_8xx ELDK_ROOT=/opt/eldk ./scripts/cross-build-eldk.sh
#   ./scripts/cross-build-eldk.sh install-sysroot
#   ./scripts/cross-build-eldk.sh stage
#
# Requires: ${ELDK_ARCH}-gcc on PATH (after sourcing ELDK env).

set -e

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
ELDK_ARCH="${ELDK_ARCH:-ppc_82xx}"
ELDK_ROOT="${ELDK_ROOT:-/opt/eldk}"
SYSROOT="${SYSROOT:-${ELDK_ROOT}/${ELDK_ARCH}}"
CROSS="${CROSS:-${ELDK_ARCH}-}"
ACTION="${1:-all}"

if ! command -v "${CROSS}gcc" >/dev/null 2>&1; then
    echo "error: ${CROSS}gcc not found on PATH" >&2
    echo "  source ELDK environment, or set CROSS= / ELDK_ARCH=" >&2
    exit 1
fi

if [ ! -d "$SYSROOT" ]; then
    echo "warning: SYSROOT=$SYSROOT does not exist (continuing)" >&2
fi

echo "==> CROSS=$CROSS"
echo "==> SYSROOT=$SYSROOT"
echo "==> ACTION=$ACTION"
echo "==> gcc: $(${CROSS}gcc -dumpmachine 2>/dev/null || true)"

cd "$ROOT"

case "$ACTION" in
    all|cross)
        make clean
        make cross CROSS="$CROSS" SYSROOT="$SYSROOT" LINK_STATIC=1
        echo "==> binaries:"
        ls -l daemons/daemon_a/daemon_a daemons/daemon_b/daemon_b \
            tools/y2k38_offsetctl lib/liby2k38safe.a
        if command -v file >/dev/null 2>&1; then
            file daemons/daemon_a/daemon_a || true
        fi
        ;;
    stage)
        make clean
        make stage CROSS="$CROSS" SYSROOT="$SYSROOT" LINK_STATIC=1 \
            STAGE="$ROOT/staging"
        echo "==> staging tree: $ROOT/staging"
        find "$ROOT/staging" -type f | head -50
        ;;
    install-sysroot)
        make install-sysroot CROSS="$CROSS" SYSROOT="$SYSROOT" LINK_STATIC=1
        ;;
    tarball)
        make stage CROSS="$CROSS" SYSROOT="$SYSROOT" LINK_STATIC=1 \
            STAGE="$ROOT/staging"
        TAR="$ROOT/y2k38-abi-${ELDK_ARCH}.tar.gz"
        tar -C "$ROOT/staging" -czf "$TAR" .
        echo "==> $TAR"
        ;;
    *)
        echo "usage: $0 [all|cross|stage|install-sysroot|tarball]" >&2
        exit 1
        ;;
esac

echo "==> done"
