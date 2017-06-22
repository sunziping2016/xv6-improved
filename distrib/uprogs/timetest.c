#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    myprintf("%d\n", clock());
    myprintf("%d\n", time(NULL));
    time_t m_time = time(NULL);
    tm t = localtime(&m_time);
    //tm t;
    //gettime(&t);
    myprintf("%s\n", asctime(&t));
    return 0;
}
