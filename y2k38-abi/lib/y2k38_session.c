/*
 * Process session: ready-flag, SIGHUP handler, SIGHUP subscriber list.
 *
 * On first library clock use (or y2k38_session_init):
 *   1) register SIGHUP → reload /etc/y2k38_offset
 *   2) add this process to the SIGHUP list (daemon_y2k38_check manages it)
 *   3) load offset file and set ready flag
 *   4) enable auto-wrap
 *
 * On exit: remove from list, restore previous SIGHUP handler.
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <y2k38/time.h>

/* File format: one line per subscriber — "<name> <pid>\n" */

static volatile sig_atomic_t g_ready;
static volatile sig_atomic_t g_sighup_pending;
static char g_proc_name[Y2K38_SIGHUP_LIST_NAME_MAX];
static int g_atexit_registered;
static int g_handler_installed;
static struct sigaction g_old_sighup;
static pthread_mutex_t g_sess_mu = PTHREAD_MUTEX_INITIALIZER;
static y2k38_sighup_callback_fn g_sighup_cb;
static void *g_sighup_cb_user;

void y2k38_clock_shared_offset_init(const char *path);

const char *y2k38_sighup_list_resolve_path(const char *path_or_null)
{
    const char *env;

    if (path_or_null && path_or_null[0])
        return path_or_null;
    env = getenv(Y2K38_SIGHUP_LIST_ENV);
    if (env && env[0])
        return env;
    return Y2K38_SIGHUP_LIST_PATH_DEFAULT;
}

static int ensure_parent_dir(const char *path)
{
    char dir[512];
    char *slash;
    size_t n;

    n = strlen(path);
    if (n >= sizeof(dir))
        return -1;
    memcpy(dir, path, n + 1);
    slash = strrchr(dir, '/');
    if (!slash || slash == dir)
        return 0;
    *slash = '\0';
    if (mkdir(dir, 0755) != 0 && errno != EEXIST)
        return -1;
    return 0;
}

static int list_lock(int fd)
{
#if defined(LOCK_EX)
    return flock(fd, LOCK_EX);
#else
    (void)fd;
    return 0;
#endif
}

static void list_unlock(int fd)
{
#if defined(LOCK_UN)
    flock(fd, LOCK_UN);
#else
    (void)fd;
#endif
}

int y2k38_sighup_list_add(const char *proc_name, long pid)
{
    const char *path = y2k38_sighup_list_resolve_path(NULL);
    char tmp[576];
    char line[128];
    FILE *in;
    FILE *out;
    int fd;
    int found = 0;

    if (!proc_name || !proc_name[0] || pid <= 0) {
        errno = EINVAL;
        return -1;
    }

    if (ensure_parent_dir(path) != 0)
        return -1;

    snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", path, (long)getpid());
    out = fopen(tmp, "w");
    if (!out)
        return -1;

    fd = fileno(out);
    if (list_lock(fd) != 0) {
        fclose(out);
        unlink(tmp);
        return -1;
    }

    fprintf(out, "# y2k38 SIGHUP subscriber list — name pid\n");
    in = fopen(path, "r");
    if (in) {
        while (fgets(line, sizeof(line), in)) {
            char name[Y2K38_SIGHUP_LIST_NAME_MAX];
            long p = 0;

            if (line[0] == '#' || line[0] == '\n')
                continue;
            if (sscanf(line, "%63s %ld", name, &p) != 2)
                continue;
            if (p == pid) {
                found = 1;
                fprintf(out, "%s %ld\n", proc_name, pid);
            } else if (p > 0) {
                /* Drop dead pids when we can detect them. */
                if (kill((pid_t)p, 0) == 0 || errno == EPERM)
                    fprintf(out, "%s %ld\n", name, p);
            }
        }
        fclose(in);
    }
    if (!found)
        fprintf(out, "%s %ld\n", proc_name, pid);

    fflush(out);
    list_unlock(fd);
    if (fclose(out) != 0) {
        unlink(tmp);
        return -1;
    }
    if (rename(tmp, path) != 0) {
        unlink(tmp);
        return -1;
    }
    return 0;
}

int y2k38_sighup_list_remove(long pid)
{
    const char *path = y2k38_sighup_list_resolve_path(NULL);
    char tmp[576];
    char line[128];
    FILE *in;
    FILE *out;
    int fd;
    int changed = 0;

    if (pid <= 0) {
        errno = EINVAL;
        return -1;
    }

    in = fopen(path, "r");
    if (!in) {
        if (errno == ENOENT)
            return 0;
        return -1;
    }

    snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", path, (long)getpid());
    out = fopen(tmp, "w");
    if (!out) {
        fclose(in);
        return -1;
    }

    fd = fileno(out);
    list_lock(fd);
    fprintf(out, "# y2k38 SIGHUP subscriber list — name pid\n");
    while (fgets(line, sizeof(line), in)) {
        char name[Y2K38_SIGHUP_LIST_NAME_MAX];
        long p = 0;

        if (line[0] == '#' || line[0] == '\n')
            continue;
        if (sscanf(line, "%63s %ld", name, &p) != 2)
            continue;
        if (p == pid) {
            changed = 1;
            continue;
        }
        if (p > 0 && (kill((pid_t)p, 0) == 0 || errno == EPERM))
            fprintf(out, "%s %ld\n", name, p);
        else
            changed = 1;
    }
    fclose(in);
    fflush(out);
    list_unlock(fd);
    if (fclose(out) != 0) {
        unlink(tmp);
        return -1;
    }
    if (rename(tmp, path) != 0) {
        unlink(tmp);
        return -1;
    }
    (void)changed;
    return 0;
}

