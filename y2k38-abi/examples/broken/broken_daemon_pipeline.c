/*
 * Reference: how Daemon A/B FAIL when using 32-bit time_t.
 * Not installed to the board — educational contrast only.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif

int main(void)
{
    /* Simulated "now" just before overflow */
    time_t now = (time_t)(INT32_MAX - 50);
    /* Future event: first second after overflow — destroyed by cast */
    long long true_future = (long long)INT32_MAX + 1;
    time_t stored = (time_t)true_future;
    long delta = (long)(stored - now);

    printf("BROKEN daemon pipeline simulation\n");
    printf("sizeof(time_t)=%lu\n", (unsigned long)sizeof(time_t));
    printf("true_future=%lld\n", true_future);
    printf("stored time_t=%ld\n", (long)stored);
    printf("now=%ld\n", (long)now);
    printf("delta(stored-now)=%ld  (correct answer would be 51)\n", delta);
    printf("Daemon B would write wrong DELTA / may go negative.\n");
    return 0;
}
