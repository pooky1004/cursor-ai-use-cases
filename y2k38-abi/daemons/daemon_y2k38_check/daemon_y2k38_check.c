/*
 * daemon_y2k38_check — monitors 32-bit kernel wrap and notifies peers.
 *
 * Responsibilities:
 *   - Manage SIGHUP subscriber list (/var/run/y2k38_sighup.list)
 *   - Register itself in that list
 *   - Poll kernel time with adaptive interval (far from wrap → rare;
 *     near wrap → frequent)
 *   - On wrap: update /etc/y2k38_offset and SIGHUP all other subscribers
 *     (never notify itself)
 *   - On SIGHUP (from offsetctl / other): reload OFFSET
 *
 * Usage:
 *   daemon_y2k38_check [--offset-file PATH] [--list PATH] [--once]
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <y2k38/time.h>

static volatile sig_atomic_t g_run = 1;

static void on_term(int sig)
{
    (void)sig;
    g_run = 0;
}

static void on_wrap(y2k38_time_t new_offset, unsigned count, void *user)
{
    int n;

    (void)user;
    fprintf(stderr,
            "daemon_y2k38_check: WRAP count=%u offset=%lld — notifying peers\n",
            count, (long long)new_offset);
    n = y2k38_sighup_list_notify((long)getpid());
    fprintf(stderr, "daemon_y2k38_check: SIGHUP sent to %d process(es)\n", n);
}

/*
 * Adaptive poll interval (seconds) based on time until signed 32-bit wrap.
 * Far → long sleep; close → short sleep.
 */
static unsigned adaptive_interval_sec(void)
{
    y2k38_time_t rem = y2k38_clock_seconds_until_wrap();

    if (rem <= 30)
        return 1;
    if (rem <= 300)
        return 5;
    if (rem <= 3600)
        return 30;
    if (rem <= 86400)
        return 300;       /* ≤ 1 day → every 5 min */
    if (rem <= 7 * 86400)
        return 1800;      /* ≤ 1 week → every 30 min */
    return 3600;          /* far → hourly */
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [--offset-file PATH] [--list PATH] [--once]\n"
            "  Monitors kernel wrap, updates OFFSET, SIGHUP subscribers.\n"
            "  SIGHUP list default: %s\n"
            "  Offset file default: %s\n",
            argv0,
            Y2K38_SIGHUP_LIST_PATH_DEFAULT,
            Y2K38_OFFSET_PATH_DEFAULT);
}

int main(int argc, char **argv)
{
    const char *offset_file = NULL;
    const char *list_file = NULL;
    int once = 0;
    int i;
    char ctl[64];

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--once") == 0) {
            once = 1;
        } else if (strcmp(argv[i], "--offset-file") == 0 && i + 1 < argc) {
            offset_file = argv[++i];
        } else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc) {
            list_file = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (list_file) {
        /* Override list path for this process via env (resolved by library). */
        setenv(Y2K38_SIGHUP_LIST_ENV, list_file, 1);
    }
    if (offset_file) {
        setenv(Y2K38_OFFSET_ENV, offset_file, 1);
    }

    signal(SIGINT, on_term);
    signal(SIGTERM, on_term);

    /*
     * Session init: register SIGHUP, add ourselves to the list, load OFFSET,
     * enable auto-wrap. Other processes will also receive SIGHUP when we wrap.
     */
    if (y2k38_session_init("daemon_y2k38_check") != 0) {
        fprintf(stderr, "daemon_y2k38_check: session_init failed\n");
        return 1;
    }

    y2k38_clock_on_wrap_callback(on_wrap, NULL);
    y2k38_clock_set_auto_wrap(1, y2k38_clock_resolve_offset_path(NULL));

    fprintf(stderr,
            "daemon_y2k38_check: started pid=%ld offset=%lld list=%s\n"
            "  until_wrap=%lld s\n",
            (long)getpid(),
            (long long)y2k38_clock_get_kernel_offset(),
            y2k38_sighup_list_resolve_path(NULL),
            (long long)y2k38_clock_seconds_until_wrap());

    do {
        y2k38_time_t now;
        unsigned interval;
        unsigned slept;
        int hit;

        now = y2k38_time(NULL);
        y2k38_format_datetime_ctl(now, ctl, sizeof(ctl));

        hit = y2k38_clock_poll_wrap();
        if (hit) {
            /* on_wrap callback already notified peers and OFFSET was saved. */
            fprintf(stderr, "daemon_y2k38_check: wrap handled at %s\n", ctl);
        }

        if (once)
            break;

        interval = adaptive_interval_sec();
        fprintf(stderr,
                "daemon_y2k38_check: utc=%s offset=%lld next_poll=%us "
                "until_wrap=%lld\n",
                ctl,
                (long long)y2k38_clock_get_kernel_offset(),
                interval,
                (long long)y2k38_clock_seconds_until_wrap());

        slept = 0;
        while (g_run && slept < interval) {
            sleep(1);
            slept++;
            /* Mid-sleep wrap nudge (SIGHUP sets session reload via library). */
            if (y2k38_clock_poll_wrap()) {
                fprintf(stderr, "daemon_y2k38_check: wrap during wait\n");
                break;
            }
        }
    } while (g_run);

    y2k38_session_exit();
    fprintf(stderr, "daemon_y2k38_check: exit\n");
    return 0;
}
