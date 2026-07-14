/*
 * Daemon A — three-thread Y2K38 time demo / logger.
 *
 * Thread 1: every 2s print now, start, and (now - start).
 * Thread 2: random-second sleep via y2k38_nsleep_relative; print enter/wake/diff;
 *           then pause random 1..5s and repeat.
 * Thread 3: nanosecond-scale work; print progress every 3s.
 *
 * Stop: clear g_run on SIGINT/SIGTERM (kill <pid> or kill -TERM).
 * Note: kill -9 (SIGKILL) cannot be caught; flag-based exit needs TERM/INT.
 *
 * Usage:
 *   daemon_a [logfile] [--offset-file PATH|--no-offset-file] [--auto-wrap]
 *            [--mock-now EPOCH] [--mock-kernel SEC]
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
#include <time.h>
#include <unistd.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>

static volatile sig_atomic_t g_run = 1;
static FILE *g_logfp;
static pthread_mutex_t g_log_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_notify_mu = PTHREAD_MUTEX_INITIALIZER;
static unsigned g_last_notified_wrap;

static void on_signal(int sig)
{
    (void)sig;
    g_run = 0;
}

/* Local wrap → persist already done by auto-wrap; notify peer daemons. */
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
            "daemon_a: WRAP count=%u offset=%lld — SIGHUP peers\n",
            count, (long long)new_offset);
    n = y2k38_sighup_list_notify((long)getpid());
    fprintf(stderr, "daemon_a: SIGHUP sent to %d process(es)\n", n);
    fflush(stderr);
}

/* Peer (or check/offsetctl) SIGHUP → OFFSET already reloaded by session. */
static void on_peer_sighup(y2k38_time_t new_offset, void *user)
{
    (void)user;
    fprintf(stderr,
            "daemon_a: received SIGHUP — reloaded offset=%lld\n",
            (long long)new_offset);
    fflush(stderr);
}

/*
 * Per-thread: read clock (may detect wrap → notify), poll wrap, drain SIGHUP.
 */
static void thread_service_clock(const char *tag)
{
    int hit;

    (void)y2k38_time(NULL);
    hit = y2k38_clock_poll_wrap();
    if (hit)
        fprintf(stderr, "daemon_a: %s poll_wrap hit\n", tag);
    (void)y2k38_session_poll_sighup();
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [logfile] [--offset-file PATH|--no-offset-file] "
            "[--auto-wrap] [--mock-now EPOCH] [--mock-kernel SEC]\n"
            "  3 threads: ticker(2s), random-sleep, nanos-work(3s report)\n"
            "  Stop with: kill -TERM <pid>  (or Ctrl-C)\n",
            argv0);
}

static int apply_startup_offset(const char *offset_file, int no_offset)
{
    if (no_offset)
        return 0;
    if (offset_file && offset_file[0])
        setenv(Y2K38_OFFSET_ENV, offset_file, 1);
    if (y2k38_session_init("daemon_a") != 0) {
        fprintf(stderr, "daemon_a: session_init failed\n");
        return -1;
    }
    y2k38_clock_on_wrap_callback(on_local_wrap, NULL);
    y2k38_session_on_sighup_callback(on_peer_sighup, NULL);
    y2k38_clock_set_auto_wrap(1, y2k38_clock_resolve_offset_path(NULL));
    fprintf(stderr, "daemon_a: session ready offset=%lld (wrap→SIGHUP, peer SIGHUP ok)\n",
            (long long)y2k38_clock_get_kernel_offset());
    return 0;
}

static void print_times(const char *tag, y2k38_time_t t0, y2k38_time_t t1)
{
    char s0[64], s1[64];

    y2k38_format_datetime_ctl(t0, s0, sizeof(s0));
    y2k38_format_datetime_ctl(t1, s1, sizeof(s1));
    printf("[%s] start=%s  now=%s  delta=%lld s\n",
           tag, s0, s1, (long long)y2k38_difftime_sec(t1, t0));
    fflush(stdout);
}

/* Interruptible relative sleep; returns early when g_run clears. */
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

/*
 * Thread 1 — wall-clock ticker every 2 seconds.
 */
static void *thread_ticker(void *arg)
{
    y2k38_time_t start = y2k38_time(NULL);

    (void)arg;
    printf("[A-T1] start captured: %lld\n", (long long)start);
    fflush(stdout);

    while (g_run) {
        y2k38_time_t now;

        thread_service_clock("A-T1");
        now = y2k38_time(NULL);
        print_times("A-T1", start, now);

        if (g_logfp) {
            struct y2k38_event ev;
            memset(&ev, 0, sizeof(ev));
            snprintf(ev.id, sizeof(ev.id), "T1");
            snprintf(ev.msg, sizeof(ev.msg), "ticker");
            ev.when = now;
            pthread_mutex_lock(&g_log_mu);
            (void)y2k38_event_append(g_logfp, &ev);
            fflush(g_logfp);
            pthread_mutex_unlock(&g_log_mu);
        }

        sleep_interruptible(2);
    }
    return NULL;
}

