// Null input and output. 3
// All write and Read operations to /dev/nullsucceed with no other effects.

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
} cons;
 int nullread(struct inode *ip, char *dst, uint off, int n)
 {
   iunlock(ip);
   ilock(ip);
   return 0;
 }
 int nullwrite(struct inode *ip, char *buf, uint off, int n)
 {
   iunlock(ip);
   ilock(ip);
   return n;
 }
 void nullinit(void)
 {
     initlock(&cons.lock, "devnull");
     devsw[NDEVNULL][MDEVNULL].write = nullwrite;
     devsw[NDEVNULL][MDEVNULL].read = nullread;
     cons.locking = 1;
 }
