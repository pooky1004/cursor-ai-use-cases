/*
 * BROKEN Y2K38 demo — models ELDK PPC32 behaviour even on 64-bit hosts.
 *
 * On real ELDK 3.1.1 / 32-bit time_t, casting and subtraction wrap naturally.
 * On modern hosts where time_t is already 64-bit, we force 32-bit narrowing
 * via int32_t so the failure mode remains visible.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Emulate signed 32-bit time_t storage (ELDK PPC32). */
typedef int32_t time32_t;

static time32_t to_time32(long long v)
{
    return (time32_t)v; /* C conversion: low 32 bits, signed interpretation */
}

static void scenario_truncate_future(void)
{
    long long real_future = 4102444800LL; /* 2100-01-01 UTC */
    time32_t stored;

    printf("=== Scenario 1: store post-2038 time in 32-bit time_t ===\n");
    printf("true future (int64) = %lld\n", real_future);
    stored = to_time32(real_future);
    printf("stored as time32_t  = %ld\n", (long)stored);
    if ((long long)stored != real_future)
        printf("FAIL: truncated/wrapped — classic Y2K38 storage bug\n");
    else
        printf("unexpected: value survived 32-bit store\n");
}

static void scenario_delta_overflow(void)
{
    time32_t now;
    time32_t future;
    long delta;
    long long wanted_future = (long long)INT32_MAX + 500;

    printf("\n=== Scenario 2: delta near Y2K38 boundary ===\n");

    now = to_time32(INT32_MAX - 100);
    future = to_time32(wanted_future); /* becomes negative on signed 32-bit */
    delta = (long)future - (long)now;

    printf("mock now           = %ld\n", (long)now);
    printf("wanted future      = %lld\n", wanted_future);
    printf("stored future32    = %ld\n", (long)future);
    printf("delta (future-now) = %ld  (wanted 600)\n", delta);

    if (delta != 600L)
        printf("FAIL: delta wrong due to 32-bit wrap / truncation\n");
    else
        printf("unexpected pass\n");
}

static void scenario_daemon_style_log(void)
{
    long long intended = (long long)INT32_MAX + 10;
    time32_t event_time = to_time32(intended);
    time32_t now = to_time32(INT32_MAX - 50);
    long delta;
    FILE *fp;
    const char *path = "broken_events.log";

    printf("\n=== Scenario 3: daemon A/B pipeline with time32_t ===\n");

    fp = fopen(path, "w");
    if (!fp) {
        perror(path);
        return;
    }
    fprintf(fp, "EVENT E1 %ld transformer-overheat\n", (long)event_time);
    fclose(fp);

    delta = (long)event_time - (long)now;

    printf("intended epoch     = %lld\n", intended);
    printf("logged time32      = %ld\n", (long)event_time);
    printf("daemon_b now32     = %ld\n", (long)now);
    printf("daemon_b delta     = %ld  (wanted 60)\n", delta);
    printf("FAIL: Daemon B would persist a bogus DELTA after Y2K38\n");
    printf("(native sizeof(time_t) on this host = %lu)\n",
           (unsigned long)sizeof(time_t));
}

int main(void)
{
    printf("Emulating ELDK 32-bit time_t (int32_t).\n");
    printf("Host sizeof(time_t)=%lu\n\n", (unsigned long)sizeof(time_t));
    scenario_truncate_future();
    scenario_delta_overflow();
    scenario_daemon_style_log();
    return 0;
}
