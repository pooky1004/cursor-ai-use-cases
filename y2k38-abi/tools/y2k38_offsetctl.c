/*
 * Board utility: inspect / set / calibrate / set-time for y2k38 kernel offset.
 *
 * Usage:
 *   y2k38_offsetctl show [--file PATH]
 *   y2k38_offsetctl set <offset> [--file PATH] [--notify]
 *   y2k38_offsetctl set-u32-wrap <count> [--file PATH] [--notify]
 *   y2k38_offsetctl calibrate <true_utc_epoch> [--file PATH] [--notify]
 *   y2k38_offsetctl set-time <true_utc_epoch> [--file PATH] [--notify]
 *       Sets Linux system clock (settimeofday/date) to the int32 kernel
 *       residue, saves OFFSET to /etc/y2k38_offset, notifies daemons.
 *   y2k38_offsetctl reload [--file PATH]
 *   y2k38_offsetctl sync [--file PATH] [--notify]
 *
 * Default file: /etc/y2k38_offset  (or $Y2K38_KERNEL_OFFSET_FILE)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <y2k38/time.h>

static const char *resolve_path(const char *cli_path)
{
    return y2k38_clock_resolve_offset_path(cli_path);
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s show [--file PATH]\n"
            "  %s set <offset> [--file PATH] [--notify]\n"
            "  %s set-u32-wrap <count> [--file PATH] [--notify]\n"
            "  %s calibrate <true_utc_epoch> [--file PATH] [--notify]\n"
            "  %s set-time <true_utc_epoch> [--file PATH] [--notify]\n"
            "      settimeofday/date + OFFSET file (post-2038 safe)\n"
            "  %s reload [--file PATH]\n"
            "  %s sync [--file PATH] [--notify]\n"
            "  %s notify\n",
            argv0, argv0, argv0, argv0, argv0, argv0, argv0, argv0);
}

static void run_shell_quiet(const char *cmd)
{
    int rc = system(cmd);
    (void)rc;
}

static int notify_y2k38_processes(void)
{
    /*
     * Daemons install SIGHUP → y2k38_clock_reload_offset_default().
     * Other y2k38 apps pick up mtime changes on the next y2k38_time() call.
     */
    run_shell_quiet("killall -HUP daemon_a 2>/dev/null");
    run_shell_quiet("killall -HUP daemon_b 2>/dev/null");
    fprintf(stderr, "notified: SIGHUP sent to daemon_a/daemon_b (if running)\n");
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
    if (y2k38_clock_reload_offset_default(path) != 0) {
        fprintf(stderr, "warn: saved but reload failed\n");
    }
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

    printf("offset_file = %s\n", resolve_path(path));
    printf("offset      = %lld\n",
           (long long)y2k38_clock_get_kernel_offset());
    printf("kernel_raw  = %lld\n", (long long)raw);
    printf("utc_now     = %lld (%s)\n", (long long)now, iso);
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

    raw = y2k38_time_kernel_raw(NULL);
    offset = y2k38_clock_compute_offset(true_utc, raw);
    printf("true_utc   = %lld\n", (long long)true_utc);
    printf("kernel_raw = %lld\n", (long long)raw);
    printf("offset     = %lld\n", (long long)offset);
    return persist_offset(path, offset, notify);
}

static int cmd_set_time(const char *path, y2k38_time_t true_utc, int notify)
{
    int32_t ksec;
    y2k38_time_t raw;
    y2k38_time_t offset;
    y2k38_time_t recovered;
    char iso[64];
    const char *p = resolve_path(path);

    ksec = y2k38_clock_kernel_sec_for_utc(true_utc);
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
    y2k38_format_iso8601_utc(recovered, iso, sizeof(iso));

    printf("kernel_raw  = %lld\n", (long long)raw);
    printf("offset      = %lld\n", (long long)offset);
    printf("recovered   = %lld (%s)\n", (long long)recovered, iso);
    printf("saved       = %s\n", p);

    if (recovered != true_utc) {
        fprintf(stderr,
                "WARN: recovered %lld != target %lld\n",
                (long long)recovered, (long long)true_utc);
    }

    if (notify)
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

static int cmd_sync(const char *path, int notify)
{
    if (cmd_reload(path) != 0)
        return 1;
    if (notify)
        notify_y2k38_processes();
    return 0;
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
        usage(argv[0]);
        return 1;
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

    if (strcmp(cmd, "show") == 0)
        return cmd_show(file);

    if (strcmp(cmd, "notify") == 0)
        return notify_y2k38_processes();

    if (strcmp(cmd, "reload") == 0)
        return cmd_reload(file);

    if (strcmp(cmd, "sync") == 0)
        return cmd_sync(file, notify);

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
        if (argc < 3 || y2k38_parse_epoch(argv[2], &truth) != 0) {
            usage(argv[0]);
            return 1;
        }
        if (have_mock_k) {
            fprintf(stderr,
                    "set-time: --mock-kernel not supported (needs real clock)\n");
            return 1;
        }
        return cmd_set_time(file, truth, notify);
    }

    usage(argv[0]);
    return 1;
}
