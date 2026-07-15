/*

 * Board utility: inspect / set / calibrate / set-time for y2k38 kernel offset.

 *

 * Default file: /etc/y2k38_offset  (or $Y2K38_KERNEL_OFFSET_FILE)

 * SIGHUP list:  /var/run/y2k38_sighup.list (or $Y2K38_SIGHUP_LIST_FILE)

 */

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <stdint.h>

#include <unistd.h>



#include <y2k38/time.h>



static const char *resolve_path(const char *cli_path)

{

    return y2k38_clock_resolve_offset_path(cli_path);

}



static void print_help(const char *argv0, FILE *out)

{

    fprintf(out,

            "y2k38_offsetctl — Y2K38 kernel offset and wall-clock tool\n"

            "\n"

            "Usage:\n"

            "  %s show [--file PATH]\n"

            "  %s get-time [--file PATH]\n"

            "  %s set <offset> [--file PATH] [--notify]\n"

            "  %s set-u32-wrap <count> [--file PATH] [--notify]\n"

            "  %s calibrate <true_utc_epoch> [--file PATH] [--notify]\n"

            "  %s set-time <YYYY-MM-DD:hh:mm:ss> [--file PATH]\n"

            "  %s reload [--file PATH]\n"

            "  %s sync [--file PATH]\n"

            "  %s notify\n"

            "\n"

            "Options:\n"

            "  --file PATH       offset file (default: /etc/y2k38_offset or\n"

            "                    $Y2K38_KERNEL_OFFSET_FILE)\n"

            "  --notify          SIGHUP all pids in SIGHUP list (exclude self)\n"

            "  --mock-kernel SEC test only: emulate signed 32-bit kernel second\n"

            "\n"

            "Commands:\n"

            "  show              offset, kernel raw, recovered UTC\n"

            "  get-time          print current UTC as YYYY-MM-DD:hh:mm:ss\n"

            "  set-time          set system+HW clock, OFFSET file, SIGHUP list\n"

            "  calibrate         OFFSET = true_utc - kernel_raw (epoch input)\n"

            "  set               write OFFSET value to file\n"

            "  set-u32-wrap      OFFSET = count * 2^32\n"

            "  reload            reload OFFSET into this process\n"

            "  sync              reload + SIGHUP all subscribers in list\n"

            "  notify            SIGHUP all pids in %s (exclude self)\n"

            "\n"

            "Time format (UTC): YYYY-MM-DD:hh:mm:ss  e.g. 2038-01-19:03:15:48\n"

            "\n"

            "Formula:  real_utc = sign_extend(kernel_time_t) + OFFSET\n",

            argv0, argv0, argv0, argv0, argv0, argv0, argv0, argv0, argv0,

            Y2K38_SIGHUP_LIST_PATH_DEFAULT);

}



static void usage(const char *argv0)

{

    print_help(argv0, stderr);

}



/*

 * Notify every subscriber in the Check-daemon SIGHUP list, except this process.

 * Replaces killall of hard-coded daemon names.

 */

static int notify_y2k38_processes(void)

{

    int n;



    n = y2k38_sighup_list_notify((long)getpid());

    if (n < 0) {

        perror(y2k38_sighup_list_resolve_path(NULL));

        return 1;

    }

    fprintf(stderr,

            "notified: SIGHUP sent to %d process(es) via %s\n",

            n, y2k38_sighup_list_resolve_path(NULL));

    return 0;

}



static int persist_offset(const char *path, y2k38_time_t offset, int notify)

{

    const char *p = resolve_path(path);



    if (y2k38_clock_save_offset_file(p, offset) != 0) {

        perror(p);

        return 1;

    }

    y2k38_clock_set_kernel_offset(offset);

    if (y2k38_clock_reload_offset_default(path) != 0)

        fprintf(stderr, "warn: saved but reload failed\n");

    printf("wrote OFFSET %lld to %s\n", (long long)offset, p);

    if (notify)

        notify_y2k38_processes();

    return 0;

}



static int cmd_show(const char *path)

