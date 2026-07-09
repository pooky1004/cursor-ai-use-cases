/*
 * Demonstrate kernel-clock wrap recovery via y2k38 offset.
 *
 * Scenario (typical after signed 32-bit overflow if HW still ticks):
 *   true UTC        = INT32_MAX + N
 *   kernel time_t   = (int32_t)(true)  → negative
 *   recovered       = kernel_raw + offset
 *
 * For a full unsigned 32-bit wrap of the second counter:
 *   offset = 2^32 = 4294967296
 *   or compute: offset = true_utc - kernel_raw
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <y2k38/time.h>
#include <y2k38/types.h>

static int fail(const char *msg)
{
    fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
}

int main(void)
{
    const char *path = "demo_y2k38_offset.conf";
    y2k38_time_t true_utc;
    y2k38_time_t raw;
    y2k38_time_t offset;
    y2k38_time_t recovered;
    y2k38_time_t future;
    y2k38_time_t delta;
    char iso[64];
    int rc = 0;

    printf("=== Kernel offset recovery demo ===\n\n");

    /*
     * 2038-01-19 03:14:08 UTC + 100s  →  beyond signed 32-bit.
     * Emulate kernel returning the wrapped signed residual.
     */
    true_utc = Y2K38_OVERFLOW_EPOCH_SEC + 100; /* 2147483748 */
    {
        int32_t wrapped = (int32_t)(uint32_t)true_utc;
        printf("true UTC          = %lld\n", (long long)true_utc);
        printf("kernel as int32   = %ld  (emulated wrap)\n", (long)wrapped);

        y2k38_clock_set_kernel_offset(0);
        y2k38_clock_set_mock_kernel(1, wrapped);
        raw = y2k38_time_kernel_raw(NULL);
        printf("y2k38 kernel raw  = %lld (no offset yet)\n", (long long)raw);

        if (y2k38_time(NULL) == true_utc)
            return fail("unexpected: recovered without offset");

        offset = y2k38_clock_compute_offset(true_utc, raw);
        printf("computed offset   = %lld\n", (long long)offset);

        /* Also show classic single u32 wrap constant relation when applicable */
        printf("u32 wrap x1       = %lld\n",
               (long long)y2k38_clock_u32_wrap_offset(1));

        y2k38_clock_set_kernel_offset(offset);
        if (y2k38_clock_save_offset_file(path, offset) != 0)
            return fail("save offset file");
        printf("saved %s\n", path);

        /* Reload as daemons would on boot */
        y2k38_clock_set_kernel_offset(0);
        if (y2k38_clock_load_offset_file(path) != 0)
            return fail("load offset file");

        recovered = y2k38_time(NULL);
        y2k38_format_iso8601_utc(recovered, iso, sizeof(iso));
        printf("recovered UTC     = %lld (%s)\n", (long long)recovered, iso);

        if (recovered != true_utc) {
            printf("mismatch recovered=%lld true=%lld\n",
                   (long long)recovered, (long long)true_utc);
            rc = fail("recovered time != true UTC");
        } else {
            printf("OK: wrap recovered via OFFSET file\n");
        }

        /* Daemon B style delta to a far-future schedule must stay sane */
        future = 4102444800LL; /* 2100-01-01 */
        delta = y2k38_difftime_sec(future, recovered);
        printf("delta to 2100-01-01 = %lld s\n", (long long)delta);
        if (delta <= 0)
            rc = fail("future delta should be positive");
        else
            printf("OK: post-wrap delta arithmetic\n");
    }

    y2k38_clock_set_mock_kernel(0, 0);
    y2k38_clock_set_kernel_offset(0);
    return rc;
}
