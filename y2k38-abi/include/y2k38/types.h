/*
 * Y2K38 Application ABI — public types for ELDK 3.1.1 / PPC32
 *
 * time_t remains 32-bit in libc; wall-clock values that cross process,
 * file, or long-lived APIs MUST use y2k38_time_t (signed 64-bit).
 */
#ifndef Y2K38_TYPES_H
#define Y2K38_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Y2K38_ABI_VERSION_MAJOR 1
#define Y2K38_ABI_VERSION_MINOR 0

/* Seconds since 1970-01-01 00:00:00 UTC (can exceed INT32_MAX). */
typedef int64_t y2k38_time_t;

typedef int32_t y2k38_suseconds_t;
typedef int32_t y2k38_nsec_t;

/*
 * Wire / in-memory layout for PPC32 EABI (big-endian boards typical).
 * sizeof == 16, alignof(tv_sec) == 8.
 */
struct y2k38_timeval {
    y2k38_time_t      tv_sec;
    y2k38_suseconds_t tv_usec;
    int32_t           __pad;
};

struct y2k38_timespec {
    y2k38_time_t tv_sec;
    y2k38_nsec_t tv_nsec;
    int32_t      __pad;
};

/* Broken-down time (UTC or local, same field meaning as struct tm). */
struct y2k38_tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;   /* years since 1900 */
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

/* First second that does NOT fit in signed 32-bit time_t. */
#define Y2K38_TIME_T32_MAX          ((y2k38_time_t)2147483647LL)
#define Y2K38_OVERFLOW_EPOCH_SEC    ((y2k38_time_t)2147483648LL) /* 2038-01-19 03:14:08 UTC */

#ifdef __cplusplus
}
#endif

#endif /* Y2K38_TYPES_H */