/*
 * Thread 2 — random sleep, report enter/wake/diff, then pause 1..5s.
 */
static void *thread_sleeper(void *arg)
{
    unsigned seed;

    (void)arg;
    seed = (unsigned)time(NULL) ^ ((unsigned)getpid() << 16) ^ 0xA000u;
    srand(seed);

    while (g_run) {
        y2k38_time_t enter;
        y2k38_time_t wake;
        int sleep_sec;
        int pause_sec;
        char ebuf[64], wbuf[64];

        sleep_sec = (rand() % 10) + 1; /* 1..10 seconds */
        thread_service_clock("A-T2");
        enter = y2k38_time(NULL);
        y2k38_format_datetime_ctl(enter, ebuf, sizeof(ebuf));
        printf("[A-T2] sleep_enter=%s  sleep_sec=%d\n", ebuf, sleep_sec);
        fflush(stdout);

        sleep_interruptible((y2k38_time_t)sleep_sec);
        if (!g_run)
            break;

        wake = y2k38_time(NULL);
        thread_service_clock("A-T2");
        y2k38_format_datetime_ctl(wake, wbuf, sizeof(wbuf));
        printf("[A-T2] sleep_wake=%s  enter=%s  delta=%lld s (wanted %d)\n",
               wbuf, ebuf,
               (long long)y2k38_difftime_sec(wake, enter),
               sleep_sec);
        fflush(stdout);

        pause_sec = (rand() % 5) + 1; /* 1..5 */
        printf("[A-T2] pause %d s before next cycle\n", pause_sec);
        fflush(stdout);
        sleep_interruptible((y2k38_time_t)pause_sec);
    }
    return NULL;
}

/*
 * Thread 3 — nanosecond-scale busy/measurement work; report every 3s.
 */
static void *thread_nanos(void *arg)
{
    uint64_t iterations = 0;
    uint64_t ns_budgeted = 0;
    y2k38_time_t last_report;
    struct timespec ts0, ts1;

    (void)arg;
    last_report = y2k38_time(NULL);
    printf("[A-T3] nanosecond worker started\n");
    fflush(stdout);

    while (g_run) {
        /*
         * One "quantum": spin until ~100us of clock_gettime time elapses,
         * counting iterations (nanosecond-granularity work).
         */
        clock_gettime(CLOCK_MONOTONIC, &ts0);
        for (;;) {
            long long elapsed_ns;
            iterations++;
            clock_gettime(CLOCK_MONOTONIC, &ts1);
            elapsed_ns = (long long)(ts1.tv_sec - ts0.tv_sec) * 1000000000LL
                         + (long long)(ts1.tv_nsec - ts0.tv_nsec);
            if (elapsed_ns >= 100000LL) { /* ~100 us */
                ns_budgeted += (uint64_t)elapsed_ns;
                break;
            }
        }

        /* Yield a little so other threads get CPU. */
        y2k38_nsleep_relative(0, 1000000L); /* 1 ms */
        thread_service_clock("A-T3");

        {
            y2k38_time_t now = y2k38_time(NULL);
            if (y2k38_difftime_sec(now, last_report) >= 3) {
                printf("[A-T3] report: iterations=%llu  measured_ns=%llu  "
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
    const char *path = NULL;
    const char *offset_file = NULL;
    int no_offset = 0;
    int auto_wrap = 0;
    int i;
    y2k38_time_t mock = 0;
    int have_mock = 0;
    int32_t mock_k = 0;
    int have_mock_k = 0;
    pthread_t th[3];

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-offset-file") == 0) {
            no_offset = 1;
        } else if (strcmp(argv[i], "--auto-wrap") == 0) {
            auto_wrap = 1;
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
        } else if (!path) {
            path = argv[i];
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (apply_startup_offset(offset_file, no_offset) != 0)
        return 1;

    if (auto_wrap) {
        const char *persist = offset_file;
        if (!persist || no_offset)
            persist = Y2K38_OFFSET_PATH_DEFAULT;
        y2k38_clock_set_auto_wrap(1, persist);
    }

    if (have_mock_k)
        y2k38_clock_set_mock_kernel(1, mock_k);
    if (have_mock)
        y2k38_clock_set_mock(1, mock);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    if (path) {
        g_logfp = fopen(path, "a");
        if (!g_logfp) {
            perror(path);
            y2k38_session_exit();
            return 1;
        }
    }

    fprintf(stderr,
            "daemon_a: pid=%ld threads=3 logfile=%s\n"
            "  stop with: kill -TERM %ld   (kill -9 cannot use g_run flag)\n",
            (long)getpid(), path ? path : "(none)", (long)getpid());

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

    if (g_logfp)
        fclose(g_logfp);
    y2k38_session_exit();
    fprintf(stderr, "daemon_a: exit\n");
    return 0;
}
