//Performance counter of xv6

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

extern struct {
    struct spinlock lock;
    struct inode inode[NINODE];
} icache;

static struct
{
    struct spinlock lock;
    int locking;
    uint r;
}perf;


int
perfctrread(struct inode *ip, char *dst, uint off, uint n)
{
    uint target;

    iunlock(ip);
    target = n;
    acquire(&perf.lock);
    if(perf.r == 0)
    {
        char info[] = "inode: ";
        int i;
        for(i = 0; i < 7;i++)
            *dst++ = info[i];
        n -= 7;
        perf.r += 7;
        uint counter = 0;
        struct inode *ipointer;
        for (ipointer = &icache.inode[0]; ipointer < &icache.inode[NINODE]; ipointer++)
        {
          int j = ipointer->ref;
          while( j > 0)
          {
          counter++;
          --j;
          }
        }
        int percentage = 100 * counter / NINODE;
        i = 0;
        *dst++ = 'n';i++;
        *dst++ = 'o';i++;
        *dst++ = 'd';i++;
        *dst++ = 'e';i++;
        *dst++ = ' ';i++;
        *dst++ = 'u';i++;
        *dst++ = 's';i++;
        *dst++ = 'e';i++;
        *dst++ = ' ';i++;
        *dst++ = (int) percentage / 10 + '0';i++;       //tens
        *dst++ = (int) percentage % 10 + '0';i++;       //ones
        *dst++ = '%';i++;
        *dst++ = '\n';i++;
        n -= i;
        perf.r += i;
    }
    else
        perf.r = 0;
    release(&perf.lock);
    ilock(ip);
    return target - n;
}
int
perfctrwrite(struct inode *ip, char *cbuf, uint off, uint n)
{
    iunlock(ip);
    ilock(ip);
    return n;
}
void
perfctrinit(void)
{
    initlock(&perf.lock, "prefctr");

    devsw[NDEVPERFCTR][MDEVPERFCTR].write = perfctrwrite;
    devsw[NDEVPERFCTR][MDEVPERFCTR].read = perfctrread;
    perf.locking = 1;
}
