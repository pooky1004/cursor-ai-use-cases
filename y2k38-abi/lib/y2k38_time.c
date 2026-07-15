/*
 * liby2k38safe — time backend for 32-bit time_t platforms (ELDK/PPC).
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <pthread.h>

#include <y2k38/time.h>

/* From y2k38_offset.c — shared /etc/y2k38_offset mtime reload. */
int y2k38_clock_shared_offset_poll(void);

static y2k38_time_t g_kernel_offset;
static int g_mock_enabled;
static y2k38_time_t g_mock_now;
static int g_mock_kernel_enabled;
static int32_t g_mock_kernel_sec;

/* Long-running daemon: auto-detect signed 32-bit kernel wrap */
static int g_auto_wrap;
static char g_auto_wrap_persist[256];
static y2k38_time_t g_last_kernel_raw;
static int g_have_last_raw;
static unsigned g_wrap_count;
static y2k38_wrap_callback_fn g_wrap_cb;
static void *g_wrap_cb_user;
static pthread_mutex_t g_clk_mu = PTHREAD_MUTEX_INITIALIZER;

#define Y2K38_WRAP_JUMP_MIN  ((y2k38_time_t)2147483648LL) /* 2^31 — suspicious backward jump */

static y2k38_time_t read_kernel_sec_raw(void);

static int set_system_via_date_cmd(int32_t ksec)
{
    char cmd[96];
    int rc;

    snprintf(cmd, sizeof(cmd), "date -s @%lld 2>/dev/null",
             (long long)(y2k38_time_t)ksec);
    rc = system(cmd);
    if (rc == 0)
        return 0;

    snprintf(cmd, sizeof(cmd), "date -u -s @%lld",
             (long long)(y2k38_time_t)ksec);
    rc = system(cmd);
    return (rc == 0) ? 0 : -1;
}

int y2k38_clock_set_system_utc(y2k38_time_t true_utc)
{
    int32_t ksec;
    struct timeval tv;
    y2k38_time_t raw;
    y2k38_time_t offset;

    g_mock_enabled = 0;
    g_mock_kernel_enabled = 0;

    ksec = y2k38_clock_kernel_sec_for_utc(true_utc);
    tv.tv_sec = (time_t)ksec;
    tv.tv_usec = 0;

    if (settimeofday(&tv, NULL) != 0) {
        if (set_system_via_date_cmd(ksec) != 0)
            return -1;
    }

    /* Optional: sync RTC when hwclock exists (best-effort). */
    if (system("hwclock -w 2>/dev/null") != 0) {
        /* ignore */
    }

    raw = (y2k38_time_t)ksec;
    offset = y2k38_clock_compute_offset(true_utc, raw);

    pthread_mutex_lock(&g_clk_mu);
    g_kernel_offset = offset;
    g_have_last_raw = 0;
    g_last_kernel_raw = raw;
    g_have_last_raw = 1;
    pthread_mutex_unlock(&g_clk_mu);

    return 0;
}

void y2k38_clock_set_kernel_offset(y2k38_time_t offset)
{
    pthread_mutex_lock(&g_clk_mu);
    g_kernel_offset = offset;
    pthread_mutex_unlock(&g_clk_mu);
}

y2k38_time_t y2k38_clock_get_kernel_offset(void)
{
    y2k38_time_t off;
    pthread_mutex_lock(&g_clk_mu);
    off = g_kernel_offset;
    pthread_mutex_unlock(&g_clk_mu);
    return off;
}

void y2k38_clock_set_auto_wrap(int enabled, const char *persist_path)
{
    pthread_mutex_lock(&g_clk_mu);
    g_auto_wrap = enabled ? 1 : 0;
    g_auto_wrap_persist[0] = '\0';
    if (enabled && persist_path && persist_path[0]) {
        snprintf(g_auto_wrap_persist, sizeof(g_auto_wrap_persist),
                 "%s", persist_path);
    }
    if (!enabled) {
        g_have_last_raw = 0;
    }
    pthread_mutex_unlock(&g_clk_mu);
}

