<<<<<<< HEAD
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
soundread(struct inode *ip, char *dst, int n)
{
    iunlock(ip);
    acquire(&sound.lock);
    release(&sound.lock);
    ilock(ip);
    return 0;
}
int 
soundwrite(struct inode *ip, char *buf, int n)
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
    initlock(&sound.lock, "sound");

    devsw[SOUND].write = soundwrite;
    devsw[SOUND].read = soundread;
    sound.locking = 1;
=======
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
        0,
        262,
        294,
        330,
        349,
        392,
        440,
        494
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
    if(tone >= 1 && tone <= 7)
    {
        uint t = new_ticks;
        playsound(freq[tone]);
        while(new_ticks <= t + 50);
        nosound();
    }
}

int
soundread(struct inode *ip, char *dst, int n)
{
    iunlock(ip);
    acquire(&sound.lock);
    release(&sound.lock);
    ilock(ip);
    return 0;
}
int 
soundwrite(struct inode *ip, char *buf, int n)
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
    initlock(&sound.lock, "sound");

    devsw[SOUND].write = soundwrite;
    devsw[SOUND].read = soundread;
    sound.locking = 1;
>>>>>>> 00a18f14de1226d0dca852f55d67e576aa074ba7
}