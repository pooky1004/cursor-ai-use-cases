/*
 * Kernel wrap offset persistence and helpers for 32-bit time_t boards.
 */
#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <y2k38/time.h>

y2k38_time_t y2k38_clock_compute_offset(y2k38_time_t true_utc,
                                        y2k38_time_t kernel_raw)
{
    return true_utc - kernel_raw;
}

y2k38_time_t y2k38_clock_u32_wrap_offset(unsigned wrap_count)
{
    /* 2^32 == 4294967296 per full unsigned wrap of a 32-bit second counter. */
    return (y2k38_time_t)wrap_count * 4294967296LL;
}

int y2k38_clock_load_offset_file(const char *path)
{
    FILE *fp;
    char line[256];
    y2k38_time_t off = 0;
    int found = 0;

    if (!path) {
        errno = EINVAL;
        return -1;
    }

    fp = fopen(path, "r");
    if (!fp)
        return -1;

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0' || *p == '\n' || *p == '#')
            continue;

        if (strncmp(p, "OFFSET", 6) == 0) {
            p += 6;
            while (*p == ' ' || *p == '\t')
                p++;
        }

        if (y2k38_parse_epoch(p, &off) != 0) {
            fclose(fp);
            errno = EINVAL;
            return -1;
        }
        found = 1;
        break;
    }
    fclose(fp);

    if (!found) {
        errno = EINVAL;
        return -1;
    }

    y2k38_clock_set_kernel_offset(off);
    return 0;
}

int y2k38_clock_save_offset_file(const char *path, y2k38_time_t offset)
{
    FILE *fp;

    if (!path) {
        errno = EINVAL;
        return -1;
    }

    fp = fopen(path, "w");
    if (!fp)
        return -1;

    fprintf(fp, "# y2k38 kernel clock offset (seconds)\n");
    fprintf(fp, "# real_utc = sign_extend(kernel_time_t) + OFFSET\n");
    fprintf(fp, "OFFSET ");
    y2k38_fprint_epoch(fp, offset);
    fprintf(fp, "\n");
    if (fclose(fp) != 0)
        return -1;
    return 0;
}

int y2k38_clock_apply_offset_default(const char *path_or_null)
{
    const char *path = path_or_null;
    const char *env;

    if (!path) {
        env = getenv(Y2K38_OFFSET_ENV);
        if (env && env[0])
            path = env;
        else
            path = Y2K38_OFFSET_PATH_DEFAULT;
    }

    if (y2k38_clock_load_offset_file(path) == 0)
        return 0;

    if (errno == ENOENT)
        return 1; /* no file — not fatal for pre-2038 boards */
    return -1;
}
