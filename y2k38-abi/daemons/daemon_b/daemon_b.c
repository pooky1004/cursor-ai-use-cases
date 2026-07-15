/*
 * Daemon B — three-thread Y2K38 time demo (mirrors Daemon A layout).
 *
 * Thread activity prints are gated by --debug / -d.
 * WRAP and peer SIGHUP messages are always printed.
 *
 * Optionally recomputes DELTA lines from Daemon A event log in thread 1
 * when event_log + delta_out are given.
 *
 * Stop: clear g_run on SIGINT/SIGTERM (kill -TERM).
 *
 * Usage:
 *   daemon_b [event_log delta_out] [--offset-file PATH|--no-offset-file]
 *            [--mock-now EPOCH] [--mock-kernel SEC] [--debug|-d]
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>

static volatile sig_atomic_t g_run = 1;
static int g_debug;
static const char *g_event_path;
static const char *g_delta_path;
static pthread_mutex_t g_delta_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_notify_mu = PTHREAD_MUTEX_INITIALIZER;
static unsigned g_last_notified_wrap;

#define DBG(...) \
    do { if (g_debug) fprintf(stdout, __VA_ARGS__); } while (0)

/* ELDK 3.1.1 may not define CLOCK_MONOTONIC — fall back to gettimeofday. */
static int mono_gettime(struct timespec *ts)
{
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, ts) == 0)
        return 0;
#endif
    {
        struct timeval tv;

        if (gettimeofday(&tv, NULL) != 0)
            return -1;
        ts->tv_sec = (time_t)tv.tv_sec;
        ts->tv_nsec = (long)tv.tv_usec * 1000L;
        return 0;
    }
}

static void on_signal(int sig)
{
    (void)sig;
    g_run = 0;
}

static void on_local_wrap(y2k38_time_t new_offset, unsigned count, void *user)
{
    int n;

    (void)user;
    pthread_mutex_lock(&g_notify_mu);
    if (count == g_last_notified_wrap) {
        pthread_mutex_unlock(&g_notify_mu);
        return;
    }
    g_last_notified_wrap = count;
    pthread_mutex_unlock(&g_notify_mu);

    fprintf(stderr,
            "daemon_b: WRAP count=%u offset=%lld — SIGHUP peers\n",
            count, (long long)new_offset);
    n = y2k38_sighup_list_notify((long)getpid());
    fprintf(stderr, "daemon_b: SIGHUP sent to %d process(es)\n", n);
    fflush(stderr);
}

static void on_peer_sighup(y2k38_time_t new_offset, void *user)
{
    (void)user;
    fprintf(stderr,
            "daemon_b: received SIGHUP — reloaded offset=%lld\n",
            (long long)new_offset);
    fflush(stderr);
}

static void thread_service_clock(const char *tag)
{
    int hit;

    (void)y2k38_time(NULL);
    hit = y2k38_clock_poll_wrap();
    if (hit)
        fprintf(stderr, "daemon_b: %s poll_wrap hit\n", tag);
    (void)y2k38_session_poll_sighup();
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [event_log delta_out] "
            "[--offset-file PATH|--no-offset-file] "
            "[--mock-now EPOCH] [--mock-kernel SEC] [--debug|-d]\n"
            "  3 threads: ticker(2s), random-sleep, nanos-work(3s report)\n"
            "  --debug: thread status prints; WRAP/SIGHUP always printed\n"
            "  Stop with: kill -TERM <pid>  (or Ctrl-C)\n",
            argv0);
}

static int apply_startup_offset(const char *offset_file, int no_offset)
{
    if (no_offset)
        return 0;
    if (offset_file && offset_file[0])
        setenv(Y2K38_OFFSET_ENV, offset_file, 1);
    if (y2k38_session_init("daemon_b") != 0) {
        fprintf(stderr, "daemon_b: session_init failed\n");
        return -1;
    }
    y2k38_clock_on_wrap_callback(on_local_wrap, NULL);
    y2k38_session_on_sighup_callback(on_peer_sighup, NULL);
    y2k38_clock_set_auto_wrap(1, y2k38_clock_resolve_offset_path(NULL));
    fprintf(stderr,
            "daemon_b: session ready offset=%lld (wrap→SIGHUP, peer SIGHUP ok)\n",
            (long long)y2k38_clock_get_kernel_offset());
    return 0;
}

