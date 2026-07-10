/*
 * Three-thread Y2K38 demo: concurrent time reads, post-2038 schedule math,
 * and sleep-until across the 2038 boundary.
 *
 * Usage:
 *   y2k38_threads_demo [--mock-now EPOCH] [--wake-abs EPOCH] [--ticks N]
 *
 * Default mock: now = INT32_MAX - 10 (pre-2038), wake = INT32_MAX + 30 (post-2038).
 * With --mock-now, a background ticker advances mock clock so sleep-until can finish.
 *
 * Build: make examples/fixed/y2k38_threads_demo
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>
#include <y2k38/types.h>

#define THREADS 3

typedef struct {
    int          use_mock;
    y2k38_time_t mock_start;
    y2k38_time_t wake_abs;
    int          tick_sec;
    int          done;
    pthread_mutex_t mu;
} shared_ctx_t;

static shared_ctx_t g_ctx;

static void lock_ctx(void)
{
    pthread_mutex_lock(&g_ctx.mu);
}

static void unlock_ctx(void)
{
    pthread_mutex_unlock(&g_ctx.mu);
}

/* Advance mock clock when enabled (simulates real time passing). */
static void mock_tick_if_needed(void)
{
    if (!g_ctx.use_mock)
        return;
    lock_ctx();
    g_ctx.mock_start += 1;
    y2k38_clock_set_mock(1, g_ctx.mock_start);
    unlock_ctx();
}

/*
 * Thread 1 — periodic wall-clock sampling (like Daemon A heartbeat).
 * Uses y2k38_time() + y2k38_gettimeofday(); logs ISO timestamp.
 */
static void *thread_logger(void *arg)
{
    int i;
    int ticks = *(int *)arg;

    for (i = 0; i < ticks; i++) {
        y2k38_time_t t;
        struct y2k38_timeval tv;
        char iso[64];
        char ep[32];

        t = y2k38_time(NULL);
        y2k38_gettimeofday(&tv);
        y2k38_format_iso8601_utc(t, iso, sizeof(iso));
        y2k38_format_epoch(t, ep, sizeof(ep));

        printf("[logger]  now=%s epoch=%s offset=%lld raw=%lld\n",
               iso, ep,
               (long long)y2k38_clock_get_kernel_offset(),
               (long long)y2k38_time_kernel_raw(NULL));

        mock_tick_if_needed();
        y2k38_nsleep_relative(1, 0);
    }
    return NULL;
}

/*
 * Thread 2 — schedule / delta worker (like Daemon B).
 * Compares "now" against events stored as y2k38_time_t past INT32_MAX.
 */
static void *thread_scheduler(void *arg)
{
    int i;
    int ticks = *(int *)arg;
    const y2k38_time_t far_future = 4102444800LL; /* 2100-01-01 */
    const y2k38_time_t at_overflow = Y2K38_OVERFLOW_EPOCH_SEC;

    for (i = 0; i < ticks; i++) {
        y2k38_time_t now;
        y2k38_time_t d_overflow;
        y2k38_time_t d_far;
        struct y2k38_event ev;

        now = y2k38_time(NULL);
        d_overflow = y2k38_difftime_sec(at_overflow, now);
        d_far = y2k38_difftime_sec(far_future, now);

        memset(&ev, 0, sizeof(ev));
        snprintf(ev.id, sizeof(ev.id), "T2-%d", i);
        snprintf(ev.msg, sizeof(ev.msg), "scheduler-tick");
        ev.when = at_overflow;
        /* Would append: EVENT id epoch msg — delta math shown below */

        printf("[schedule] now=%lld  delta_to_overflow=%lld  delta_to_2100=%lld"
               "  event_when=%lld\n",
               (long long)now, (long long)d_overflow, (long long)d_far,
               (long long)ev.when);

        if (y2k38_is_past_time_t32_max(at_overflow))
            printf("[schedule] OK: post-2038 target still representable in int64\n");

        y2k38_nsleep_relative(1, 0);
    }
    return NULL;
}

/*
 * Thread 3 — sleep until absolute wake time (now pre-2038, wake post-2038).
 * Uses y2k38_sleep_until() = relative nanosleep chunks vs y2k38_time().
 * NEVER uses timer_settime absolute or time_t-based pthread_cond_timedwait.
 */
