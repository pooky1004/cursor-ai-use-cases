#!/bin/sh
# Deploy staged Y2K38 tree to a board (or print copy instructions).
#
# Usage:
#   ./scripts/cross-build-eldk.sh stage
#   ./scripts/deploy-board.sh root@192.168.1.10
#   ./scripts/deploy-board.sh root@board:/
#   DEST=root@board:/opt ./scripts/deploy-board.sh
#
# Uses scp/rsync if available. Static-linked daemons need only binaries + offset file.

set -e

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
STAGE="${STAGE:-$ROOT/staging}"
DEST="${1:-${DEST:-}}"

if [ ! -d "$STAGE/usr" ]; then
    echo "error: missing $STAGE — run: ./scripts/cross-build-eldk.sh stage" >&2
    exit 1
fi

if [ -z "$DEST" ]; then
    echo "Staged payload ready at: $STAGE"
    echo
    echo "Manual deploy example:"
    echo "  scp -r $STAGE/usr/bin/daemon_a $STAGE/usr/bin/daemon_b \\"
    echo "         $STAGE/usr/bin/y2k38_offsetctl root@BOARD:/usr/bin/"
    echo "  scp $STAGE/usr/lib/liby2k38safe.so.1.0.0 root@BOARD:/usr/lib/"
    echo "  ssh root@BOARD 'cd /usr/lib && ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1'"
    echo "  scp $STAGE/etc/y2k38_offset root@BOARD:/etc/y2k38_offset"
    echo
    echo "Or: $0 root@BOARD:/"
    exit 0
fi

# Normalize DEST: host or host:/path
case "$DEST" in
    *:*) remote="$DEST" ;;
    *)   remote="$DEST:/" ;;
esac

echo "==> deploy $STAGE -> $remote"

if command -v rsync >/dev/null 2>&1; then
    rsync -av --checksum \
        "$STAGE/usr/" "${remote}usr/" \
        "$STAGE/etc/" "${remote}etc/"
else
    scp -r "$STAGE/usr" "$STAGE/etc" "$remote"
fi

echo "==> on board, set post-2038 time or calibrate if kernel clock is wrapped:"
echo "  y2k38_offsetctl set-time YYYY-MM-DD:hh:mm:ss --file /etc/y2k38_offset --notify"
echo "  y2k38_offsetctl get-time"
echo "  y2k38_offsetctl calibrate <true_utc_epoch> --file /etc/y2k38_offset --notify"
echo "  y2k38_offsetctl show"
echo "==> done"
