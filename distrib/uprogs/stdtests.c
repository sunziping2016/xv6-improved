#include <xv6/user.h>
#include <stdio.h>
#include <setjmp.h>

jmp_buf jump_buffer;

void a(int count)
{
    myprintf("a(%d) called\n", count);
    longjmp(jump_buffer, count+1); // will return count+1 out of setjmp
}

int main(int argc, char *argv[])
{
    volatile int count = 0; // modified local vars in setjmp scope must be volatile
    if (setjmp(jump_buffer) != 9) // compare against constant in an if
        a(++count);
    exit();
}