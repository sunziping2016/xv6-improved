#include <time.h>
#include <string.h>

time_t time(time_t *arg)
{
    struct rtcdate tm;
    gettime(&tm);
}