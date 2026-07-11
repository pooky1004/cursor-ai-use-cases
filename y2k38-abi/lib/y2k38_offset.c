/*
 * Kernel wrap offset persistence and helpers for 32-bit time_t boards.
 */
#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <y2k38/time.h>

/* Shared /etc/y2k38_offset reload across threads and processes (mtime poll). */
static char g_shared_offset_path[512];
static time_t g_shared_offset_mtime;
static int g_shared_offset_enabled;
static pthread_mutex_t g_shared_mu = PTHREAD_MUTEX_INITIALIZER;

const char *y2k38_clock_resolve_offset_path(const char *path_or_null)
{
    const char *env;

    if (path_or_null && path_or_null[0])
        return path_or_null;
    env = getenv(Y2K38_OFFSET_ENV);
    if (env && env[0])
        return env;
    return Y2K38_OFFSET_PATH_DEFAULT;
}

static void shared_offset_register_locked(const char *path)
{
    struct stat st;

    if (!path || !path[0]) {
        g_shared_offset_enabled = 0;
        g_shared_offset_path[0] = '\0';
        return;
    }

    snprintf(g_shared_offset_path, sizeof(g_shared_offset_path), "%s", path);
    g_shared_offset_enabled = 1;
    g_shared_offset_mtime = 0;
    if (stat(path, &st) == 0)
        g_shared_offset_mtime = st.st_mtime;
}

void y2k38_clock_shared_offset_init(const char *path)
{
    pthread_mutex_lock(&g_shared_mu);
    shared_offset_register_locked(path);
    pthread_mutex_unlock(&g_shared_mu);
}

/*
 * Called from y2k38_time() — reload OFFSET when another process updated the file.
 * Returns 1 if reloaded, 0 if unchanged, -1 on error.
 */
int y2k38_clock_shared_offset_poll(void)
{
    struct stat st;
    const char *path;
    int reloaded = 0;

    pthread_mutex_lock(&g_shared_mu);
    if (!g_shared_offset_enabled || !g_shared_offset_path[0]) {
        pthread_mutex_unlock(&g_shared_mu);
        return 0;
    }
    path = g_shared_offset_path;
    if (stat(path, &st) != 0) {
        pthread_mutex_unlock(&g_shared_mu);
        return -1;
    }
    if (st.st_mtime != g_shared_offset_mtime) {
        g_shared_offset_mtime = st.st_mtime;
        pthread_mutex_unlock(&g_shared_mu);
        if (y2k38_clock_load_offset_file(path) == 0)
            return 1;
        return -1;
    }
    pthread_mutex_unlock(&g_shared_mu);
    return reloaded;
}

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

int32_t y2k38_clock_kernel_sec_for_utc(y2k38_time_t true_utc)
{
    return (int32_t)(uint32_t)true_utc;
}

int y2k38_clock_load_offset_file(const char *path)
{
    FILE *fp;
    char line[256];
    y2k38_time_t off = 0;
    int found = 0;
    struct stat st;

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

    pthread_mutex_lock(&g_shared_mu);
    if (g_shared_offset_enabled &&
        strcmp(g_shared_offset_path, path) == 0 &&
        stat(path, &st) == 0) {
        g_shared_offset_mtime = st.st_mtime;
    }
    pthread_mutex_unlock(&g_shared_mu);

    return 0;
}

int y2k38_clock_save_offset_file(const char *path, y2k38_time_t offset)
{
    char tmp[576];
    FILE *fp;
    struct stat st;

    if (!path) {
        errno = EINVAL;
        return -1;
    }

    snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", path, (long)getpid());
    fp = fopen(tmp, "w");
    if (!fp)
        return -1;

    fprintf(fp, "# y2k38 kernel clock offset (seconds)\n");
    fprintf(fp, "# real_utc = sign_extend(kernel_time_t) + OFFSET\n");
    fprintf(fp, "# updated by y2k38_offsetctl — shared by all y2k38 processes\n");
    fprintf(fp, "OFFSET ");
    y2k38_fprint_epoch(fp, offset);
    fprintf(fp, "\n");
    if (fclose(fp) != 0) {
        unlink(tmp);
        return -1;
    }

    if (rename(tmp, path) != 0) {
        unlink(tmp);
        return -1;
    }

    pthread_mutex_lock(&g_shared_mu);
    if (g_shared_offset_enabled &&
        strcmp(g_shared_offset_path, path) == 0 &&
        stat(path, &st) == 0) {
        g_shared_offset_mtime = st.st_mtime;
    }
    pthread_mutex_unlock(&g_shared_mu);

    return 0;
}

int y2k38_clock_apply_offset_default(const char *path_or_null)
{
    const char *path = y2k38_clock_resolve_offset_path(path_or_null);
    int st;

    y2k38_clock_shared_offset_init(path);

    if (y2k38_clock_load_offset_file(path) == 0)
        return 0;

    st = errno;
    if (st == ENOENT)
        return 1; /* no file — not fatal for pre-2038 boards */
    return -1;
}

int y2k38_clock_reload_offset_default(const char *path_or_null)
{
    const char *path = y2k38_clock_resolve_offset_path(path_or_null);

    y2k38_clock_shared_offset_init(path);
    return y2k38_clock_load_offset_file(path);
}

int y2k38_clock_set_system_and_offset(y2k38_time_t true_utc,
                                      const char *path_or_null)
{
    const char *path = y2k38_clock_resolve_offset_path(path_or_null);
    y2k38_time_t off;

    if (y2k38_clock_set_system_utc(true_utc) != 0)
        return -1;

    off = y2k38_clock_get_kernel_offset();
    if (y2k38_clock_save_offset_file(path, off) != 0)
        return -1;

    y2k38_clock_shared_offset_init(path);
    return 0;
}
