//Play sound from an array of numbers
//Input is from a file

#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/traps.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"
#include "xv6/fs.h"
#include "xv6/file.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/x86.h"

extern uint new_ticks;

uint freq[] = {
        262,
        294,
        330,
        349,
        392,
        440,
        494,
        523,
        587,
        660
    };

static struct
{
    struct spinlock lock;
    int locking;
}sound;

void
soundplayc(char ch)
{
    int tone = ch - '0';
    if(tone >= 0 && tone <= 9)
    {
        uint t = new_ticks;
        playsound(freq[tone]);
        while(new_ticks <= t + 50);
        nosound();
    }
    else if (ch == ' ')
    {
        uint t = new_ticks;
        while(new_ticks <= t + 50);
    }
}

int
soundread(struct inode *ip, char *dst, uint off, int n)
{
    iunlock(ip);
    acquire(&sound.lock);
    release(&sound.lock);
    ilock(ip);
    return 0;
}
int
soundwrite(struct inode *ip, char *buf, uint off, int n)
{
    int i;
    iunlock(ip);
    acquire(&sound.lock);
    for (i = 0; i < n; i++)
        soundplayc(buf[i] & 0xff);
    release(&sound.lock);
    ilock(ip);
    return n;
}
void
soundinit(void)
{
    initlock(&sound.lock, "devsound");
    devsw[NDEVSOUND][MDEVSOUND].write = soundwrite;
    devsw[NDEVSOUND][MDEVSOUND].read = soundread;
    sound.locking = 1;
}