int y2k38_clock_get_auto_wrap(void)
{
    int v;
    pthread_mutex_lock(&g_clk_mu);
    v = g_auto_wrap;
    pthread_mutex_unlock(&g_clk_mu);
    return v;
}

unsigned y2k38_clock_get_wrap_count(void)
{
    unsigned n;
    pthread_mutex_lock(&g_clk_mu);
    n = g_wrap_count;
    pthread_mutex_unlock(&g_clk_mu);
    return n;
}

void y2k38_clock_on_wrap_callback(y2k38_wrap_callback_fn fn, void *userdata)
{
    pthread_mutex_lock(&g_clk_mu);
    g_wrap_cb = fn;
    g_wrap_cb_user = userdata;
    pthread_mutex_unlock(&g_clk_mu);
}

/*
 * Detect classic signed 32-bit overflow: raw jumps from large positive
 * (near INT32_MAX) to negative while real time moves forward.
 */
static int detect_and_apply_wrap_locked(y2k38_time_t raw)
{
    y2k38_time_t jump;
    y2k38_wrap_callback_fn cb;
    void *cb_user;
    char persist[256];

    if (!g_auto_wrap || g_mock_enabled)
        return 0;

    if (!g_have_last_raw) {
        g_last_kernel_raw = raw;
        g_have_last_raw = 1;
        return 0;
    }

    if (raw >= g_last_kernel_raw) {
        g_last_kernel_raw = raw;
        return 0;
    }

    jump = g_last_kernel_raw - raw;
    if (jump < Y2K38_WRAP_JUMP_MIN)
        return 0;

    /* One full unsigned 32-bit revolution of the kernel second counter. */
    g_kernel_offset += y2k38_clock_u32_wrap_offset(1);
    g_wrap_count += 1;
    g_last_kernel_raw = raw;

    persist[0] = '\0';
    if (g_auto_wrap_persist[0])
        snprintf(persist, sizeof(persist), "%s", g_auto_wrap_persist);

    cb = g_wrap_cb;
    cb_user = g_wrap_cb_user;

    if (persist[0])
        y2k38_clock_save_offset_file(persist, g_kernel_offset);

    if (cb)
        cb(g_kernel_offset, g_wrap_count, cb_user);

    return 1;
}

int y2k38_clock_poll_wrap(void)
{
    y2k38_time_t raw;
    int hit;

    if (g_mock_enabled)
        return 0;

    pthread_mutex_lock(&g_clk_mu);
    raw = read_kernel_sec_raw();
    hit = detect_and_apply_wrap_locked(raw);
    pthread_mutex_unlock(&g_clk_mu);
    return hit;
}

static y2k38_time_t read_and_track_raw(void)
{
    y2k38_time_t raw;

    raw = read_kernel_sec_raw();
    detect_and_apply_wrap_locked(raw);
    return raw;
}

void y2k38_clock_set_mock(int enabled, y2k38_time_t mock_now)
{
    g_mock_enabled = enabled ? 1 : 0;
    g_mock_now = mock_now;
    if (enabled)
        g_mock_kernel_enabled = 0;
}

void y2k38_clock_set_mock_kernel(int enabled, int32_t kernel_sec)
{
    g_mock_kernel_enabled = enabled ? 1 : 0;
    g_mock_kernel_sec = kernel_sec;
    if (enabled)
        g_mock_enabled = 0;
}

static y2k38_time_t read_kernel_sec_raw(void)
{
    struct timeval tv;

    if (g_mock_kernel_enabled)
        return (y2k38_time_t)g_mock_kernel_sec;

    if (gettimeofday(&tv, NULL) == 0)
        return (y2k38_time_t)tv.tv_sec;

    return (y2k38_time_t)time(NULL);
}

static y2k38_time_t apply_offset(y2k38_time_t raw)
{
    return raw + g_kernel_offset;
}

