#include "xv6/syscall.h"
#include "xv6/traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret


#define T_SYSCALL 64




SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(getcrtc)
SYSCALL(setcrtc)
SYSCALL(getcurpos)
SYSCALL(setcurpos)
SYSCALL(gettime)
#define SYSCALL(name, call_num) \
    .globl xv6_sys_##name; \
    xv6_sys_##name: \
    movl $call_num, %eax; \
    int $T_SYSCALL; \
    ret
SYSCALL(sockcall, 22)