static void *thread_sleeper(void *arg)
{
    y2k38_time_t wake = *(y2k38_time_t *)arg;
    y2k38_time_t now0;
    y2k38_time_t rem0;
    int rc;

    now0 = y2k38_time(NULL);
    rem0 = y2k38_difftime_sec(wake, now0);

    printf("[sleeper]  start now=%lld  wake=%lld  remaining=%lld sec\n",
           (long long)now0, (long long)wake, (long long)rem0);

    if (now0 < Y2K38_TIME_T32_MAX && wake > Y2K38_TIME_T32_MAX)
        printf("[sleeper]  crossing Y2K38 boundary (pre -> post) via relative waits\n");

    y2k38_sleep_set_max_chunk(5);

    rc = y2k38_sleep_until(wake);
    if (rc != 0) {
        perror("[sleeper]  y2k38_sleep_until");
        return (void *)(intptr_t)1;
    }

    printf("[sleeper]  woke at %lld (target %lld)\n",
           (long long)y2k38_time(NULL), (long long)wake);
    return NULL;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [--mock-now EPOCH] [--wake-abs EPOCH] [--ticks N]\n"
            "  --mock-now   fixed start; logger thread advances +1s per tick\n"
            "  --wake-abs   absolute wake target (default: INT32_MAX+30)\n"
            "  --ticks      logger/scheduler iterations (default: 35)\n",
            argv0);
}

int main(int argc, char **argv)
{
    pthread_t th[THREADS];
    int ticks = 35;
    int i;
    y2k38_time_t mock = Y2K38_TIME_T32_MAX - 10;
    y2k38_time_t wake = Y2K38_TIME_T32_MAX + 30;
    int have_mock = 0;
    void *ret;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mock-now") == 0 && i + 1 < argc) {
            if (y2k38_parse_epoch(argv[++i], &mock) != 0) {
                fprintf(stderr, "bad --mock-now\n");
                return 1;
            }
            have_mock = 1;
        } else if (strcmp(argv[i], "--wake-abs") == 0 && i + 1 < argc) {
            if (y2k38_parse_epoch(argv[++i], &wake) != 0) {
                fprintf(stderr, "bad --wake-abs\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--ticks") == 0 && i + 1 < argc) {
            ticks = atoi(argv[++i]);
            if (ticks < 5)
                ticks = 5;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    memset(&g_ctx, 0, sizeof(g_ctx));
    pthread_mutex_init(&g_ctx.mu, NULL);
    g_ctx.use_mock = have_mock;
    g_ctx.mock_start = mock;
    g_ctx.wake_abs = wake;
    g_ctx.tick_sec = 1;

    if (have_mock) {
        y2k38_clock_set_mock(1, mock);
        printf("mock now=%lld  wake=%lld  (logger advances mock +1s/tick)\n",
               (long long)mock, (long long)wake);
    } else {
        /* Real clock: wake a few seconds from now for runnable demo */
        y2k38_time_t now = y2k38_time(NULL);
        if (wake <= now || wake > now + 3600)
            wake = now + 5;
        printf("real clock now=%lld  wake=%lld\n",
               (long long)now, (long long)wake);
    }

    printf("\n=== Thread 1: logger (y2k38_time / gettimeofday) ===\n");
    printf("=== Thread 2: scheduler (delta to post-2038 events) ===\n");
    printf("=== Thread 3: sleeper (y2k38_sleep_until across boundary) ===\n\n");

    if (pthread_create(&th[0], NULL, thread_logger, &ticks) != 0) {
        perror("pthread_create logger");
        return 1;
    }
    if (pthread_create(&th[1], NULL, thread_scheduler, &ticks) != 0) {
        perror("pthread_create scheduler");
        return 1;
    }
    if (pthread_create(&th[2], NULL, thread_sleeper, &wake) != 0) {
        perror("pthread_create sleeper");
        return 1;
    }

    for (i = 0; i < THREADS; i++) {
        if (pthread_join(th[i], &ret) != 0) {
            perror("pthread_join");
            return 1;
        }
        if (ret != NULL) {
            fprintf(stderr, "thread %d returned error\n", i);
            return 1;
        }
    }

    pthread_mutex_destroy(&g_ctx.mu);
    printf("\nAll threads finished OK.\n");
    return 0;
}
