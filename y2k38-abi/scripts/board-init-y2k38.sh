#!/bin/sh
# Board bring-up for Y2K38-safe daemons (run once at boot or from rcS).
#
# 1) Apply /etc/y2k38_offset if present
# 2) Optionally start daemon_a / daemon_b
#
# Install: copy to /etc/init.d/y2k38 or source from /etc/rc.local
#
# Environment overrides:
#   Y2K38_KERNEL_OFFSET_FILE  offset path (default /etc/y2k38_offset)
#   Y2K38_EVENT_LOG           daemon A log (default /var/log/y2k38_events.log)
#   Y2K38_DELTA_OUT           daemon B out (default /var/log/y2k38_deltas.out)
#   Y2K38_START_DAEMONS=1     start A/B in background

OFFSET_FILE="${Y2K38_KERNEL_OFFSET_FILE:-/etc/y2k38_offset}"
EVENT_LOG="${Y2K38_EVENT_LOG:-/var/log/y2k38_events.log}"
DELTA_OUT="${Y2K38_DELTA_OUT:-/var/log/y2k38_deltas.out}"

export Y2K38_KERNEL_OFFSET_FILE="$OFFSET_FILE"

if [ -x /usr/bin/y2k38_offsetctl ]; then
    /usr/bin/y2k38_offsetctl show --file "$OFFSET_FILE" || true
    /usr/bin/y2k38_offsetctl reload --file "$OFFSET_FILE" 2>/dev/null || true
elif [ -f "$OFFSET_FILE" ]; then
    echo "y2k38: offset file present at $OFFSET_FILE"
fi

if [ "${Y2K38_START_DAEMONS:-0}" = "1" ]; then
    mkdir -p "$(dirname "$EVENT_LOG")" "$(dirname "$DELTA_OUT")"
    if [ -x /usr/bin/daemon_a ]; then
        /usr/bin/daemon_a "$EVENT_LOG" --offset-file "$OFFSET_FILE" &
        echo "y2k38: daemon_a pid $!"
    fi
    if [ -x /usr/bin/daemon_b ]; then
        /usr/bin/daemon_b "$EVENT_LOG" "$DELTA_OUT" 5 \
            --offset-file "$OFFSET_FILE" &
        echo "y2k38: daemon_b pid $!"
    fi
fi
