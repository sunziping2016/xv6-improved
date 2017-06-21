//
// Created by freeman on 17-6-21.
//

#ifndef TIME_H
#define TIME_H

#include <xv6/user.h>
#include <xv6/date.h>
#include <xv6/types.h>

typedef unsigned int clock_t;
typedef unsigned int time_t;


#define CLOCK_PER_SECOND 1000

clock_t clock();
time_t time(time_t *arg);
char *asctime(time_t tm);


#endif
