#include "xv6/types.h"
#include "xv6/x86.h"

void playsound(uint frequence)
{
    uint div;
    uchar tmp;
    div = 1193180 / frequence;
    outb(0x43, 0xb6);
    outb(0x42, (uchar) div);
    outb(0x42, (uchar) (div >> 8));
    tmp = inb(0x61);
    if (tmp != (tmp | 0x03))
        outb(0x61, tmp | 0x03);
}

void nosound()
{
    uchar tmp;
    tmp = inb(0x61) & 0xfc;
    outb(0x61, tmp);
}

int sys_playsound()
{
    uint frequence;
    if (argint(0, &frequence) < 0)
        return -1;
    playsound(frequence);
    return 0;
}

int sys_nosound()
{
    nosound();
    return 0;
}
