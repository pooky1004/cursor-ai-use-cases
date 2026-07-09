/*
 * Durable event log helpers for Daemon A / B (Y2K38-safe).
 */
#ifndef Y2K38_EVENTLOG_H
#define Y2K38_EVENTLOG_H

#include <stdio.h>

#include <y2k38/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Y2K38_EVENT_MSG_MAX 256
#define Y2K38_EVENT_ID_MAX  64

struct y2k38_event {
    char          id[Y2K38_EVENT_ID_MAX];
    y2k38_time_t  when;   /* absolute UTC epoch seconds (64-bit) */
    char          msg[Y2K38_EVENT_MSG_MAX];
};

/* Append: EVENT <id> <epoch64> <msg>\n */
int y2k38_event_append(FILE *fp, const struct y2k38_event *ev);

/*
 * Parse one line. Returns 1 on success, 0 on EOF/empty, -1 on hard error.
 * Lines starting with '#' are skipped (return 0).
 */
int y2k38_event_parse_line(const char *line, struct y2k38_event *ev);

#ifdef __cplusplus
}
#endif

#endif /* Y2K38_EVENTLOG_H */
