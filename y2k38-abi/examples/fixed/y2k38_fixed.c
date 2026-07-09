/*
 * FIXED Y2K38 demo — same scenarios using liby2k38safe / y2k38_time_t.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>
#include <y2k38/types.h>

static void scenario_store_future(void)
{
    y2k38_time_t real_future = 4102444800LL; /* 2100-01-01 UTC */
    char buf[64];
    char iso[64];

    printf("=== Scenario 1: store post-2038 in y2k38_time_t ===\n");
    y2k38_format_epoch(real_future, buf, sizeof(buf));
    y2k38_format_iso8601_utc(real_future, iso, sizeof(iso));
    printf("stored (int64) = %s\n", buf);
    printf("ISO-8601 UTC   = %s\n", iso);
    if (y2k38_is_past_time_t32_max(real_future))
        printf("OK: past signed 32-bit time_t max, still representable\n");
}

static void scenario_delta_safe(void)
{
    y2k38_time_t now;
    y2k38_time_t future;
    y2k38_time_t delta;

    printf("\n=== Scenario 2: delta near Y2K38 boundary ===\n");

    now = Y2K38_TIME_T32_MAX - 100;
    future = Y2K38_TIME_T32_MAX + 500;
    delta = y2k38_difftime_sec(future, now);

    printf("mock now    = %lld\n", (long long)now);
    printf("mock future = %lld\n", (long long)future);
    printf("delta       = %lld (expected 600)\n", (long long)delta);
    if (delta == 600)
        printf("OK: 64-bit delta correct across the 2038 boundary\n");
    else
        printf("FAIL: unexpected delta\n");
}

static void scenario_daemon_style_log(void)
{
    struct y2k38_event ev;
    FILE *fp;
    const char *path = "fixed_events.log";
    y2k38_time_t intended = Y2K38_TIME_T32_MAX + 10;
    char line[512];
    struct y2k38_event parsed;
    y2k38_time_t now;
    y2k38_time_t delta;

    printf("\n=== Scenario 3: daemon A/B style with library ===\n");

    memset(&ev, 0, sizeof(ev));
    snprintf(ev.id, sizeof(ev.id), "E1");
    snprintf(ev.msg, sizeof(ev.msg), "transformer-overheat");
    ev.when = intended;

    fp = fopen(path, "w");
    if (!fp) {
        perror(path);
        return;
    }
    if (y2k38_event_append(fp, &ev) != 0) {
        fprintf(stderr, "append failed\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return;
    }
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return;
    }
    fclose(fp);

    if (y2k38_event_parse_line(line, &parsed) != 1) {
        fprintf(stderr, "parse failed: %s", line);
        return;
    }

    y2k38_clock_set_mock(1, Y2K38_TIME_T32_MAX - 50);
    now = y2k38_time(NULL);
    delta = y2k38_difftime_sec(parsed.when, now);

    printf("logged when = %lld\n", (long long)parsed.when);
    printf("mock now    = %lld\n", (long long)now);
    printf("delta       = %lld (expected 60)\n", (long long)delta);
    if (parsed.when == intended && delta == 60)
        printf("OK: durable log + delta survive the Y2K38 boundary\n");
    else
        printf("FAIL\n");

    y2k38_clock_set_mock(0, 0);
}

int main(void)
{
    printf("sizeof(y2k38_time_t)=%lu\n\n",
           (unsigned long)sizeof(y2k38_time_t));
    scenario_store_future();
    scenario_delta_safe();
    scenario_daemon_style_log();
    return 0;
}
