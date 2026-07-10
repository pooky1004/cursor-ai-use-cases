/*
 * Long-running daemon pattern: start pre-2038, survive kernel wrap automatically.
 *
 * Demonstrates y2k38_clock_set_auto_wrap() — each y2k38_time() call detects
 * the signed 32-bit overflow and bumps OFFSET by 2^32 without restart.
 */
#include <stdio.h>
#include <stdint.h>

#include <y2k38/time.h>
#include <y2k38/types.h>

static void on_wrap(y2k38_time_t new_offset, unsigned count, void *user)
{
    (void)user;
    printf("WRAP detected: count=%u new_offset=%lld\n",
           count, (long long)new_offset);
}

int main(void)
{
    y2k38_time_t t0;
    y2k38_time_t t1;
    char iso[64];

    printf("=== Long-run auto-wrap demo ===\n\n");

    /* 1) Daemon startup (pre-2038) */
    y2k38_clock_apply_offset_default(NULL); /* OFFSET 0 if no file */
    y2k38_clock_on_wrap_callback(on_wrap, NULL);
    y2k38_clock_set_auto_wrap(1, "/tmp/y2k38_autowrap_offset");

    /* Simulate kernel near overflow */
    y2k38_clock_set_mock_kernel(1, (int32_t)(Y2K38_TIME_T32_MAX - 5));
    t0 = y2k38_time(NULL);
    y2k38_format_iso8601_utc(t0, iso, sizeof(iso));
    printf("before wrap: utc=%lld (%s) offset=%lld\n",
           (long long)t0, iso, (long long)y2k38_clock_get_kernel_offset());

    /* 2) Kernel wraps to signed negative (2038+1 second) — no daemon restart */
    y2k38_clock_set_mock_kernel(1, (int32_t)Y2K38_OVERFLOW_EPOCH_SEC);
    t1 = y2k38_time(NULL); /* auto-wrap fires inside this call */
    y2k38_format_iso8601_utc(t1, iso, sizeof(iso));
    printf("after wrap:  utc=%lld (%s) offset=%lld wrap_count=%u\n",
           (long long)t1, iso,
           (long long)y2k38_clock_get_kernel_offset(),
           y2k38_clock_get_wrap_count());

    if (t1 > t0 && t1 == (y2k38_time_t)Y2K38_OVERFLOW_EPOCH_SEC)
        printf("\nOK: continuous daemon would see monotonic UTC across Y2K38\n");
    else
        printf("\nFAIL: expected utc=%lld got %lld\n",
               (long long)Y2K38_OVERFLOW_EPOCH_SEC, (long long)t1);

    return (t1 == (y2k38_time_t)Y2K38_OVERFLOW_EPOCH_SEC) ? 0 : 1;
}
