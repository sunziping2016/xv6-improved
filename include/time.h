//
// Created by freeman on 17-6-21.
//

#ifndef TIME_H
#define TIME_H

#include <xv6/types.h>
#include <xv6/date.h>
#include <xv6/user.h>

#define NULL 0
typedef struct rtcdate tm;
typedef int clock_t;
typedef int time_t;


#define CLOCK_PER_SECOND 1000

clock_t clock();
time_t time(time_t *arg);
char *asctime(tm *mtm);
tm localtime(const time_t *time);

static clock_t process_start_time;
#endif
