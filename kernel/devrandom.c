// input and output.random : 8  urandom:9
// simplify random.c from linux
// Read operations from /dev/random return hexadecimal data;
// write operations to /dev/random add entropy

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
#define MAX_POOL 8096
#define ENTROPY_SHIFT 3
extern uint ticks;
uint last_ticks;
static struct {
  struct spinlock lock;
  int locking;
  uint r;
  uint e;
  uint w;
} cons, ucons;
static struct poolinfo {
	int tap1, tap2, tap3, tap4, tap5;
} poolinfo_table[] = {
	/* was: x^128 + x^103 + x^76 + x^51 +x^25 + x + 1 */
	/* x^128 + x^104 + x^76 + x^51 +x^25 + x + 1 */
	{ 104, 76,	51,	25,	1 },
	/* was: x^32 + x^26 + x^20 + x^14 + x^7 + x + 1 */
	/* x^32 + x^26 + x^19 + x^14 + x^7 + x + 1 */
	{ 26,	19,	14,	7,	1 },
};
struct queue_random
{
  int head, tail, num;
  uint init_entropy;
  uint pool[MAX_POOL];
}source_entropy, second_entropy, urandom_entropy;
void init(struct queue_random *r)
{
  r->num = r->head = r->tail = 0;
  r->init_entropy = ticks;
}
int pop(struct queue_random *r)
{
  if(r->num == 0)return -1;
  uint key = r->pool[r->head % MAX_POOL];
  r->head++;
  r->head = r->head%MAX_POOL;
  r->num--;
  return key;
}
uint front(struct queue_random *r)
{
  uint key = r->pool[r->head % MAX_POOL];
  return key;
}
void push(struct queue_random *r,int data)
{
  while(r->num > MAX_POOL)
  {
    pop(r);
  }
  r->pool[r->tail % MAX_POOL] = data;
  r->num++;
  r->tail = r->head + r->num;
}
int empty(struct queue_random *r)
{
  if(r->num == 0)return 1;
  return 0;
}
unsigned int rol32(int a, int b)
{
  return a ^ (1<<(32-b));
}
static unsigned int trans_entropy(int key, int type)
{
  int tap1, tap2, tap3 ,tap4;
  tap1 = poolinfo_table[type].tap1;
  tap2 = poolinfo_table[type].tap2;
  tap3 = poolinfo_table[type].tap3;
  tap4 = poolinfo_table[type].tap4;

  tap1 += tap2;   tap3 += tap4;
  tap2 = rol32(tap2, 6); tap4 =rol32(tap4, 27);
  tap4 ^= tap1;   tap2 ^= tap3;
  tap1 += tap2;   tap3 += tap4;
  tap2 = rol32(tap2, 16); tap4 =rol32(tap4, 14);
  tap4 ^= tap1;   tap2 ^= tap3;
  tap1 += tap2;   tap3 += tap4;
  tap2 = rol32(tap2, 6); tap4 =rol32(tap4, 27);
  tap4 ^= tap1;   tap2 ^= tap3;
  tap1 += tap2;   tap3 += tap4;
  tap2 = rol32(tap2, 16); tap4 =rol32(tap4, 14);
  tap4 ^= tap1;   tap2 ^= tap3;

  poolinfo_table[type].tap1 = tap1;
  poolinfo_table[type].tap2 = tap2;
  poolinfo_table[type].tap3 = tap3;
  poolinfo_table[type].tap4 = tap4;

  key ^= poolinfo_table[type].tap1;
      key ^= poolinfo_table[type].tap2;
          key ^= poolinfo_table[type].tap3;
              key ^= poolinfo_table[type].tap4;
  return key;
}
static void mix_second_entropy(int key0)
{
  int key1 = trans_entropy(key0, 1);
  int key2 = trans_entropy(key1, 0);
  int key3 = trans_entropy(key2, 1);
  int key4 = trans_entropy(key3, 0);

  push(&second_entropy,key1);
  push(&second_entropy,key2);
  //cprintf("  %d  %d ok\n", key2, second_entropy.num);
  push(&second_entropy,key3);
  push(&second_entropy,key4);
  //cprintf("  %d  %d ok\n", key3, second_entropy.num);
}
static void mix_urandom_entropy(int key0)
{
  int key1 = trans_entropy(key0, 1);
  int key2 = trans_entropy(key1, 0);
  int key3 = trans_entropy(key2, 1);
  int key4 = trans_entropy(key3, 0);
  push(&urandom_entropy,key1);
  push(&urandom_entropy,key2);
  push(&urandom_entropy,key3);
  push(&urandom_entropy,key4);
}
void mix_source_entropy()
{
  if(last_ticks == ticks)return;
  last_ticks = ticks;
  push(&source_entropy, trans_entropy(ticks, 0));
  //cprintf("%d ok\n", ticks);
}
static void expand_second_entropy()
{
  if(empty(&source_entropy))
  {
    int key = source_entropy.init_entropy = ticks;
    push(&source_entropy,trans_entropy(key, 0));
  }
  uint key0= pop(&source_entropy);
  mix_second_entropy(key0);
}
static void expand_urandom_entropy()
{
  uint key0;
  if(empty(&source_entropy))
  {
    if(urandom_entropy.init_entropy == 0)urandom_entropy.init_entropy =ticks;
     key0 = (urandom_entropy.init_entropy*ticks+ 0xa00ae278)%32767;
     //cprintf("%d ok\n", key0);
     urandom_entropy.init_entropy = key0;
  }
  else
  {
    key0 = pop(&source_entropy);
    //urandom_entropy.init_entropy ^= trans_entropy(key0, 0);
  }
  mix_urandom_entropy(trans_entropy(key0, 0));
}
static int fast_extract_entropy()
{
  while(empty(&urandom_entropy))
  {
    expand_urandom_entropy();
  }
  uint key = pop(&urandom_entropy);
  //cprintf("  %d  ok\n", key);
  return key % 16;
}
static int extract_entropy()
{
   while(empty(&second_entropy))
   {
     expand_second_entropy();
     return -1;
   }
   uint key = pop(&second_entropy);
   //cprintf("  %d  ok\n", key);
   return key % 16;
}
static char digits[] = "0123456789abcdef";
 int randomread(struct inode *ip, char *dst, uint off, int n)
 {
   iunlock(ip);
   acquire(&cons.lock);
   uint target;
   target = n;
   if(cons.r == 0)
   {
     cons.r = n;
     cons.e = 0;
   }
   if(cons.r > cons.e)
   {
     while(n > 0)
      {
        int key0 = extract_entropy();
        if(key0 == -1)
        {
          key0 = extract_entropy();
          int sleep = 0;
          while ( sleep < ((ticks % 40 + 60)*20000))sleep++;
          *dst= digits[key0];
          n--;
          cons.e++;
          break;
        }
        *dst= digits[key0];
        dst++;
        n--;
        cons.e++;
      }
   }
   else if(cons.e >= cons.r)
   {
     cons.r = 0;
     target = n;
   }
   release(&cons.lock);
   ilock(ip);
   return target - n;
 }
 int urandomread(struct inode *ip, char *dst, uint off, int n)
 {
   iunlock(ip);
   acquire(&ucons.lock);
   uint target;
   target = n;
   if(ucons.r == 0)
   {
     ucons.r = n;
     ucons.e = 0;
   }
   if(ucons.r > ucons.e)
   {
     while(n > 0)
      {
        int key0 = fast_extract_entropy();
        *dst= digits[key0];
        dst++;
        n--;
        ucons.e++;
      }
   }
   else if(ucons.e >= ucons.r)
   {
     ucons.r = 0;
     target = n;
   }
   release(&ucons.lock);
   ilock(ip);
   return target - n;
 }

 int randomwrite(struct inode *ip, char *buf, uint off, int n)
 {
   iunlock(ip);
   acquire(&cons.lock);
   cons.w = 0;
     while(n>0)
     {
       uint key = *buf;
       buf++;
       cons.w++;
       n--;
       int sleep = 0;
       while ( sleep < ((ticks % 40 + 60)*2000))sleep++;
       key += (second_entropy.init_entropy*(ticks+ucons.w)+ sleep)%32767;
       second_entropy.init_entropy = key;
       key = trans_entropy(key, 1);
       mix_second_entropy(key);
     }
   release(&cons.lock);
   ilock(ip);
   return cons.w;
 }
 int urandomwrite(struct inode *ip, char *buf, uint off, int n)
 {
   iunlock(ip);
   acquire(&ucons.lock);
   ucons.w = 0;
     while(n>0)
     {
       uint key = *buf;
       buf++;
       ucons.w++;
       n--;
       key += (urandom_entropy.init_entropy*ticks+ucons.w)%32767;
       urandom_entropy.init_entropy = key;
       key = trans_entropy(key, 1);
       mix_urandom_entropy(key);
     }
   //cprintf("%d ok\n", ucons.w);
   release(&ucons.lock);
   ilock(ip);
   return ucons.w;
 }
 void randominit(void)
 {
     initlock(&cons.lock, "devrandnom");
     initlock(&ucons.lock, "devurandnom");
     init(&source_entropy);
     init(&second_entropy);
     init(&urandom_entropy);
     devsw[NDEVRANDOM][MDEVRANDOM].write = randomwrite;
     devsw[NDEVRANDOM][MDEVRANDOM].read = randomread;
     devsw[NDEVURANDOM][MDEVURANDOM].write = urandomwrite;
     devsw[NDEVURANDOM][MDEVURANDOM].read = urandomread;
     cons.locking = 1;
 }
