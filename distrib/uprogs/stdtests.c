#include <xv6/user.h>
#include <stdio.h>

volatile int count = 0;

int main(int argc, char *argv[])
{
    char a[128];
    FILE *f = fopen("README", "r");
    int i, j;

    for(i = 0; i < 3; ++i)
    {
        a[i] = fgetc(f);
        for(j = 0; j < BUFSIZ; ++j)
            putchar(f->buffer[j]);
        putchar('\n');
    }

    ungetc('.', f);
    ungetc('.', f);

    for(i = 3; i < 20; ++i)
    {
        a[i] = fgetc(f);
    }
    a[20] = 0;
    puts(a);
    exit();
}
