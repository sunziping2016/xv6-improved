#include <xv6/types.h>
#include <xv6/date.h>
#include <xv6/user.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    struct rtcdate tm;
    gettime(&tm);
    myprintf("%02d:%02d:%02d %d/%d%/%d", tm.hour, tm.minute, tm.day, tm.month, tm.day, tm.year);
    exit();
}
