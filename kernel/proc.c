#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/mmu.h"
#include "xv6/x86.h"
#include "xv6/proc.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"
#include "xv6/fs.h"
#include "xv6/file.h"
#include "xv6/proc_fs.h"
struct {
    struct spinlock lock;
    struct proc_table t;
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
    initlock(&ptable.lock, "ptable");
    for(int i = 0; i < NPQUEUES; ++i)
        ptable.t.ready[i].head = ptable.t.ready[i].tail = 0;
    ptable.t.sleeping.head = ptable.t.sleeping.tail = 0;
    ptable.t.unused.head = ptable.t.unused.tail = 0;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *allocproc(void)
{
    //cprintf("allocproc:\n");
    struct proc *p = NULL;
    char *sp;

    p = kalloc();
    if(p == NULL)
    {
        return 0;
    }
    memset(p, 0, sizeof(struct proc));
    p->state = EMBRYO;
    p->pid = nextpid++;
    // Allocate kernel stack.
    if((p->kstack = kalloc()) == 0) 
    {
        kfree(p);
        return 0;
    }
    
    sp = p->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof * p->tf;
    p->tf = (struct trapframe*)sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint*)sp = (uint)trapret;

    sp -= sizeof * p->context;
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof * p->context);
    p->context->eip = (uint)forkret;
    
    acquire(&ptable.lock);
    add_bar_front(&ptable.t.unused, p);
    release(&ptable.lock);
    //procdump();
    
    ///////////////////
    proc_add_process(p);

    return p;
}
//PAGEBREAK: 32
// Set up first user process.
void userinit(void)
{
    //cprintf("userinit:\n");
    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    p = allocproc();

    initproc = p;
    if ((p->pgdir = setupkvm()) == 0)
        panic("userinit: out of memory?");
    inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
    p->sz = PGSIZE;
    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;  // beginning of initcode.

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");

    // this assignment to p->state lets other cores
    // run this process. the acquire forces the above
    // writes to be visible, and the lock is also needed
    // because the assignment might not be atomic.
    acquire(&ptable.lock);
    erase_bar(p->which, p);
    p->state = RUNNABLE;
    p->rank = 0;
    p->length = 0;
    add_bar_back(&ptable.t.ready[0], p);
    release(&ptable.lock);
    //procdump();
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{

    uint sz;

    sz = proc->sz;
    if (n > 0) {
        if ((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
            return -1;
    } else if (n < 0) {
        if ((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
            return -1;
    }
    proc->sz = sz;
    switchuvm(proc);
    return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int fork(void)
{
    //cprintf("fork:\n");
    int i, pid;
    struct proc *np;

    // Allocate process.
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // Copy process state from p.
    if ((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0) {
        kfree(np->kstack);
        np->kstack = 0;
        acquire(&ptable.lock);
        erase_bar(np->which, np);
        release(&ptable.lock);
        kfree(np);
        return -1;
    }
    np->sz = proc->sz;
    np->parent = proc;
    *np->tf = *proc->tf;

    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;

    for (i = 0; i < NOFILE; i++)
        if (proc->ofile[i])
            np->ofile[i] = filedup(proc->ofile[i]);
    np->cwd = idup(proc->cwd);

    safestrcpy(np->name, proc->name, sizeof(proc->name));

    pid = np->pid;

    acquire(&ptable.lock);
    erase_bar(np->which, np);
    np->state = RUNNABLE;
    np->rank = 0;
    np->length = 0;
    add_bar_back(&ptable.t.ready[0], np);

    release(&ptable.lock);

    //procdump();
    return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
    //cprintf("exit:\n");
    struct proc *p;
    int fd;

    if (proc == initproc)
        panic("init exiting");

    // Close all open files.
    for (fd = 0; fd < NOFILE; fd++) {
        if (proc->ofile[fd]) {
            fileclose(proc->ofile[fd]);
            proc->ofile[fd] = 0;
        }
    }

    begin_op();
    iput(proc->cwd);
    end_op();
    proc->cwd = 0;

    acquire(&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1(proc->parent);

    // Pass abandoned children to init.
    struct proc_queue *qu = &ptable.t.ready[0];
    for(int i = 0; i < NPQUEUES + 2; ++i, ++qu)
        for (p = qu->head; p != NULL; p = p->next) {
            if (p->parent == proc) {
                p->parent = initproc;
                if (p->state == ZOMBIE)
                    wakeup1(initproc);
            }
        }

    // Jump into the scheduler, never to return.
    erase_bar(proc->which, proc);
    proc->state = ZOMBIE;
    add_bar_back(&ptable.t.unused, proc);
    sched();
    panic("zombie exit");
    //procdump();
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
    //cprintf("wait:\n");
    struct proc *p;
    int havekids, pid;

    acquire(&ptable.lock);
    for (;;) {
        // Scan through table looking for exited children.
        havekids = 0;
        struct proc_queue *qu = &ptable.t.ready[0];
        for(int i = 0; i < NPQUEUES + 2; ++i, ++qu)
            for (p = qu->head; p != NULL; p = p->next) {
                if (p->parent != proc)
                    continue;
                havekids = 1;
                if (p->state == ZOMBIE) {
                    ///////////////////
                    proc_del_process(p->pid);
                    // Found one.
                    erase_bar(p->which, p);
                    pid = p->pid;
                    kfree(p->kstack);
                    p->kstack = 0;
                    freevm(p->pgdir);
                    p->pid = 0;
                    p->parent = 0;
                    p->name[0] = 0;
                    p->killed = 0;
                    kfree(p);
                    release(&ptable.lock);
                    return pid;
                }
            }

        // No point waiting if we don't have any children.
        if (!havekids || proc->killed) {
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(proc, &ptable.lock);  //DOC: wait-sleep
    }
    //procdump();
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
    struct proc *p;
    for (;;) {
        //cprintf("scheduler:\n");
        // Enable interrupts on this processor.
        sti();

        // Loop over process table looking for process to run.
        acquire(&ptable.lock);

        p = ptable.t.ready[0].head;
        if(p == NULL)
            p = ptable.t.ready[1].head;
        if(p == NULL)
            p = ptable.t.ready[2].head;

        if(p != NULL && p->state == RUNNABLE)
        {
            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.
            erase_bar(p->which, p);
            p->state = RUNNING;
            add_bar_front(&ptable.t.ready[3], p);
            proc = p;

            switchuvm(p);
            swtch(&cpu->scheduler, proc->context);
            switchkvm();
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            proc = 0;

        }
        release(&ptable.lock);
    }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void) {

    int intena;

    if (!holding(&ptable.lock))
        panic("sched ptable.lock");

    if (cpu->ncli != 1)
        panic("sched locks");

    if (proc->state == RUNNING)
        panic("sched running");

    if (readeflags() & FL_IF)
        panic("sched interruptible");

    intena = cpu->intena;
    swtch(&proc->context, cpu->scheduler);
    cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
    //cprintf("yield:\n");
    acquire(&ptable.lock);  //DOC: yieldlock
    erase_bar(proc->which, proc);
    proc->state = RUNNABLE;
    if(proc->rank < 2)
        ++proc->rank;
    proc->length = 0;
    add_bar_back(&ptable.t.ready[proc->rank], proc);
    sched();
    release(&ptable.lock);
    //procdump();
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{

    static int first = 1;
    // Still holding ptable.lock from scheduler.
    release(&ptable.lock);

    if (first) {
        // Some initialization functions must be run in the context
        // of a regular process (e.g., they call sleep), and thus cannot
        // be run from main().
        first = 0;
        iinit(ROOTDEV);
        initlog(ROOTDEV);
    }

    // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
    //cprintf("sleep:\n");
    if (proc == 0)
        panic("sleep");

    if (lk == 0)
        panic("sleep without lk");

    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if (lk != &ptable.lock) {  //DOC: sleeplock0
        acquire(&ptable.lock);  //DOC: sleeplock1
        release(lk);
    }

    // Go to sleep.
    proc->chan = chan;
    erase_bar(proc->which, proc);
    proc->state = SLEEPING;
    add_bar_back(&ptable.t.sleeping, proc);
    sched();
    // Tidy up.
    proc->chan = 0;

    // Reacquire original lock.
    if (lk != &ptable.lock) {  //DOC: sleeplock2
        release(&ptable.lock);
        acquire(lk);
    }
    //procdump();
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
    struct proc *p;

    for (p = ptable.t.sleeping.head; p != NULL; p = p->next)
        if (p->chan == chan)
        {
            erase_bar(p->which, p);
            p->state = RUNNABLE;
            if(p->rank > 0)
                --p->rank;
            p->length = 0;
            add_bar_back(&ptable.t.ready[p->rank], p);
        }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
    //cprintf("wakeup:\n");
    acquire(&ptable.lock);
    wakeup1(chan);
    release(&ptable.lock);
    //procdump();
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
    //cprintf("kill:\n");
    struct proc *p;

    acquire(&ptable.lock);
    struct proc_queue *qu = &ptable.t.ready[0];
    for(int i = 0; i < NPQUEUES + 2; ++i, ++qu)
        for (p = qu->head; p != NULL; p = p->next)
            if(p->pid == pid)
            {
                p->killed = 1;
                // Wake process from sleep if necessary.
                if (p->state == SLEEPING)
                {
                    erase_bar(p->which, p);
                    p->state = RUNNABLE;
                    p->rank = 0;
                    p->length = 0;
                    add_bar_back(&ptable.t.ready[0], p);
                }
                release(&ptable.lock);
                //procdump();
                return 0;
            }
    release(&ptable.lock);
    //procdump();
    return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{

    static char *states[] = {
        [UNUSED]    "unused",
        [EMBRYO]    "embryo",
        [SLEEPING]  "sleep ",
        [RUNNABLE]  "runble",
        [RUNNING]   "run   ",
        [ZOMBIE]    "zombie"
    };
    struct proc *p;
    char *state;
    uint pc[10];
    struct proc_queue *qu = &ptable.t.ready[0];
    for(int i = 0; i < NPQUEUES + 3; ++i, ++qu)
    {
        if(i <= 3)
            cprintf("running and runnable %d:\n", i);
        else if(i == 4)
            cprintf("sleeping:\n");
        else
            cprintf("embryo and zombie:\n");
        for (p = qu->head; p != NULL; p = p->next) {
            if (p->state == UNUSED)
                continue;
            if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
                state = states[p->state];
            else
                state = "???";
            cprintf("%d %s %s", p->pid, state, p->name);
            if (p->state == SLEEPING) {
                getcallerpcs((uint*)p->context->ebp + 2, pc);
                for (i = 0; i < 10 && pc[i] != 0; i++)
                    cprintf(" %p", pc[i]);
            }
            cprintf("\n");
        }
    }
    cprintf("\n");
}
