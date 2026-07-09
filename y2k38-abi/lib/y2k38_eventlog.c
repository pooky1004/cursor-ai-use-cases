/*
 * Event log line I/O using 64-bit epoch fields.
 */
#include <stdio.h>
#include <string.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>

int y2k38_event_append(FILE *fp, const struct y2k38_event *ev)
{
    if (!fp || !ev)
        return -1;

    /* EVENT <id> <epoch64> <msg> */
    if (fprintf(fp, "EVENT %s ", ev->id) < 0)
        return -1;
    if (y2k38_fprint_epoch(fp, ev->when) < 0)
        return -1;
    if (fprintf(fp, " %s\n", ev->msg) < 0)
        return -1;
    fflush(fp);
    return 0;
}

int y2k38_event_parse_line(const char *line, struct y2k38_event *ev)
{
    const char *p;
    char epoch_buf[64];
    size_t i;

    if (!line || !ev)
        return -1;

    while (*line == ' ' || *line == '\t')
        line++;
    if (*line == '\0' || *line == '\n' || *line == '#')
        return 0;

    if (strncmp(line, "EVENT ", 6) != 0)
        return -1;
    p = line + 6;

    i = 0;
    while (*p && *p != ' ' && *p != '\t' && i + 1 < sizeof(ev->id))
        ev->id[i++] = *p++;
    ev->id[i] = '\0';
    if (i == 0)
        return -1;
    while (*p == ' ' || *p == '\t')
        p++;

    i = 0;
    while (*p && *p != ' ' && *p != '\t' && i + 1 < sizeof(epoch_buf))
        epoch_buf[i++] = *p++;
    epoch_buf[i] = '\0';
    if (i == 0 || y2k38_parse_epoch(epoch_buf, &ev->when) != 0)
        return -1;
    while (*p == ' ' || *p == '\t')
        p++;

    i = 0;
    while (*p && *p != '\n' && *p != '\r' && i + 1 < sizeof(ev->msg))
        ev->msg[i++] = *p++;
    ev->msg[i] = '\0';
    return 1;
}