int y2k38_sighup_list_notify(long exclude_pid)
{
    const char *path = y2k38_sighup_list_resolve_path(NULL);
    char line[128];
    FILE *fp;
    int sent = 0;

    fp = fopen(path, "r");
    if (!fp) {
        if (errno == ENOENT)
            return 0;
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        char name[Y2K38_SIGHUP_LIST_NAME_MAX];
        long p = 0;

        if (line[0] == '#' || line[0] == '\n')
            continue;
        if (sscanf(line, "%63s %ld", name, &p) != 2)
            continue;
        if (p <= 0 || p == exclude_pid)
            continue;
        if (kill((pid_t)p, SIGHUP) == 0)
            sent++;
    }
    fclose(fp);
    return sent;
}

static void on_session_sighup(int sig)
{
    (void)sig;
    g_sighup_pending = 1;
}

void y2k38_session_on_sighup_callback(y2k38_sighup_callback_fn fn, void *userdata)
{
    pthread_mutex_lock(&g_sess_mu);
    g_sighup_cb = fn;
    g_sighup_cb_user = userdata;
    pthread_mutex_unlock(&g_sess_mu);
}

/*
 * Reload OFFSET after peer SIGHUP; invoke optional user callback.
 * Returns 1 if a pending SIGHUP was consumed.
 */
int y2k38_session_poll_sighup(void)
{
    y2k38_sighup_callback_fn cb;
    void *cb_user;
    y2k38_time_t off;

    if (!g_sighup_pending)
        return 0;

    g_sighup_pending = 0;
    (void)y2k38_clock_reload_offset_default(NULL);

    pthread_mutex_lock(&g_sess_mu);
    cb = g_sighup_cb;
    cb_user = g_sighup_cb_user;
    pthread_mutex_unlock(&g_sess_mu);

    off = y2k38_clock_get_kernel_offset();
    if (cb)
        cb(off, cb_user);
    return 1;
}

static void drain_sighup_reload(void)
{
    (void)y2k38_session_poll_sighup();
}

int y2k38_session_is_ready(void)
{
    return g_ready ? 1 : 0;
}

void y2k38_session_exit(void)
{
    pthread_mutex_lock(&g_sess_mu);

    if (g_handler_installed) {
        sigaction(SIGHUP, &g_old_sighup, NULL);
        g_handler_installed = 0;
    }

    if (g_ready)
        (void)y2k38_sighup_list_remove((long)getpid());

    g_ready = 0;
    g_proc_name[0] = '\0';
    pthread_mutex_unlock(&g_sess_mu);
}

static void session_atexit(void)
{
    y2k38_session_exit();
}

int y2k38_session_ensure(const char *proc_name)
{
    struct sigaction sa;
    const char *name;
    int st;

    drain_sighup_reload();

    if (g_ready)
        return 0;

    pthread_mutex_lock(&g_sess_mu);
    if (g_ready) {
        pthread_mutex_unlock(&g_sess_mu);
        return 0;
    }

    name = (proc_name && proc_name[0]) ? proc_name : NULL;
    if (name)
        snprintf(g_proc_name, sizeof(g_proc_name), "%s", name);
    else if (!g_proc_name[0])
        snprintf(g_proc_name, sizeof(g_proc_name), "%s", "y2k38_app");

    /* 1) Register SIGHUP handler (reload offset on notify). */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_session_sighup;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, &g_old_sighup) == 0)
        g_handler_installed = 1;

    /* 2) Add this process to the Check-daemon SIGHUP list. */
    (void)y2k38_sighup_list_add(g_proc_name, (long)getpid());

    /* 3) Load /etc/y2k38_offset (or env override) and share mtime. */
    st = y2k38_clock_apply_offset_default(NULL);
    if (st < 0) {
        /* Bad file — still mark ready with OFFSET 0 after shared init. */
        y2k38_clock_shared_offset_init(
            y2k38_clock_resolve_offset_path(NULL));
        y2k38_clock_set_kernel_offset(0);
    }

    /* 4) Enable wrap detect on every y2k38_time(); persist OFFSET file. */
    y2k38_clock_set_auto_wrap(1, y2k38_clock_resolve_offset_path(NULL));

    g_ready = 1;

    if (!g_atexit_registered) {
        atexit(session_atexit);
        g_atexit_registered = 1;
    }

    pthread_mutex_unlock(&g_sess_mu);
    return 0;
}

int y2k38_session_init(const char *proc_name)
{
    return y2k38_session_ensure(proc_name);
}
