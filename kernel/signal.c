#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/x86.h"
#include "xv6/signal.h"
#include "xv6/proc.h"
#include "xv6/spinlock.h"

extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
}ptable;



int    sigemptyset(sigset_t *set)
{
    set->number = 0;
    for(int i = 0; i < _SIG_NUM; i++)
    {
        set->sig[i] = 0;
    }
    return 0;
}

int sigfillset(sigset_t *set)
{
    switch (_NSIG_WORDS) {
        case 1:	set->sig[0] = -1;
            break;
        case 2: set->sig[1] = -1;
            break;
        default:
            memset(set, -1, sizeof(sigset_t));
            break;
    }
    return 0;
}


int    sigaddset(sigset_t *set, int signo)
{
    if(!sigismember(set, signo))
    {
        set->sig[set->number] = signo;
        set->number += 1;
    }
    return 0;
}

int  sigdelset(sigset_t *set, int signo)
{
    if(sigismember(set, signo))
    {
        int i;
        for(i = 0; i < set->number; i++)
        {
            if(set->sig[i] == signo)
                break;
        }
        for(; i < set->number - 1; i++)
        {
            set->sig[i] = set->sig[i + 1];
        }
        set->sig[set->number-1] = 0;
        set->number -= 1;
    }
    return 0;
}

int sigismember(const sigset_t *set, int signo)
{
    int i;
    for(i = 0; i < set->number; i++)
    {
        if(set->sig[i] == signo)
        {
            return 1;
        }
    }
    return 0;
}

int sigisemptyset(sigset_t *set)
{
    if(set->number == 0)
        return 0;
    else
        return 1;
}

int sigaction(int sig, const struct sigaction *act,
              struct sigaction *oact)
{
    struct sigaction *k;
    k = &(proc->sighand.action[sig - 1]);
    if (oact)
        *oact = *k;
    if (act) {
        *k = *act;
    }
    return 0;
}

int killpg(int pgrp, int sig) {
    if (pgrp > 1)
        sigkill(pgrp, sig);
}

int raise(int sig)
{
    if(sig == 0)
    {
        return -1;
    }
    else
    {
        proc->sighand.action[sig-1].sa_handler(sig);
        return 0;
    }
}

int sigqueue(int pid, int sig, union sigval value)
{
    if (sig == 0)
    {
        return -1;
    }
    else if(pid >= 0)
    {
        struct siginfo_t siginfo;
        siginfo.si_value = value;
        proc->sighand.action[sig-1].sa_handler(sig);
        proc->sighand.action[sig-1].sa_sigaction(sig, &siginfo, (void*)0);
        return 0;
    }
    return 0;
}

int siginterrupt(int sig, int flag)
{
    int ret;
    struct sigaction act;
    (void) sigaction(sig, ((void*)0), &act);
    if (flag)
        act.sa_flags &= ~SA_RESTART;
    else
        act.sa_flags |= SA_RESTART;
    ret = sigaction(sig, &act, ((void*)0));
    return ret;
}

int sigignore(int sig)
{
    return sig;
}

int sighold(int sig)
{
    sigaddset(&(proc->blocked), sig);
    return 0;
}

int sigrelse(int sig)
{
    sigdelset(&(proc->blocked), sig);
    return 0;
}

void (*sigset(int sig, void (*disp)(int)))(int)
{
    if(disp == SIG_HOLD)
    {
        sighold(sig);
    }
    else if(disp == SIG_IGN)
    {
        sigignore(sig);
    }
    else if(disp == SIG_DEF)
    {
        struct sigaction act, oact;
        act.sa_handler = sigdefhandler;
        act.sa_flags = SA_RESTART;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        if (sigaction(sig, &act, &oact) == -1)
            return(SIG_ERR);
        return(oact.sa_handler);
    }
    else
    {
        struct sigaction act, oact;
        act.sa_handler = disp;
        act.sa_flags = SA_RESTART;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        if (sigaction(sig, &act, &oact) == -1)
            return(SIG_ERR);
        return(oact.sa_handler);
    }
    return *disp;
}


int    sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset)
{
    *oset = proc->blocked;
    int i;
    if (how == SIG_BLOCK)
    {
        for(i = 0; i < set->number; i++)
            sigaddset(&(proc->blocked), set->sig[i]);
    }
    else if(how == SIG_UNBLOCK)
    {
        for(i = 0; i < set->number; i++)
            sigdelset(&(proc->blocked), set->sig[i]);
    }
    else if(how == SIG_SETMASK)
    {
        sigemptyset(&(proc->blocked));
        for(i = 0; i < set->number; i++)
            sigaddset(&(proc->blocked), set->sig[i]);
    }
    return 0;
}

void (*signal(int sig, void (*func)(int)))(int) {
    if (func == SIG_IGN) {
        sigignore(sig);
    } else if (func == SIG_DEF) {
        struct sigaction act, oact;
        act.sa_handler = sigdefhandler;
        act.sa_flags = SA_RESTART;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        if (sigaction(sig, &act, &oact) == -1)
            return (SIG_ERR);
        return (oact.sa_handler);
    } else {
        struct sigaction act, oact;
        act.sa_handler = func;
        act.sa_flags = SA_RESTART;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        if (sigaction(sig, &act, &oact) == -1)
            return (SIG_ERR);
        return (oact.sa_handler);
    }
}


int sigpending(sigset_t *set)
{
    *set = proc->pending.signal;
}

int sigsuspend(const sigset_t *sigmask)
{
    sigset_t temp_set = proc->blocked;
    proc->blocked = *sigmask;
    return 0;
}