y2k38_time_t y2k38_time_kernel_raw(y2k38_time_t *tloc)
{
    y2k38_time_t raw;

    if (g_mock_enabled) {
        /*
         * Absolute mock represents recovered UTC; expose undo of offset so
         * diagnostics stay consistent when offset is also set in tests.
         */
        raw = g_mock_now - g_kernel_offset;
    } else {
        raw = read_kernel_sec_raw();
    }

    if (tloc)
        *tloc = raw;
    return raw;
}

y2k38_time_t y2k38_clock_seconds_until_wrap(void)
{
    y2k38_time_t raw;
    int32_t r32;

    /*
     * Kernel counter is always a signed 32-bit residual. After wrap it becomes
     * negative and counts -2^31 .. 2^31-1 until the next overflow.
     * Do NOT treat raw < 0 as "rem=0" — that pinned the check daemon at 1s.
     */
    raw = y2k38_time_kernel_raw(NULL);
    r32 = (int32_t)raw;
    if (r32 >= (int32_t)Y2K38_TIME_T32_MAX)
        return 0;
    return (y2k38_time_t)Y2K38_TIME_T32_MAX - (y2k38_time_t)r32;
}

y2k38_time_t y2k38_time(y2k38_time_t *tloc)
{
    y2k38_time_t now;
    y2k38_time_t raw;
    y2k38_time_t off;

    /* Ensure offset applied (flag) + SIGHUP + list registration once. */
    if (!g_mock_enabled)
        (void)y2k38_session_ensure(NULL);

    if (!g_mock_enabled)
        (void)y2k38_clock_shared_offset_poll();

    pthread_mutex_lock(&g_clk_mu);

    if (g_mock_enabled) {
        now = g_mock_now;
    } else {
        raw = read_and_track_raw();
        off = g_kernel_offset;
        now = raw + off;
    }

    pthread_mutex_unlock(&g_clk_mu);

    if (tloc)
        *tloc = now;
    return now;
}

int y2k38_gettimeofday(struct y2k38_timeval *tv)
{
    struct timeval host;

    if (!tv) {
        errno = EINVAL;
        return -1;
    }

    if (g_mock_enabled) {
        tv->tv_sec = g_mock_now;
        tv->tv_usec = 0;
        tv->__pad = 0;
        return 0;
    }

    (void)y2k38_session_ensure(NULL);
    (void)y2k38_clock_shared_offset_poll();

    if (g_mock_kernel_enabled) {
        tv->tv_sec = apply_offset((y2k38_time_t)g_mock_kernel_sec);
        tv->tv_usec = 0;
        tv->__pad = 0;
        return 0;
    }

    pthread_mutex_lock(&g_clk_mu);
    if (gettimeofday(&host, NULL) != 0) {
        pthread_mutex_unlock(&g_clk_mu);
        return -1;
    }
    {
        y2k38_time_t raw = (y2k38_time_t)host.tv_sec;
        detect_and_apply_wrap_locked(raw);
        tv->tv_sec = raw + g_kernel_offset;
    }
    pthread_mutex_unlock(&g_clk_mu);

    tv->tv_usec = (y2k38_suseconds_t)host.tv_usec;
    tv->__pad = 0;
    return 0;
}

y2k38_time_t y2k38_difftime_sec(y2k38_time_t later, y2k38_time_t earlier)
{
    return later - earlier;
}

int y2k38_timeval_diff(const struct y2k38_timeval *later,
                       const struct y2k38_timeval *earlier,
                       struct y2k38_timeval *out_diff)
{
    y2k38_time_t sec;
    int64_t usec;

    if (!later || !earlier || !out_diff) {
        errno = EINVAL;
        return -1;
    }

    sec = later->tv_sec - earlier->tv_sec;
    usec = (int64_t)later->tv_usec - (int64_t)earlier->tv_usec;
    if (usec < 0) {
        sec -= 1;
        usec += 1000000;
    }

    out_diff->tv_sec = sec;
    out_diff->tv_usec = (y2k38_suseconds_t)usec;
    out_diff->__pad = 0;
    return 0;
}

