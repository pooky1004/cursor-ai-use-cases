/*
 * Daemon A — records events with Y2K38-safe timestamps (y2k38_time_t).
 *
 * Usage:
 *   daemon_a <logfile> [options]
 *
 * Options:
 *   --once
 *   --mock-now EPOCH          absolute UTC mock (ignores kernel/offset)
 *   --mock-kernel SEC         emulate signed 32-bit kernel second
 *   --offset-file PATH        load kernel offset (default: env or /etc/y2k38_offset)
 *   --no-offset-file          do not auto-load offset file
 *   --auto-wrap               detect kernel 32-bit wrap at runtime; persist OFFSET
 *
 * Without --once: runs until SIGTERM/SIGINT, appending a sample event every
 * few seconds, and also accepts lines on stdin:
 *   EVENT <id> <epoch64|NOW> <message...>
 *   SCHED <id> <epoch64> <message...>   (explicit future time)
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <y2k38/eventlog.h>
#include <y2k38/time.h>

static volatile sig_atomic_t g_run = 1;

static void on_signal(int sig)
{
    (void)sig;
    g_run = 0;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s <logfile> [--once] [--mock-now EPOCH] "
            "[--mock-kernel SEC] [--offset-file PATH|--no-offset-file] "
            "[--auto-wrap]\n",
            argv0);
}

static int apply_startup_offset(const char *offset_file, int no_offset)
{
    int st;

    if (no_offset)
        return 0;
    st = y2k38_clock_apply_offset_default(offset_file);
    if (st < 0) {
        fprintf(stderr, "daemon_a: bad offset file\n");
        return -1;
    }
    if (st == 0) {
        fprintf(stderr, "daemon_a: kernel offset=%lld\n",
                (long long)y2k38_clock_get_kernel_offset());
    }
    return 0;
}

static int handle_command(FILE *logfp, const char *line)
{
    struct y2k38_event ev;
    char cmd[16];
    char id[Y2K38_EVENT_ID_MAX];
    char when_tok[64];
    char msg[Y2K38_EVENT_MSG_MAX];
    y2k38_time_t when;

    memset(&ev, 0, sizeof(ev));
    msg[0] = '\0';

    if (sscanf(line, "%15s %63s %63s %255[^\n]", cmd, id, when_tok, msg) < 3)
        return -1;

    if (strcmp(cmd, "EVENT") != 0 && strcmp(cmd, "SCHED") != 0)
        return -1;

    if (strcmp(when_tok, "NOW") == 0)
        when = y2k38_time(NULL);
    else if (y2k38_parse_epoch(when_tok, &when) != 0)
        return -1;

    snprintf(ev.id, sizeof(ev.id), "%s", id);
    snprintf(ev.msg, sizeof(ev.msg), "%s", msg[0] ? msg : "event");
    ev.when = when;

    if (y2k38_event_append(logfp, &ev) != 0)
        return -1;

    fprintf(stderr, "daemon_a: logged %s at %lld\n",
            ev.id, (long long)ev.when);
    return 0;
}

static int seed_future_samples(FILE *logfp)
{
    struct y2k38_event ev;
    y2k38_time_t now = y2k38_time(NULL);

    memset(&ev, 0, sizeof(ev));

    /* Event at/after the classic overflow instant — MUST stay 64-bit. */
    snprintf(ev.id, sizeof(ev.id), "FUT2038");
    snprintf(ev.msg, sizeof(ev.msg), "first-second-after-time_t-overflow");
    ev.when = Y2K38_OVERFLOW_EPOCH_SEC;
    if (y2k38_event_append(logfp, &ev) != 0)
        return -1;

    snprintf(ev.id, sizeof(ev.id), "FAR2100");
    snprintf(ev.msg, sizeof(ev.msg), "century-maintenance-window");
    ev.when = 4102444800LL;
    if (y2k38_event_append(logfp, &ev) != 0)
        return -1;

    snprintf(ev.id, sizeof(ev.id), "NEAR");
    snprintf(ev.msg, sizeof(ev.msg), "relative-plus-120s");
    ev.when = now + 120;
    if (y2k38_event_append(logfp, &ev) != 0)
        return -1;

    return 0;
}

int main(int argc, char **argv)
{
    const char *path = NULL;
    const char *offset_file = NULL;
    int once = 0;
    int no_offset = 0;
    int auto_wrap = 0;
    int i;
    FILE *fp;
    y2k38_time_t mock = 0;
    int have_mock = 0;
    int32_t mock_k = 0;
    int have_mock_k = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--once") == 0) {
            once = 1;
        } else if (strcmp(argv[i], "--no-offset-file") == 0) {
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

    if (!path) {
        usage(argv[0]);
        return 1;
    }

    if (apply_startup_offset(offset_file, no_offset) != 0)
        return 1;

    if (auto_wrap) {
        const char *persist = offset_file;
        if (!persist || no_offset)
            persist = Y2K38_OFFSET_PATH_DEFAULT;
        y2k38_clock_set_auto_wrap(1, persist);
        fprintf(stderr, "daemon_a: auto-wrap enabled persist=%s\n", persist);
    }

    if (have_mock_k)
        y2k38_clock_set_mock_kernel(1, mock_k);
    if (have_mock)
        y2k38_clock_set_mock(1, mock);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    fp = fopen(path, "a");
    if (!fp) {
        perror(path);
        return 1;
    }

    fprintf(stderr, "daemon_a: logfile=%s sizeof(y2k38_time_t)=%lu\n",
            path, (unsigned long)sizeof(y2k38_time_t));

    if (seed_future_samples(fp) != 0) {
        fprintf(stderr, "daemon_a: seed failed\n");
        fclose(fp);
        return 1;
    }

    if (once) {
        fclose(fp);
        return 0;
    }

    /*
     * Simple loop: every 5s log a heartbeat with true 64-bit now, and
     * non-blocking-ish stdin commands when available (line buffered).
     */
    while (g_run) {
        struct y2k38_event beat;
        fd_set rfds;
        struct timeval tv;
        int ready;

        memset(&beat, 0, sizeof(beat));
        snprintf(beat.id, sizeof(beat.id), "HB");
        snprintf(beat.msg, sizeof(beat.msg), "heartbeat");
        beat.when = y2k38_time(NULL);
        y2k38_event_append(fp, &beat);

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        ready = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        if (ready > 0 && FD_ISSET(STDIN_FILENO, &rfds)) {
            char line[512];
            if (fgets(line, sizeof(line), stdin)) {
                if (handle_command(fp, line) != 0)
                    fprintf(stderr, "daemon_a: bad command\n");
            }
        } else if (ready < 0 && errno != EINTR) {
            perror("select");
            break;
        }
    }

    fclose(fp);
    fprintf(stderr, "daemon_a: exit\n");
    return 0;
}