{

    y2k38_time_t raw;

    y2k38_time_t now;

    char iso[64];

    char ctl[64];

    int st;



    st = y2k38_clock_apply_offset_default(path);

    if (st < 0) {

        fprintf(stderr, "failed to load %s\n", resolve_path(path));

        return 1;

    }

    if (st > 0)

        fprintf(stderr, "note: no offset file at %s (offset=0)\n",

                resolve_path(path));



    raw = y2k38_time_kernel_raw(NULL);

    now = y2k38_time(NULL);

    y2k38_format_iso8601_utc(now, iso, sizeof(iso));

    y2k38_format_datetime_ctl(now, ctl, sizeof(ctl));



    printf("offset_file = %s\n", resolve_path(path));

    printf("offset      = %lld\n",

           (long long)y2k38_clock_get_kernel_offset());

    printf("kernel_raw  = %lld\n", (long long)raw);

    printf("utc_now     = %s\n", ctl);

    printf("epoch       = %lld (%s)\n", (long long)now, iso);

    printf("session     = %s\n",

           y2k38_session_is_ready() ? "ready" : "not-ready");

    return 0;

}



static int cmd_get_time(const char *path)

{

    y2k38_time_t now;

    char ctl[64];

    int st;



    st = y2k38_clock_apply_offset_default(path);

    if (st < 0) {

        fprintf(stderr, "failed to load %s\n", resolve_path(path));

        return 1;

    }



    now = y2k38_time(NULL);

    if (y2k38_format_datetime_ctl(now, ctl, sizeof(ctl)) < 0) {

        fprintf(stderr, "format failed\n");

        return 1;

    }

    printf("%s\n", ctl);

    return 0;

}



static int cmd_set(const char *path, y2k38_time_t offset, int notify)

{

    return persist_offset(path, offset, notify);

}



static int cmd_calibrate(const char *path, y2k38_time_t true_utc, int notify)

{

    y2k38_time_t raw;

    y2k38_time_t offset;

    char ctl[64];



    raw = y2k38_time_kernel_raw(NULL);

    offset = y2k38_clock_compute_offset(true_utc, raw);

    y2k38_format_datetime_ctl(true_utc, ctl, sizeof(ctl));

    printf("true_utc   = %lld (%s)\n", (long long)true_utc, ctl);

    printf("kernel_raw = %lld\n", (long long)raw);

    printf("offset     = %lld\n", (long long)offset);

    return persist_offset(path, offset, notify);

}



/*

 * set-time: system clock + HW (hwclock) + OFFSET file + SIGHUP list (always).

 */

static int cmd_set_time(const char *path, y2k38_time_t true_utc)

{

    int32_t ksec;

    y2k38_time_t raw;

    y2k38_time_t offset;

    y2k38_time_t recovered;

    char ctl[64];

    char iso[64];

    const char *p = resolve_path(path);



    y2k38_format_datetime_ctl(true_utc, ctl, sizeof(ctl));

    ksec = y2k38_clock_kernel_sec_for_utc(true_utc);

    printf("target      = %s\n", ctl);

    printf("target_utc  = %lld\n", (long long)true_utc);

    printf("kernel_set  = %ld (int32 residue for settimeofday/date)\n",

           (long)ksec);



    if (y2k38_clock_set_system_and_offset(true_utc, path) != 0) {

        perror("set-time");

        fprintf(stderr,

                "hint: run as root; needs settimeofday(2) or date(1)\n");

        return 1;

    }



    raw = y2k38_time_kernel_raw(NULL);

    offset = y2k38_clock_get_kernel_offset();

    recovered = y2k38_time(NULL);

    y2k38_format_datetime_ctl(recovered, ctl, sizeof(ctl));

    y2k38_format_iso8601_utc(recovered, iso, sizeof(iso));



    printf("kernel_raw  = %lld\n", (long long)raw);

    printf("offset      = %lld\n", (long long)offset);

    printf("recovered   = %s\n", ctl);

    printf("epoch       = %lld (%s)\n", (long long)recovered, iso);

    printf("saved       = %s\n", p);



    if (recovered != true_utc) {

        fprintf(stderr,

                "WARN: recovered %lld != target %lld\n",

                (long long)recovered, (long long)true_utc);

    }



    /* Always notify SIGHUP-list subscribers (exclude offsetctl itself). */

    notify_y2k38_processes();

    return (recovered == true_utc) ? 0 : 1;

}



