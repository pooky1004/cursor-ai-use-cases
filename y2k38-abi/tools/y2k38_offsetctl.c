/*
 * Board utility: inspect / set / calibrate y2k38 kernel clock offset.
 *
 * Usage:
 *   y2k38_offsetctl show [--file PATH]
 *   y2k38_offsetctl set <offset> [--file PATH]
 *   y2k38_offsetctl set-u32-wrap <count> [--file PATH]
 *   y2k38_offsetctl calibrate <true_utc_epoch> [--file PATH]
 *       Uses current kernel raw time (or --mock-kernel SEC) to compute offset.
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
    const char *env;

    if (cli_path && cli_path[0])
        return cli_path;
    env = getenv(Y2K38_OFFSET_ENV);
    if (env && env[0])
        return env;
    return Y2K38_OFFSET_PATH_DEFAULT;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s show [--file PATH]\n"
            "  %s set <offset> [--file PATH]\n"
            "  %s set-u32-wrap <count> [--file PATH]\n"
            "  %s calibrate <true_utc_epoch> [--file PATH] "
            "[--mock-kernel SEC]\n",
            argv0, argv0, argv0, argv0);
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

static int cmd_set(const char *path, y2k38_time_t offset)
{
    const char *p = resolve_path(path);

    if (y2k38_clock_save_offset_file(p, offset) != 0) {
        perror(p);
        return 1;
    }
    y2k38_clock_set_kernel_offset(offset);
    printf("wrote OFFSET %lld to %s\n", (long long)offset, p);
    return 0;
}

static int cmd_calibrate(const char *path, y2k38_time_t true_utc)
{
    y2k38_time_t raw;
    y2k38_time_t offset;

    raw = y2k38_time_kernel_raw(NULL);
    offset = y2k38_clock_compute_offset(true_utc, raw);
    printf("true_utc   = %lld\n", (long long)true_utc);
    printf("kernel_raw = %lld\n", (long long)raw);
    printf("offset     = %lld\n", (long long)offset);
    return cmd_set(path, offset);
}

int main(int argc, char **argv)
{
    const char *cmd;
    const char *file = NULL;
    int i;
    int32_t mock_k = 0;
    int have_mock_k = 0;

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
        }
    }

    if (have_mock_k)
        y2k38_clock_set_mock_kernel(1, mock_k);

    if (strcmp(cmd, "show") == 0)
        return cmd_show(file);

    if (strcmp(cmd, "set") == 0) {
        y2k38_time_t off;
        if (argc < 3 || y2k38_parse_epoch(argv[2], &off) != 0) {
            usage(argv[0]);
            return 1;
        }
        return cmd_set(file, off);
    }

    if (strcmp(cmd, "set-u32-wrap") == 0) {
        unsigned n;
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }
        n = (unsigned)strtoul(argv[2], NULL, 10);
        return cmd_set(file, y2k38_clock_u32_wrap_offset(n));
    }

    if (strcmp(cmd, "calibrate") == 0) {
        y2k38_time_t truth;
        if (argc < 3 || y2k38_parse_epoch(argv[2], &truth) != 0) {
            usage(argv[0]);
            return 1;
        }
        return cmd_calibrate(file, truth);
    }

    usage(argv[0]);
    return 1;
}
