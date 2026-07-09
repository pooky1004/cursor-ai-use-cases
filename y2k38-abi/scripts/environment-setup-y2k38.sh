#!/bin/sh
# Source this file after ELDK environment is set, e.g.:
#   . /opt/eldk/eldk_init ppc_82xx   # vendor-specific
#   . /path/to/y2k38-abi/scripts/environment-setup-y2k38.sh
#
# Then: make cross   OR   ./scripts/cross-build-eldk.sh

# Adjust to your board prefix (ppc_82xx, ppc_8xx, ppc_4xx, ...)
ELDK_ARCH="${ELDK_ARCH:-ppc_82xx}"
ELDK_ROOT="${ELDK_ROOT:-/opt/eldk}"
SYSROOT="${SYSROOT:-${ELDK_ROOT}/${ELDK_ARCH}}"

# Location of installed Y2K38 ABI (after: make install-sysroot)
Y2K38_PREFIX="${Y2K38_PREFIX:-${SYSROOT}/usr}"
Y2K38_SPECS="${Y2K38_SPECS:-${Y2K38_PREFIX}/lib/gcc-specs/y2k38.specs}"

export CROSS="${CROSS:-${ELDK_ARCH}-}"
export CC="${CROSS}gcc"
export AR="${CROSS}ar"
export RANLIB="${CROSS}ranlib"
export STRIP="${CROSS}strip"
export SYSROOT
export ELDK_ARCH ELDK_ROOT Y2K38_PREFIX
export CPATH="${Y2K38_PREFIX}/include${CPATH:+:$CPATH}"
export LIBRARY_PATH="${Y2K38_PREFIX}/lib${LIBRARY_PATH:+:$LIBRARY_PATH}"
export LD_LIBRARY_PATH="${Y2K38_PREFIX}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export Y2K38_KERNEL_OFFSET_FILE="${Y2K38_KERNEL_OFFSET_FILE:-/etc/y2k38_offset}"

# Convenience wrappers
y2k38_gcc() {
    if [ -f "$Y2K38_SPECS" ]; then
        "${CC}" --sysroot="${SYSROOT}" -specs="$Y2K38_SPECS" \
            -isystem "${Y2K38_PREFIX}/include" -L"${Y2K38_PREFIX}/lib" "$@"
    else
        "${CC}" --sysroot="${SYSROOT}" \
            -isystem "${Y2K38_PREFIX}/include" -L"${Y2K38_PREFIX}/lib" "$@"
    fi
}

echo "Y2K38 ABI env: CROSS=$CROSS SYSROOT=$SYSROOT Y2K38_PREFIX=$Y2K38_PREFIX"
echo "  helper: y2k38_gcc ... -ly2k38safe"
echo "  offset file: $Y2K38_KERNEL_OFFSET_FILE"
