#include <stdio.h>

int main(int argc, char *argv[])
{

    char a[128], b, c;
    int t, i;
    float fl;

    scanf("%c %c %f %d %s", &b, &c, &fl, &t, a);
    myprintf("%c  %c  %f  %d  %s\n", b, c, fl, t, a);
    return 0;
}