/*
 * Civil time conversion without relying on 32-bit time_t libc helpers for
 * values beyond Y2K38_TIME_T32_MAX. Algorithm: Howard Hinnant / public domain
 * style civil_from_days / days_from_civil.
 */
static void civil_from_days(int64_t z, int *y, unsigned *m, unsigned *d)
{
    int64_t era;
    unsigned doe;
    unsigned yoe;
    int64_t y_tmp;
    unsigned doy;
    unsigned mp;

    z += 719468; /* shift to civil algorithm epoch */
    era = (z >= 0 ? z : z - 146096) / 146097;
    doe = (unsigned)(z - era * 146097);
    yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y_tmp = (int64_t)yoe + era * 400;
    doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    mp = (5 * doy + 2) / 153;
    *d = (unsigned)(doy - (153 * mp + 2) / 5 + 1);
    *m = (unsigned)(mp < 10 ? mp + 3 : mp - 9);
    *y = (int)(y_tmp + (*m <= 2));
}

static int64_t days_from_civil(int y, unsigned m, unsigned d)
{
    int64_t era;
    unsigned yoe;
    unsigned doy;
    unsigned doe;
    int64_t yadj;

    yadj = y - (m <= 2);
    era = (yadj >= 0 ? yadj : yadj - 399) / 400;
    yoe = (unsigned)(yadj - era * 400);
    doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (int64_t)doe - 719468;
}

static void fill_wday_yday(struct y2k38_tm *out, int64_t days)
{
    /* 1970-01-01 was Thursday (4). */
    int64_t w = (days + 4) % 7;
    if (w < 0)
        w += 7;
    out->tm_wday = (int)w;

    {
        int y;
        unsigned m, d;
        int64_t jan1;

        civil_from_days(days, &y, &m, &d);
        jan1 = days_from_civil(y, 1, 1);
        out->tm_yday = (int)(days - jan1);
    }
}

int y2k38_gmtime_r(const y2k38_time_t *t, struct y2k38_tm *out)
{
    y2k38_time_t sec;
    int64_t days;
    int64_t rem;
    int y;
    unsigned m, d;

    if (!t || !out) {
        errno = EINVAL;
        return -1;
    }

    sec = *t;
    days = sec / 86400;
    rem = sec % 86400;
    if (rem < 0) {
        rem += 86400;
        days -= 1;
    }

    out->tm_hour = (int)(rem / 3600);
    rem %= 3600;
    out->tm_min = (int)(rem / 60);
    out->tm_sec = (int)(rem % 60);

    civil_from_days(days, &y, &m, &d);
    out->tm_year = y - 1900;
    out->tm_mon = (int)m - 1;
    out->tm_mday = (int)d;
    out->tm_isdst = 0;
    fill_wday_yday(out, days);
    return 0;
}

int y2k38_localtime_r(const y2k38_time_t *t, struct y2k38_tm *out)
{
    /*
     * For embedded PPC images without reliable post-2038 TZ in libc, treat
     * local == UTC. Sites that need TZ can plug an offset here later.
     */
    return y2k38_gmtime_r(t, out);
}

y2k38_time_t y2k38_timegm(const struct y2k38_tm *tm)
{
    int64_t days;
    y2k38_time_t sec;

    if (!tm) {
        errno = EINVAL;
        return (y2k38_time_t)-1;
    }

    days = days_from_civil(tm->tm_year + 1900,
                           (unsigned)(tm->tm_mon + 1),
                           (unsigned)tm->tm_mday);
    sec = days * 86400;
    sec += (y2k38_time_t)tm->tm_hour * 3600;
    sec += (y2k38_time_t)tm->tm_min * 60;
    sec += (y2k38_time_t)tm->tm_sec;
    return sec;
}

int y2k38_format_epoch(y2k38_time_t t, char *buf, size_t buflen)
{
    int n;

    if (!buf || buflen < 2) {
        errno = EINVAL;
        return -1;
    }
    n = snprintf(buf, buflen, "%lld", (long long)t);
    if (n < 0 || (size_t)n >= buflen)
        return -1;
    return n;
}

