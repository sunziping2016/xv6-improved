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
<<<<<<< HEAD
}ptable;

int sigemptyset(sigset_t *set)
=======
} ptable;

int    sigemptyset(sigset_t *set)
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
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

<<<<<<< HEAD
int sigaddset(sigset_t *set, int signo)
=======
int    sigaddset(sigset_t *set, int signo)
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
{
    if(!sigismember(set, signo))
    {
        set->sig[set->number] = signo;
        set->number += 1;
    }
    return 0;
}

<<<<<<< HEAD
int sigdelset(sigset_t *set, int signo)
=======
int  sigdelset(sigset_t *set, int signo)
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
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
<<<<<<< HEAD
    struct sigaction *k;
    k = &(proc->sighand.action[sig - 1]);
    if (oact)
        *oact = *k;
    if (act) {
        *k = *act;
    }
=======
    //struct proc *current;
    // Enable interrupts on this processor.
    //sti();

    // Loop over process table looking for process to run.
    //acquire(&ptable.lock);
    //current = &ptable.proc[1];
    //for (current = ptable.proc; current < &ptable.proc[NPROC]; current++) {
    //if (current->state == RUNNING)
    //break;
    //}
    //release(&ptable.lock);
    //struct proc *p = current, *t;
    struct sigaction *k;
    //sigset_t mask;
    //if (!valid_signal(sig) || sig < 1 || (act && sig_kernel_only(sig)))
    //return EINVAL;
    k = &(proc->sighand.action[sig - 1]);
    //spin_lock_irq(&p->sighand->siglock);
    if (oact)
        *oact = *k;

    if (act) {
        //sigdelsetmask(&act->sa_mask,
        //sigmask(SIGKILL) | sigmask(SIGSTOP));
        //*k = *act;
        *k = *act;
        //if (sig_handler_ignored(sig_handler(p, sig), sig)) {
        //sigemptyset(&mask);
        //sigaddset(&mask, sig);
        //flush_sigqueue_mask(&mask, &p->signal->shared_pending);
        //for_each_thread(p, t)
        //flush_sigqueue_mask(&mask, &t->pending);
        //}
    }

    //spin_unlock_irq(&p->sighand->siglock);
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
    return 0;
}


<<<<<<< HEAD

int killpg(int pgrp, int sig)
{
    if(pgrp > 1)
        sigkill(pgrp, sig);
=======
int sigkill(int pid, int sig)
{
    if(sig == 0)
    {
        return -1;
    }
    else if(pid > 0)
    {
        struct proc *p;
        acquire(&ptable.lock);
        for (;;) {
            // Scan through table looking for exited children.
            for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
                if (p->pid == pid)
                    break;
            }
            if(!sigismember(&(p->blocked), sig) || sig == SIGKILL || sig == SIGSTOP)
            {
                release(&ptable.lock);
                proc = p;
                p->sighand.action[sig - 1].sa_handler(sig);
            }
            release(&ptable.lock);
            return 1;
        }
    }
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
    return 0;
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
<<<<<<< HEAD
=======


>>>>>>> d309231487fed12c6fae079d2b20295733d43142
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
<<<<<<< HEAD
        act.sa_handler = *disp;
=======
        act.sa_handler = disp;
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
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

<<<<<<< HEAD
void (*signal(int sig, void (*func)(int)))(int)
{
    if(func == SIG_IGN)
    {
        sigignore(sig);
    }
    else if(func == SIG_DEF)
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
        act.sa_handler = func;
        act.sa_flags = SA_RESTART;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        if (sigaction(sig, &act, &oact) == -1)
            return(SIG_ERR);
        return(oact.sa_handler);
=======
void sigdefhandler(int sig)
{
    void *p = 0;
    if(sig == SIGALRM || sig == SIGHUP || sig == SIGINT || sig == SIGKILL || sig == SIGPIPE || sig == SIGTERM || sig == SIGUSR1 || sig == SIGUSR2 || sig == SIGPOLL || sig == SIGPROF || sig == SIGVTALRM)
    {
        exit();
    }
    else if(sig == SIGABRT || sig == SIGBUS || sig == SIGFPE || sig == SIGILL || sig == SIGQUIT || sig == SIGSEGV || sig == SIGSYS || sig == SIGTRAP || sig == SIGXCPU || sig == SIGXFSZ)
    {
        exit();
    }
    else if(sig == SIGCHLD || sig == SIGURG)
    {
        return;
    }
    else if(sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU)
    {
        acquire(&ptable.lock);
        sleep(p, &ptable.lock);
        release(&ptable.lock);
    }
    else if(sig == SIGCONT)
    {
        wakeup(p);
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
    }
}


<<<<<<< HEAD
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



=======
>>>>>>> d309231487fed12c6fae079d2b20295733d43142
