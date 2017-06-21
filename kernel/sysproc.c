#include "xv6/types.h"
#include "xv6/x86.h"
#include "xv6/defs.h"
#include "xv6/date.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/signal.h"
#include "xv6/proc.h"
#include "xv6/signal.h"

int
sys_fork(void)
{
    return fork();
}

int
sys_exit(void)
{
    exit();
    return 0;  // not reached
}

int
sys_wait(void)
{
    return wait();
}

int
sys_kill(void)
{
    int pid;

    if (argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

int
sys_getpid(void)
{
    return proc->pid;
}

int
sys_sbrk(void)
{
    int addr;
    int n;

    if (argint(0, &n) < 0)
        return -1;
    addr = proc->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

int
sys_sleep(void)
{
    int n;
    uint ticks0;

    if (argint(0, &n) < 0)
        return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (proc->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

int
sys_sigaction(void)
{
    int sig;
    struct sigaction *act, *oact;
    if (argint(0, &sig) < 0 || argptr(1, (void*)&act, sizeof(*act)) < 0 || argptr(2, (void*)&oact, sizeof(*oact)) < 0)
        return -1;
    return sigaction(sig, act, oact);
}

int
sys_sigkill(void)
{
    int pid, sig;
    if (argint(0, &pid) < 0 || argint(1, &sig) < 0)
        return -1;
    return sigkill(pid, sig);
}

int
sys_raise(void)
{
    int sig;
    if (argint(0, &sig) < 0)
        return -1;
    return raise(sig);
}

int
sys_sigqueue(void)
{
    int sig, pid;
    union sigval* sigv;
    if (argint(0, &pid) < 0 || argint(1, &sig) < 0 || argptr(2, (void*)&sigv, sizeof(*sigv)) < 0)
        return -1;
    return sigqueue(pid, sig, *sigv);
}

int
sys_siginterrupt(void)
{
    int sig, flag;
    if (argint(0, &sig) < 0 || argint(1, &flag) < 0)
        return -1;
    return siginterrupt(sig, flag);
}

int
sys_sigset(void)
{
    int sig;
    void (*disp)(int);
    if (argint(0, &sig) < 0 || argptr(1, (void*)&disp, sizeof(*disp)) < 0)
        return -1;
    return sigset(sig, disp);
}

int
sys_sigrelse(void)
{
    int sig;
    if (argint(0, &sig) < 0)
        return -1;
    return sigrelse(sig);
}