static void print_times(const char *tag, y2k38_time_t t0, y2k38_time_t t1)
{
    char s0[64], s1[64];

    if (!g_debug)
        return;
    y2k38_format_datetime_ctl(t0, s0, sizeof(s0));
    y2k38_format_datetime_ctl(t1, s1, sizeof(s1));
    printf("[%s] start=%s  now=%s  delta=%lld s\n",
           tag, s0, s1, (long long)y2k38_difftime_sec(t1, t0));
    fflush(stdout);
}

static void sleep_interruptible(y2k38_time_t total_sec)
{
    y2k38_time_t left = total_sec;

    while (g_run && left > 0) {
        y2k38_time_t chunk = (left > 1) ? 1 : left;
        thread_service_clock("sleep");
        y2k38_nsleep_relative(chunk, 0);
        left -= chunk;
    }
}

static void recomputedeltas_once(void)
{
    FILE *in;
    FILE *out;
    char line[512];
    y2k38_time_t now;
    int count = 0;

    if (!g_event_path || !g_delta_path)
        return;

    pthread_mutex_lock(&g_delta_mu);
    in = fopen(g_event_path, "r");
    if (!in) {
        pthread_mutex_unlock(&g_delta_mu);
        return;
    }
    out = fopen(g_delta_path, "w");
    if (!out) {
        fclose(in);
        pthread_mutex_unlock(&g_delta_mu);
        return;
    }

    now = y2k38_time(NULL);
    fprintf(out, "# recomputed at ");
    y2k38_fprint_epoch(out, now);
    fprintf(out, "\n");

    while (fgets(line, sizeof(line), in)) {
        struct y2k38_event ev;
        y2k38_time_t delta;
        int pr;

        pr = y2k38_event_parse_line(line, &ev);
        if (pr <= 0)
            continue;
        if (strcmp(ev.id, "HB") == 0 || strcmp(ev.id, "T1") == 0)
            continue;

        delta = y2k38_difftime_sec(ev.when, now);
        fprintf(out, "DELTA %s %lld ", ev.id, (long long)delta);
        y2k38_fprint_epoch(out, ev.when);
        fprintf(out, " ");
        y2k38_fprint_epoch(out, now);
        fprintf(out, " %s\n", ev.msg);
        count++;
    }

    fclose(in);
    fclose(out);
    pthread_mutex_unlock(&g_delta_mu);
    if (g_debug) {
        printf("[B-T1] recomputed %d deltas -> %s\n", count, g_delta_path);
        fflush(stdout);
    }
}

static void *thread_ticker(void *arg)
{
    y2k38_time_t start = y2k38_time(NULL);

    (void)arg;
    DBG("[B-T1] start captured: %lld\n", (long long)start);
    if (g_debug)
        fflush(stdout);

    while (g_run) {
        y2k38_time_t now;

        thread_service_clock("B-T1");
        now = y2k38_time(NULL);
        print_times("B-T1", start, now);
        recomputedeltas_once();
        sleep_interruptible(2);
    }
    return NULL;
}

static void *thread_sleeper(void *arg)
{
    unsigned seed;

    (void)arg;
    seed = (unsigned)time(NULL) ^ ((unsigned)getpid() << 16) ^ 0xB000u;
    srand(seed);

    while (g_run) {
        y2k38_time_t enter;
        y2k38_time_t wake;
        int sleep_sec;
        int pause_sec;
        char ebuf[64], wbuf[64];

        sleep_sec = (rand() % 10) + 1;
        thread_service_clock("B-T2");
        enter = y2k38_time(NULL);
        if (g_debug) {
            y2k38_format_datetime_ctl(enter, ebuf, sizeof(ebuf));
            printf("[B-T2] sleep_enter=%s  sleep_sec=%d\n", ebuf, sleep_sec);
            fflush(stdout);
        }

        sleep_interruptible((y2k38_time_t)sleep_sec);
        if (!g_run)
            break;

        wake = y2k38_time(NULL);
        thread_service_clock("B-T2");
        if (g_debug) {
            y2k38_format_datetime_ctl(wake, wbuf, sizeof(wbuf));
            y2k38_format_datetime_ctl(enter, ebuf, sizeof(ebuf));
            printf("[B-T2] sleep_wake=%s  enter=%s  delta=%lld s (wanted %d)\n",
                   wbuf, ebuf,
                   (long long)y2k38_difftime_sec(wake, enter),
                   sleep_sec);
            fflush(stdout);
        }

        pause_sec = (rand() % 5) + 1;
        DBG("[B-T2] pause %d s before next cycle\n", pause_sec);
        if (g_debug)
            fflush(stdout);
        sleep_interruptible((y2k38_time_t)pause_sec);
    }
    return NULL;
}

