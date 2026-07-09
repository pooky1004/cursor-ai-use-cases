/*
 * liby2k38safe — time backend for 32-bit time_t platforms (ELDK/PPC).
 */
#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <y2k38/time.h>

static y2k38_time_t g_kernel_offset;
static int g_mock_enabled;
static y2k38_time_t g_mock_now;
static int g_mock_kernel_enabled;
static int32_t g_mock_kernel_sec;

void y2k38_clock_set_kernel_offset(y2k38_time_t offset)
{
    g_kernel_offset = offset;
}

y2k38_time_t y2k38_clock_get_kernel_offset(void)
{
    return g_kernel_offset;
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

y2k38_time_t y2k38_time(y2k38_time_t *tloc)
{
    y2k38_time_t now;

    if (g_mock_enabled)
        now = g_mock_now;
    else
        now = apply_offset(read_kernel_sec_raw());

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

    if (g_mock_kernel_enabled) {
        tv->tv_sec = apply_offset((y2k38_time_t)g_mock_kernel_sec);
        tv->tv_usec = 0;
        tv->__pad = 0;
        return 0;
    }

    if (gettimeofday(&host, NULL) != 0)
        return -1;

    tv->tv_sec = apply_offset((y2k38_time_t)host.tv_sec);
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

int y2k38_fprint_epoch(FILE *fp, y2k38_time_t t)
{
    return fprintf(fp, "%lld", (long long)t);
}

int y2k38_is_past_time_t32_max(y2k38_time_t t)
{
    return t > Y2K38_TIME_T32_MAX;
}
