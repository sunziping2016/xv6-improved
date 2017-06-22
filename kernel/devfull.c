// Full input and output. 3
// Read operations from /dev/full return as many null characters (0x00) as requested
// Write operations from /dev/full always return the error code ENOSPC

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
static struct {
  struct spinlock lock;
  int locking;
  uint r;
} cons;
 int fullread(struct inode *ip, char *dst, int n)
 {
   iunlock(ip);
   acquire(&cons.lock);
   uint target;
   target = n;
   if(cons.r == 0)
   {
     while(n>0)
     {
       *dst=0;
       dst++;
       cons.r++;
       n--;
     }
     cprintf("%d\n" , n);
   }
   else cons.r = 0;
   release(&cons.lock);
   ilock(ip);
   return target - n;
 }
 int fullwrite(struct inode *ip, char *buf, int n)
 {
   iunlock(ip);
   cprintf("No space left on device\n");
   ilock(ip);
   return n;
 }
 void fullinit(void)
 {
     initlock(&cons.lock, "devfull");
     devsw[NDEVFULL][MDEVFULL].write = fullwrite;
     devsw[NDEVFULL][MDEVFULL].read = fullread;
     cons.locking = 1;
 }