int y2k38_format_iso8601_utc(y2k38_time_t t, char *buf, size_t buflen)
{
    struct y2k38_tm tm;
    int n;

    if (y2k38_gmtime_r(&t, &tm) != 0)
        return -1;
    n = snprintf(buf, buflen, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (n < 0 || (size_t)n >= buflen)
        return -1;
    return n;
}

int y2k38_parse_epoch(const char *s, y2k38_time_t *out)
{
    char *end = NULL;
    long long v;

    if (!s || !out) {
        errno = EINVAL;
        return -1;
    }
    errno = 0;
    v = strtoll(s, &end, 10);
    if (end == s || errno == ERANGE)
        return -1;
    while (*end == ' ' || *end == '\t')
        end++;
    if (*end != '\0' && *end != '\n' && *end != '\r')
        return -1;
    *out = (y2k38_time_t)v;
    return 0;
}

int y2k38_format_datetime_ctl(y2k38_time_t t, char *buf, size_t buflen)
{
    struct y2k38_tm tm;
    int n;

    if (!buf || buflen < 20) {
        errno = EINVAL;
        return -1;
    }
    if (y2k38_gmtime_r(&t, &tm) != 0)
        return -1;
    n = snprintf(buf, buflen, "%04d-%02d-%02d:%02d:%02d:%02d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (n < 0 || (size_t)n >= buflen)
        return -1;
    return n;
}

int y2k38_parse_datetime_ctl(const char *s, y2k38_time_t *out)
{
    struct y2k38_tm tm;
    int y, mo, d, h, mi, sec;
    int n;
    y2k38_time_t epoch;

    if (!s || !out) {
        errno = EINVAL;
        return -1;
    }

    memset(&tm, 0, sizeof(tm));
    n = sscanf(s, "%d-%d-%d:%d:%d:%d", &y, &mo, &d, &h, &mi, &sec);
    if (n != 6)
        return -1;

    if (y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31)
        return -1;
    if (h < 0 || h > 23 || mi < 0 || mi > 59 || sec < 0 || sec > 59)
        return -1;

    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mi;
    tm.tm_sec = sec;

    epoch = y2k38_timegm(&tm);
    if (epoch < 0)
        return -1;
    *out = epoch;
    return 0;
}

int y2k38_fprint_epoch(FILE *fp, y2k38_time_t t)
{
    return fprintf(fp, "%lld", (long long)t);
}

int y2k38_is_past_time_t32_max(y2k38_time_t t)
{
    return t > Y2K38_TIME_T32_MAX;
}

static y2k38_time_t g_sleep_max_chunk = 3600;

void y2k38_sleep_set_max_chunk(y2k38_time_t max_sec)
{
    if (max_sec > 0)
        g_sleep_max_chunk = max_sec;
}

y2k38_time_t y2k38_sleep_get_max_chunk(void)
{
    return g_sleep_max_chunk;
}

int y2k38_nsleep_relative(y2k38_time_t sec, long nsec)
{
    struct timespec req;
    struct timespec rem;

    if (sec < 0 || nsec < 0) {
        errno = EINVAL;
        return -1;
    }
    while (nsec >= 1000000000L) {
        sec += 1;
        nsec -= 1000000000L;
    }

    req.tv_sec = (time_t)sec;
    req.tv_nsec = nsec;

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR)
            return -1;
        req = rem;
    }
    return 0;
}

int y2k38_sleep_until(y2k38_time_t wake_utc)
{
    y2k38_time_t now;
    y2k38_time_t rem;

    for (;;) {
        now = y2k38_time(NULL);
        rem = y2k38_difftime_sec(wake_utc, now);
        if (rem <= 0)
            return 0;

        if (rem > g_sleep_max_chunk)
            rem = g_sleep_max_chunk;

        if (y2k38_nsleep_relative(rem, 0) != 0)
            return -1;
    }
}
