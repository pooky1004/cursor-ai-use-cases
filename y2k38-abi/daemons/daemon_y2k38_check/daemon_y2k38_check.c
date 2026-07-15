/*
 * daemon_y2k38_check — monitors 32-bit kernel wrap and notifies peers.
 *
 * Responsibilities:
 *   - Manage SIGHUP subscriber list (/var/run/y2k38_sighup.list)
 *   - Register itself in that list
 *   - Poll kernel time with adaptive interval:
 *       min 1s, max 60s; based on seconds until next wrap
 *       (right after wrap, rem ≈ 2^32 → interval = 60s)
 *   - On wrap: update /etc/y2k38_offset and SIGHUP all other subscribers
 *     (never notify itself) — always printed
 *   - On SIGHUP (from offsetctl / other): reload OFFSET — always printed
 *
 * Usage:
 *   daemon_y2k38_check [--offset-file PATH] [--list PATH] [--once]
 *                      [--debug|-d]
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

#define POLL_INTERVAL_MIN_SEC  1u
#define POLL_INTERVAL_MAX_SEC  60u

static volatile sig_atomic_t g_run = 1;
static int g_debug;

static void on_term(int sig)
{
    (void)sig;
    g_run = 0;
}

#define DBG(...) \
    do { if (g_debug) fprintf(stderr, __VA_ARGS__); } while (0)

static void on_wrap(y2k38_time_t new_offset, unsigned count, void *user)
{
    int n;
    y2k38_time_t until;

    (void)user;
    /* Always print wrap events (independent of --debug). */
    until = y2k38_clock_seconds_until_wrap();
    fprintf(stderr,
            "daemon_y2k38_check: WRAP count=%u offset=%lld "
            "until_next_wrap=%lld s — notifying peers\n",
            count, (long long)new_offset, (long long)until);
    n = y2k38_sighup_list_notify((long)getpid());
    fprintf(stderr, "daemon_y2k38_check: SIGHUP sent to %d process(es)\n", n);
    fflush(stderr);
}

/* Always print peer SIGHUP / OFFSET reload. */
static void on_peer_sighup(y2k38_time_t new_offset, void *user)
{
    (void)user;
    fprintf(stderr,
            "daemon_y2k38_check: received SIGHUP — reloaded offset=%lld "
            "until_next_wrap=%lld s\n",
            (long long)new_offset,
            (long long)y2k38_clock_seconds_until_wrap());
    fflush(stderr);
}

/*
 * Adaptive poll interval in [1, 60] seconds from time until next wrap.
 * Far from wrap (including just after a wrap, rem ≈ 2^32) → 60s.
 * Near wrap → down to 1s.
 */
static unsigned adaptive_interval_sec(void)
{
    y2k38_time_t rem = y2k38_clock_seconds_until_wrap();
    unsigned interval;

    if (rem <= 0)
        interval = POLL_INTERVAL_MIN_SEC;
    else if (rem <= 30)
        interval = 1;
    else if (rem <= 120)
        interval = 5;
    else if (rem <= 600)
        interval = 15;
    else if (rem <= 3600)
        interval = 30;
    else
        interval = POLL_INTERVAL_MAX_SEC; /* wrap just happened or years away */

    if (interval < POLL_INTERVAL_MIN_SEC)
        interval = POLL_INTERVAL_MIN_SEC;
    if (interval > POLL_INTERVAL_MAX_SEC)
        interval = POLL_INTERVAL_MAX_SEC;
    return interval;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [--offset-file PATH] [--list PATH] [--once] "
            "[--debug|-d]\n"
            "  Monitors kernel wrap, updates OFFSET, SIGHUP subscribers.\n"
            "  Poll interval: 1..60 s from time until next wrap.\n"
            "  --debug: print periodic status (wrap/SIGHUP always printed).\n"
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
        } else if (strcmp(argv[i], "--debug") == 0 ||
                   strcmp(argv[i], "-d") == 0) {
            g_debug = 1;
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

    if (list_file)
        setenv(Y2K38_SIGHUP_LIST_ENV, list_file, 1);
    if (offset_file)
        setenv(Y2K38_OFFSET_ENV, offset_file, 1);

    signal(SIGINT, on_term);
    signal(SIGTERM, on_term);

    if (y2k38_session_init("daemon_y2k38_check") != 0) {
        fprintf(stderr, "daemon_y2k38_check: session_init failed\n");
        return 1;
    }

    y2k38_clock_on_wrap_callback(on_wrap, NULL);
    y2k38_session_on_sighup_callback(on_peer_sighup, NULL);
    y2k38_clock_set_auto_wrap(1, y2k38_clock_resolve_offset_path(NULL));

    fprintf(stderr,
            "daemon_y2k38_check: started pid=%ld offset=%lld list=%s "
            "debug=%d\n"
            "  until_wrap=%lld s  (poll interval 1..60 s)\n",
            (long)getpid(),
            (long long)y2k38_clock_get_kernel_offset(),
            y2k38_sighup_list_resolve_path(NULL),
            g_debug,
            (long long)y2k38_clock_seconds_until_wrap());

    do {
        y2k38_time_t now;
        y2k38_time_t until;
        unsigned interval;
        unsigned slept;
        int hit;

        now = y2k38_time(NULL);
        y2k38_format_datetime_ctl(now, ctl, sizeof(ctl));

        /* Drain peer SIGHUP (callback always prints). */
        (void)y2k38_session_poll_sighup();

        hit = y2k38_clock_poll_wrap();
        if (hit) {
            /* on_wrap already printed; recompute interval for next sleep. */
            fprintf(stderr,
                    "daemon_y2k38_check: wrap handled at %s "
                    "until_next_wrap=%lld s\n",
                    ctl,
                    (long long)y2k38_clock_seconds_until_wrap());
            fflush(stderr);
        }

        if (once)
            break;

        until = y2k38_clock_seconds_until_wrap();
        interval = adaptive_interval_sec();
        DBG("daemon_y2k38_check: utc=%s offset=%lld next_poll=%us "
            "until_wrap=%lld\n",
            ctl,
            (long long)y2k38_clock_get_kernel_offset(),
            interval,
            (long long)until);

        slept = 0;
        while (g_run && slept < interval) {
            unsigned chunk;

            /*
             * Near wrap: wake every 1s. Far from wrap: larger chunks up to
             * remaining interval so we do not busy-poll every second for
             * decades after a wrap.
             */
            if (until <= 60)
                chunk = 1;
            else if (interval - slept > 10)
                chunk = 10;
            else
                chunk = interval - slept;
            if (chunk < 1)
                chunk = 1;

            sleep(chunk);
            slept += chunk;

            (void)y2k38_session_poll_sighup();
            until = y2k38_clock_seconds_until_wrap();
            if (y2k38_clock_poll_wrap()) {
                fprintf(stderr, "daemon_y2k38_check: wrap during wait\n");
                fflush(stderr);
                break;
            }
            /* Re-clamp remaining sleep if wrap got closer unexpectedly. */
            if (until <= 30 && (interval - slept) > 1)
                break;
        }
    } while (g_run);

    y2k38_session_exit();
    fprintf(stderr, "daemon_y2k38_check: exit\n");
    return 0;
}
