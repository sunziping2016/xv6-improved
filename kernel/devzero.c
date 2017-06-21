// Zero input and output. 5
// Read operations from /dev/zero return as many null characters (0x00) as requested
// All write operations to /dev/zero succeed with no other effects.

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
 int zeroread(struct inode *ip, char *dst, uint off, int n)
 {
   iunlock(ip);
   acquire(&cons.lock);
   uint target;
   target;
   if(cons.r == 0)
   {
     while(n>0)
     {
       *dst=0;
       dst++;
       cons.r++;
       n--;
     }
   }
   else cons.r = 0;
   release(&cons.lock);
   ilock(ip);
   return target - n;
 }
 int zerowrite(struct inode *ip, char *buf, uint off, int n)
 {
   iunlock(ip);
   ilock(ip);
   return n;
 }
 void zeroinit(void)
 {
     initlock(&cons.lock, "devzero");
     devsw[NDEVZERO][MDEVZERO].write = zerowrite;
     devsw[NDEVZERO][MDEVZERO].read = zeroread;
     cons.locking = 1;
 }
