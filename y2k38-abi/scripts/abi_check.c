/*
 * ABI layout sanity check for y2k38 types (run on host or target).
 */
#include <stdio.h>
#include <y2k38/types.h>

int main(void)
{
    int rc = 0;

    printf("sizeof(y2k38_time_t)=%lu (expect 8)\n",
           (unsigned long)sizeof(y2k38_time_t));
    printf("sizeof(struct y2k38_timeval)=%lu (expect 16)\n",
           (unsigned long)sizeof(struct y2k38_timeval));
    printf("sizeof(struct y2k38_timespec)=%lu (expect 16)\n",
           (unsigned long)sizeof(struct y2k38_timespec));

    if (sizeof(y2k38_time_t) != 8)
        rc = 1;
    if (sizeof(struct y2k38_timeval) != 16)
        rc = 1;
    if (sizeof(struct y2k38_timespec) != 16)
        rc = 1;

    if (rc)
        printf("ABI CHECK FAILED\n");
    else
        printf("ABI CHECK OK\n");
    return rc;
}