static void *thread_nanos(void *arg)
{
    uint64_t iterations = 0;
    uint64_t ns_budgeted = 0;
    y2k38_time_t last_report;
    struct timespec ts0, ts1;

    (void)arg;
    last_report = y2k38_time(NULL);
    DBG("[B-T3] nanosecond worker started\n");
    if (g_debug)
        fflush(stdout);

    while (g_run) {
        if (mono_gettime(&ts0) != 0)
            break;
        for (;;) {
            long long elapsed_ns;

            iterations++;
            if (mono_gettime(&ts1) != 0)
                break;
            elapsed_ns = (long long)(ts1.tv_sec - ts0.tv_sec) * 1000000000LL
                         + (long long)(ts1.tv_nsec - ts0.tv_nsec);
            if (elapsed_ns >= 100000LL) {
                ns_budgeted += (uint64_t)elapsed_ns;
                break;
            }
        }

        y2k38_nsleep_relative(0, 1000000L);
        thread_service_clock("B-T3");

        {
            y2k38_time_t now = y2k38_time(NULL);
            if (g_debug && y2k38_difftime_sec(now, last_report) >= 3) {
                printf("[B-T3] report: iterations=%llu  measured_ns=%llu  "
                       "avg_ns_per_quantum=%llu  utc_epoch=%lld\n",
                       (unsigned long long)iterations,
                       (unsigned long long)ns_budgeted,
                       iterations
                           ? (unsigned long long)(ns_budgeted / iterations)
                           : 0ULL,
                       (long long)now);
                fflush(stdout);
                last_report = now;
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    const char *event_path = NULL;
    const char *out_path = NULL;
    const char *offset_file = NULL;
    int no_offset = 0;
    int i;
    y2k38_time_t mock = 0;
    int have_mock = 0;
    int32_t mock_k = 0;
    int have_mock_k = 0;
    pthread_t th[3];

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-offset-file") == 0) {
            no_offset = 1;
        } else if (strcmp(argv[i], "--debug") == 0 ||
                   strcmp(argv[i], "-d") == 0) {
            g_debug = 1;
        } else if (strcmp(argv[i], "--offset-file") == 0 && i + 1 < argc) {
            offset_file = argv[++i];
        } else if (strcmp(argv[i], "--mock-now") == 0 && i + 1 < argc) {
            if (y2k38_parse_epoch(argv[++i], &mock) != 0) {
                fprintf(stderr, "bad --mock-now\n");
                return 1;
            }
            have_mock = 1;
        } else if (strcmp(argv[i], "--mock-kernel") == 0 && i + 1 < argc) {
            mock_k = (int32_t)strtoll(argv[++i], NULL, 10);
            have_mock_k = 1;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            usage(argv[0]);
            return 1;
        } else if (!event_path) {
            event_path = argv[i];
        } else if (!out_path) {
            out_path = argv[i];
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if ((event_path && !out_path) || (!event_path && out_path)) {
        usage(argv[0]);
        return 1;
    }

    if (apply_startup_offset(offset_file, no_offset) != 0)
        return 1;

    if (have_mock_k)
        y2k38_clock_set_mock_kernel(1, mock_k);
    if (have_mock)
        y2k38_clock_set_mock(1, mock);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    g_event_path = event_path;
    g_delta_path = out_path;

    fprintf(stderr,
            "daemon_b: pid=%ld threads=3 events=%s deltas=%s debug=%d\n"
            "  stop with: kill -TERM %ld   (kill -9 cannot use g_run flag)\n",
            (long)getpid(),
            event_path ? event_path : "(none)",
            out_path ? out_path : "(none)",
            g_debug,
            (long)getpid());

    if (pthread_create(&th[0], NULL, thread_ticker, NULL) != 0 ||
        pthread_create(&th[1], NULL, thread_sleeper, NULL) != 0 ||
        pthread_create(&th[2], NULL, thread_nanos, NULL) != 0) {
        perror("pthread_create");
        g_run = 0;
    }

    while (g_run)
        sleep(1);

    pthread_join(th[0], NULL);
    pthread_join(th[1], NULL);
    pthread_join(th[2], NULL);

    y2k38_session_exit();
    fprintf(stderr, "daemon_b: exit\n");
    return 0;
}
