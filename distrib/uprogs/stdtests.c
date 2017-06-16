#include <xv6/user.h>
#include <stdio.h>

volatile int count = 0;

int main(int argc, char *argv[])
{
    char a[128];
    long i = 1023;
    const char *p = "world";
    double pi = 3.14159;
    FILE *fi = fopen("README", "r");
    //myprintf("%d", fi);
    if(fgets(a, 50, fi))
        fputs(a, _std_files + 1);
    //fprintf(_std_files + 1, "hello, %s: %#06x   %.5f!\n", p, i, pi / 0);
    exit();
}
