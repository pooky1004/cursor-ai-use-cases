/*
 * Y2K38-safe time API (userspace ABI for 32-bit time_t platforms).
 */
#ifndef Y2K38_TIME_H
#define Y2K38_TIME_H

#include <stddef.h>
#include <stdio.h>

#include <y2k38/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- clock ------------------------------------------------------------- */

/*
 * Return current wall time as y2k38_time_t.
 * Uses gettimeofday and widens tv_sec. If a kernel wrap offset was set via
 * y2k38_clock_set_kernel_offset(), it is applied.
 * If tloc != NULL, also stores the result there (like time()).
 */
y2k38_time_t y2k38_time(y2k38_time_t *tloc);

int y2k38_gettimeofday(struct y2k38_timeval *tv);

/*
 * When the 32-bit kernel clock has wrapped or been reset, apps publish an
 * adjustment so: real_utc = sign_extend(kernel_sec) + offset.
 * Classic unsigned wrap recovery uses offset == 2^32 (4294967296).
 */
void y2k38_clock_set_kernel_offset(y2k38_time_t offset);
y2k38_time_t y2k38_clock_get_kernel_offset(void);

/* Default path and env var for durable offset on the board. */
#define Y2K38_OFFSET_PATH_DEFAULT   "/etc/y2k38_offset"
#define Y2K38_OFFSET_ENV            "Y2K38_KERNEL_OFFSET_FILE"

/*
 * Raw kernel seconds widened to 64-bit WITHOUT applying g_kernel_offset.
 * Useful when computing a new offset from a trusted UTC source.
 */
y2k38_time_t y2k38_time_kernel_raw(y2k38_time_t *tloc);

/* offset = true_utc - kernel_raw (both as y2k38_time_t). */
y2k38_time_t y2k38_clock_compute_offset(y2k38_time_t true_utc,
                                        y2k38_time_t kernel_raw);

/* Single unsigned 32-bit wrap: 2^32. */
y2k38_time_t y2k38_clock_u32_wrap_offset(unsigned wrap_count);

/*
 * File format (text):
 *   # comment
 *   OFFSET <int64>
 * or a single decimal line with the offset.
 * load applies via y2k38_clock_set_kernel_offset().
 */
int y2k38_clock_load_offset_file(const char *path);
int y2k38_clock_save_offset_file(const char *path, y2k38_time_t offset);

/*
 * Load offset from path, or from getenv(Y2K38_OFFSET_ENV), or
 * Y2K38_OFFSET_PATH_DEFAULT. Returns 0 on success, 1 if no file, -1 on error.
 */
int y2k38_clock_apply_offset_default(const char *path_or_null);

/* Test/demo: force absolute "now" (bypasses kernel + offset). */
void y2k38_clock_set_mock(int enabled, y2k38_time_t mock_now);

/*
 * Test/demo: force the raw 32-bit kernel second (signed). Offset is still
 * applied. Cleared when absolute mock is enabled.
 */
void y2k38_clock_set_mock_kernel(int enabled, int32_t kernel_sec);

/* ---- arithmetic (no 32-bit truncation) --------------------------------- */

/* later - earlier (seconds). */
y2k38_time_t y2k38_difftime_sec(y2k38_time_t later, y2k38_time_t earlier);

int y2k38_timeval_diff(const struct y2k38_timeval *later,
                       const struct y2k38_timeval *earlier,
                       struct y2k38_timeval *out_diff);

/* ---- conversion / formatting ------------------------------------------- */

int y2k38_gmtime_r(const y2k38_time_t *t, struct y2k38_tm *out);
int y2k38_localtime_r(const y2k38_time_t *t, struct y2k38_tm *out);

/* Convert calendar fields (UTC) to epoch seconds. Returns -1 on error. */
y2k38_time_t y2k38_timegm(const struct y2k38_tm *tm);

/* Format into buf; returns bytes written (excluding NUL) or -1. */
int y2k38_format_epoch(y2k38_time_t t, char *buf, size_t buflen);
int y2k38_format_iso8601_utc(y2k38_time_t t, char *buf, size_t buflen);

/* Parse signed decimal epoch seconds (full 64-bit). Returns 0 on success. */
int y2k38_parse_epoch(const char *s, y2k38_time_t *out);

/*
 * Write/read durable log fields — always 64-bit decimal, never %ld time_t.
 * Line helpers used by daemons.
 */
int y2k38_fprint_epoch(FILE *fp, y2k38_time_t t);

/* Detect if a (possibly narrowed) 32-bit value is past the signed max. */
int y2k38_is_past_time_t32_max(y2k38_time_t t);

#ifdef __cplusplus
}
#endif

#endif /* Y2K38_TIME_H */
