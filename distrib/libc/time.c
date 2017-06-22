#include <time.h>
#include <string.h>

#define SEC_PER_DAY 86400
int day_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

void int2str(int a, char *s)
{
    int index;
    for(index = 0; index < 20; ++index)
    {
        s[index] = a % 10 + '0';
        a = a / 10;
        if (a == 0)
        {
            s[index + 1] = '\0';
            break;
        }
    }
    int length = strlen(s);
    index = 0;
    while (1) {
        char temp = s[index];
        s[index] = s[length - index - 1];
        s[length - index - 1] = temp;
        index++;
        if (index >= length - index - 1)
            break;
    }
}

time_t time(time_t *arg)
{
    time_t result = 0;
    struct rtcdate time;
    gettime(&time);
    int i;
    for (i = 1970;i < time.year;++i)
    {
        if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0))//is leap year
            result += 366 * SEC_PER_DAY;
        else
            result += 365 * SEC_PER_DAY;
    }
    for (i = 1;i < time.month;++i)
    {
        result += day_per_month[i-1] * SEC_PER_DAY;
        if (i == 2 && ((time.year % 4 == 0 && time.year % 100 != 0) || (time.year % 400 == 0)))//Feb in leap year
            result += SEC_PER_DAY;
    }
    result += ((time.day - 1) * SEC_PER_DAY + time.hour * 3600 + time.minute * 60 + time.second);
    if (arg) *arg = result;
    return result;
}

clock_t clock()
{
    return uptime() - process_start_time;
}

char *asctime(tm *time)
{
    char s[20] = "110";
    if (time == NULL)
        return NULL;
    int day_index = 0;
    int i;
    for (i = 1;i < time->month;++i)
    {
        day_index += day_per_month[i-1];
        if (i == 2 && ((time->year % 4 == 0 && time->year % 100 != 0) || (time->year % 400 == 0)))//Feb in leap year
            day_index += 1;
    }
    day_index += time->day;
    int m;
    if (time->month <= 2)
        m = time->month + 12;
    else
        m = time->month;
    int week = ((time->year % 100) / 4 + (time->year % 100) + (time->year / 100) / 4 - 2 * (time->year / 100) + 26 * (m + 1) / 10 + time->day - 1) % 7;
    static char ascii_time[100]="";
    switch(week)
    {
        case 0:
            strcat(ascii_time,"Sun ");
            break;
        case 1:
            strcat(ascii_time,"Mon ");
            break;
        case 2:
            strcat(ascii_time,"Tue ");
            break;
        case 3:
            strcat(ascii_time,"Wed ");
            break;
        case 4:
            strcat(ascii_time,"Thu ");
            break;
        case 5:
            strcat(ascii_time,"Fri ");
            break;
        case 6:
            strcat(ascii_time,"Sat ");
            break;
        default:
            break;
    }
    switch(time->month)
    {
        case 1:
            strcat(ascii_time,"Jan ");
            break;
        case 2:
            strcat(ascii_time,"Feb ");
            break;
        case 3:
            strcat(ascii_time,"Mar ");
            break;
        case 4:
            strcat(ascii_time,"Apr ");
            break;
        case 5:
            strcat(ascii_time,"May ");
            break;
        case 6:
            strcat(ascii_time,"Jun ");
            break;
        case 7:
            strcat(ascii_time,"Jul ");
            break;
        case 8:
            strcat(ascii_time,"Aug ");
            break;
        case 9:
            strcat(ascii_time,"Sep ");
            break;
        case 10:
            strcat(ascii_time,"Otc ");
            break;
        case 11:
            strcat(ascii_time,"Nov ");
            break;
        case 12:
            strcat(ascii_time,"Dec ");
            break;
        default:
            break;
    }
    int2str(time->day,s);
    strcat(ascii_time,s);
    strcat(ascii_time," ");
    int2str(time->hour,s);
    strcat(ascii_time,s);
    strcat(ascii_time,":");
    int2str(time->minute,s);
    strcat(ascii_time,s);
    strcat(ascii_time,":");
    int2str(time->second,s);
    strcat(ascii_time,s);
    strcat(ascii_time," ");
    int2str(time->year,s);
    strcat(ascii_time,s);
    return ascii_time;
}

tm localtime(const time_t *time)
{
    tm mtm;
    int a = *time + 28800;
    mtm.second = a % 60;
    mtm.minute = (a / 60) % 60;
    mtm.hour = (a / 3600) % 24;
    int total_day = a / SEC_PER_DAY;
    int year = 1970, month = 1, day = 1;
    while (total_day >= 365)
    {
        if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && total_day == 365)
        {
            mtm.year = year;
            mtm.month = 12;
            mtm.day = 31;
            return mtm;
        }
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
            total_day -= 366;
        else
            total_day -= 365;
        year++;
    }
    while (1)
    {
        if (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && month == 2)
        {
            if (total_day >= 29)
            {
                month++;
                total_day -= 29;
                continue;
            }
            else
                break;
        }
        if (total_day >= day_per_month[month])
        {
            total_day -= day_per_month[month];
            month++;
        }
        else
            break;
    }
    day += total_day - 1;
    mtm.year = year;
    mtm.month = month;
    mtm.day = day;
    return mtm;
}