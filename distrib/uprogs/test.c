#include "xv6/types.h"
#include "xv6/user.h"

volatile int count = 0;

int main(int argc, char *argv[])
{
    printf(1, "hello, world!\n");
    exit();
}
