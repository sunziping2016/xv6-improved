#include "xv6/syscall.h"
#include "xv6/traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

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
SYSCALL(thread_create)
SYSCALL(thread_exit)
SYSCALL(thread_wait)

SYSCALL(lock_create)
SYSCALL(lock_acquire)
SYSCALL(lock_release)
SYSCALL(lock_holding)
SYSCALL(lock_free)

SYSCALL(semaphore_create)
SYSCALL(semaphore_acquire)
SYSCALL(semaphore_release)
SYSCALL(semaphore_getcounter)
SYSCALL(semaphore_free)

SYSCALL(rwlock_create)
SYSCALL(rwlock_acquire_read)
SYSCALL(rwlock_acquire_write)
SYSCALL(rwlock_release_read)
SYSCALL(rwlock_release_write)
SYSCALL(rwlock_holding_read)
SYSCALL(rwlock_holding_write)
SYSCALL(rwlock_free)
SYSCALL(getcrtc)
SYSCALL(setcrtc)
SYSCALL(getcurpos)
SYSCALL(setcurpos)
SYSCALL(geteditstatus)
SYSCALL(seteditstatus)
SYSCALL(mount)
SYSCALL(unmount)
SYSCALL(gettime)