static int cmd_reload(const char *path)

{

    const char *p = resolve_path(path);



    if (y2k38_clock_reload_offset_default(path) != 0) {

        perror(p);

        return 1;

    }

    printf("reloaded OFFSET %lld from %s\n",

           (long long)y2k38_clock_get_kernel_offset(), p);

    return 0;

}



/*

 * sync: reload local OFFSET, then SIGHUP everyone on the subscriber list.

 */

static int cmd_sync(const char *path)

{

    if (cmd_reload(path) != 0)

        return 1;

    return notify_y2k38_processes();

}



int main(int argc, char **argv)

{

    const char *cmd;

    const char *file = NULL;

    int i;

    int32_t mock_k = 0;

    int have_mock_k = 0;

    int notify = 0;



    if (argc < 2) {

        print_help(argv[0], stdout);

        return 0;

    }



    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {

        print_help(argv[0], stdout);

        return 0;

    }



    cmd = argv[1];

    for (i = 2; i < argc; i++) {

        if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {

            file = argv[++i];

        } else if (strcmp(argv[i], "--mock-kernel") == 0 && i + 1 < argc) {

            mock_k = (int32_t)strtoll(argv[++i], NULL, 10);

            have_mock_k = 1;

        } else if (strcmp(argv[i], "--notify") == 0) {

            notify = 1;

        }

    }



    if (have_mock_k)

        y2k38_clock_set_mock_kernel(1, mock_k);



    if (strcmp(cmd, "help") == 0) {

        print_help(argv[0], stdout);

        return 0;

    }



    if (strcmp(cmd, "show") == 0)

        return cmd_show(file);



    if (strcmp(cmd, "get-time") == 0)

        return cmd_get_time(file);



    if (strcmp(cmd, "notify") == 0)

        return notify_y2k38_processes();



    if (strcmp(cmd, "reload") == 0)

        return cmd_reload(file);



    if (strcmp(cmd, "sync") == 0)

        return cmd_sync(file);



    if (strcmp(cmd, "set") == 0) {

        y2k38_time_t off;

        if (argc < 3 || y2k38_parse_epoch(argv[2], &off) != 0) {

            usage(argv[0]);

            return 1;

        }

        return cmd_set(file, off, notify);

    }



    if (strcmp(cmd, "set-u32-wrap") == 0) {

        unsigned n;

        if (argc < 3) {

            usage(argv[0]);

            return 1;

        }

        n = (unsigned)strtoul(argv[2], NULL, 10);

        return cmd_set(file, y2k38_clock_u32_wrap_offset(n), notify);

    }



    if (strcmp(cmd, "calibrate") == 0) {

        y2k38_time_t truth;

        if (argc < 3 || y2k38_parse_epoch(argv[2], &truth) != 0) {

            usage(argv[0]);

            return 1;

        }

        return cmd_calibrate(file, truth, notify);

    }



    if (strcmp(cmd, "set-time") == 0) {

        y2k38_time_t truth;

        if (argc < 3) {

            usage(argv[0]);

            return 1;

        }

        if (y2k38_parse_datetime_ctl(argv[2], &truth) != 0) {

            fprintf(stderr,

                    "bad time: use YYYY-MM-DD:hh:mm:ss (UTC), "

                    "e.g. 2038-01-19:03:15:48\n");

            return 1;

        }

        if (have_mock_k) {

            fprintf(stderr,

                    "set-time: --mock-kernel not supported (needs real clock)\n");

            return 1;

        }

        return cmd_set_time(file, truth);

    }



    usage(argv[0]);

    return 1;

}


