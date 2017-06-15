#include <xv6/user.h>
#include <stdio.h>

volatile int count = 0;

int main(int argc, char *argv[])
{
    long i = 1023;
    const char *p = "world";
    double pi = 3.14159;
    myprintf("hello, %10s: %#06x   %.5f!\n", p, i, pi / 0);
    exit();
}
