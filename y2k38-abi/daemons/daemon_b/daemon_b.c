/*
 * Daemon B — periodically recomputes (event_time - now) for scheduled
 * future events written by Daemon A. Uses y2k38_difftime_sec so deltas
 * stay correct across the 2038 boundary.
 *
 * Usage:
 *   daemon_b <event_log> <delta_out> [period_sec] [options]
 *
 * Options:
 *   --once
 *   --mock-now EPOCH
 *   --mock-kernel SEC
 *   --offset-file PATH | --no-offset-file
 *
 * Output lines:
 *   DELTA <id> <seconds> <when_epoch> <now_epoch> <msg>
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>

static volatile sig_atomic_t g_run = 1;
static const char *g_offset_file;

static void on_signal(int sig)
{
    (void)sig;
    g_run = 0;
}

static void on_sighup(int sig)
{
    (void)sig;
    if (g_offset_file && g_offset_file[0]) {
        if (y2k38_clock_reload_offset_default(g_offset_file) == 0)
            fprintf(stderr, "daemon_b: reloaded offset=%lld\n",
                    (long long)y2k38_clock_get_kernel_offset());
    }
}

static void store_offset_path(const char *offset_file, int no_offset)
{
    static char path_buf[512];

    if (no_offset) {
        g_offset_file = NULL;
        return;
    }
    snprintf(path_buf, sizeof(path_buf), "%s",
             y2k38_clock_resolve_offset_path(offset_file));
    g_offset_file = path_buf;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s <event_log> <delta_out> [period_sec] [--once] "
            "[--mock-now EPOCH] [--mock-kernel SEC] "
            "[--offset-file PATH|--no-offset-file]\n",
            argv0);
}

static int apply_startup_offset(const char *offset_file, int no_offset)
{
    int st;

    if (no_offset)
        return 0;
    st = y2k38_clock_apply_offset_default(offset_file);
    if (st < 0) {
        fprintf(stderr, "daemon_b: bad offset file\n");
        return -1;
    }
    if (st == 0) {
        fprintf(stderr, "daemon_b: kernel offset=%lld\n",
                (long long)y2k38_clock_get_kernel_offset());
    }
    return 0;
}

static int recomputedeltas(const char *event_path, FILE *out)
{
    FILE *in;
    char line[512];
    y2k38_time_t now;
    int count = 0;

    in = fopen(event_path, "r");
    if (!in) {
        perror(event_path);
        return -1;
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
        if (pr == 0)
            continue;
        if (pr < 0) {
            fprintf(stderr, "daemon_b: skip bad line: %s", line);
            continue;
        }

        /* Heartbeats are "now" snapshots, not irregular future events. */
        if (strcmp(ev.id, "HB") == 0)
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
    fflush(out);
    fprintf(stderr, "daemon_b: wrote %d deltas (now=%lld)\n",
            count, (long long)now);
    return 0;
}

int main(int argc, char **argv)
{
    const char *event_path = NULL;
    const char *out_path = NULL;
    const char *offset_file = NULL;
    int period = 5;
    int once = 0;
    int no_offset = 0;
    int i;
    y2k38_time_t mock = 0;
    int have_mock = 0;
    int32_t mock_k = 0;
    int have_mock_k = 0;
    FILE *out;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--once") == 0) {
            once = 1;
        } else if (strcmp(argv[i], "--no-offset-file") == 0) {
            no_offset = 1;
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
        } else if (argv[i][0] == '-') {
            usage(argv[0]);
            return 1;
        } else if (!event_path) {
            event_path = argv[i];
        } else if (!out_path) {
            out_path = argv[i];
        } else {
            period = atoi(argv[i]);
            if (period <= 0)
                period = 5;
        }
    }

    if (!event_path || !out_path) {
        usage(argv[0]);
        return 1;
    }

    if (apply_startup_offset(offset_file, no_offset) != 0)
        return 1;

    if (!no_offset)
        store_offset_path(offset_file, no_offset);

    if (have_mock_k)
        y2k38_clock_set_mock_kernel(1, mock_k);
    if (have_mock)
        y2k38_clock_set_mock(1, mock);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGHUP, on_sighup);

    out = fopen(out_path, "w");
    if (!out) {
        perror(out_path);
        return 1;
    }

    fprintf(stderr, "daemon_b: events=%s deltas=%s period=%d\n",
            event_path, out_path, period);

    do {
        if (fseek(out, 0, SEEK_SET) != 0) {
            fclose(out);
            out = fopen(out_path, "w");
            if (!out) {
                perror(out_path);
                return 1;
            }
        }
        recomputedeltas(event_path, out);
        if (once)
            break;
        {
            int slept = 0;
            while (g_run && slept < period) {
                sleep(1);
                slept++;
            }
        }
    } while (g_run);

    fclose(out);
    fprintf(stderr, "daemon_b: exit\n");
    return 0;
}
