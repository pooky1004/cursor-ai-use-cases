#include <stdio.h>
#include <stdint.h>
#include <y2k38/time.h>

int main(void)
{
    y2k38_time_t rem;
    int fail = 0;

    y2k38_clock_set_mock_kernel(1, 2147483600);
    rem = y2k38_clock_seconds_until_wrap();
    printf("near wrap raw=2147483600 rem=%lld\n", (long long)rem);
    if (rem != 47) {
        printf("FAIL: expected 47\n");
        fail = 1;
    }

    y2k38_clock_set_mock_kernel(1, (int32_t)0x80000000);
    rem = y2k38_clock_seconds_until_wrap();
    printf("after wrap INT32_MIN rem=%lld\n", (long long)rem);
    if (rem != 4294967295LL) {
        printf("FAIL: expected 4294967295\n");
        fail = 1;
    }

    y2k38_clock_set_mock_kernel(1, -1);
    rem = y2k38_clock_seconds_until_wrap();
    printf("raw=-1 rem=%lld\n", (long long)rem);
    if (rem != 2147483648LL) {
        printf("FAIL: expected 2147483648\n");
        fail = 1;
    }

    return fail;
}
