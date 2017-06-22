//
// Created by huz on 17-5-11.
//

#ifndef XV6_IMPROVED_SIGNAL_H
#define XV6_IMPROVED_SIGNAL_H
#define _NSIG		64
#define _NSIG_BPW	32
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)
#define _SIG_NUM 28
typedef unsigned int size_t;//Used for sizes of objects.
typedef int time_t;  //Used for time in seconds.
typedef unsigned int pthread_t;  //Used to identify a thread.
typedef int uid_t;
typedef signed int pid_t; //Used for process IDs and process group IDs.
//typedef int sigset_t;  // Integer or structure type of an object used to represent sets of signals.
typedef struct {
    int sig[_SIG_NUM];
    int number;
} sigset_t;
struct timespec
{
    time_t  tv_sec;   //Seconds.
    long    tv_nsec;  //Nanoseconds.
};
struct sched_param
{
    int  sched_priority;
    int  sched_curpriority;
    union {
        int  reserved[8];
        struct {
            int  __ss_low_priority;
            int  __ss_max_repl;
            struct timespec     __ss_repl_period;
            struct timespec     __ss_init_budget;
        }           __ss;
    }           __ss_un;
};
struct pthread_attr_t
{
    int contentionscope;
    struct sched_param schedparam;
    int  detachstate;
    void *stackaddr;
    size_t stacksize;
};


#define SA_NOCLDSTOP 1
#define SA_ONSTACK 2
#define SA_RESETHAND 3
#define SA_RESTART 4
#define SA_SIGINFO 5
#define SA_NOCLDWAIT 6
#define SA_NODEFER 7


union sigval
{
    int    sival_int;    // Integer signal value
    void  *sival_ptr;    // Pointer signal value
};
struct sigevent
{
    int              sigev_notify;          // Notification type.
    int              sigev_signo;            // Signal number.
    union sigval     sigev_value;            // Signal value.
    void           (*sigev_notify_function)(union sigval);
    //Notification function.
    struct pthread_attr_t *sigev_notify_attributes;  //Notification attributes.
};
//symbolic constants for the values of sigev_notify
#define SIGEV_NONE 1 //No asynchronous notification is delivered when the event of interest occurs.
#define SIGEV_SIGNAL 2 //A queued signal, with an application-defined value, is generated when the event of interest occurs.
#define SIGEV_THREAD 3 //A notification function is called to perform notification.

#define SIGABRT 1 //A Process abort signal.
#define SIGALRM 2 //T Alarm clock.
#define SIGBUS 3//A Access to an undefined portion of a memory object.
#define SIGCHLD 4//I Child process terminated, stopped,
#define SIGCONT 5//C Continue executing, if stopped.
#define SIGFPE 6//A Erroneous arithmetic operation.
#define SIGHUP 7//T Hangup.
#define SIGILL 8//A Illegal instruction.
#define SIGINT 9//T Terminal interrupt signal.
#define SIGKILL 10//T Kill (cannot be caught or ignored).
#define SIGPIPE 11//T Write on a pipe with no one to read it.
#define SIGQUIT 12//A Terminal quit signal.
#define SIGSEGV 13//A Invalid memory reference.
#define SIGSTOP 14//S Stop executing (cannot be caught or ignored).
#define SIGTERM 15//T Termination signal.
#define SIGTSTP 16//S Terminal stop signal.
#define SIGTTIN 17//S Background process attempting read.
#define SIGTTOU 18//S Background process attempting write.
#define SIGUSR1 19//T User-defined signal 1.
#define SIGUSR2 20//T User-defined signal 2.
#define SIGPOLL 21//T Pollable event.
#define SIGPROF 22//T Profiling timer expired.
#define SIGSYS 23//A Bad system call
#define SIGTRAP 24//A Trace/breakpoint trap.
#define SIGURG 25//I High bandwidth data is available at a socket.
#define SIGVTALRM 26//T Virtual timer expired.
#define SIGXCPU 27//A CPU time limit exceeded.
#define SIGXFSZ 28//A File size limit exceeded.
/*
 * default action:
 * T Abnormal termination of the process.
 * A Abnormal termination of the process [XSI] [Option Start]  with additional actions. [Option End]
 * I Ignore the signal.
 * S Stop the process.
 * C Continue the process, if it is stopped; otherwise, ignore the signal.
 */

#define sigmask(sig)	(1UL << ((sig) - 1))


#define SIG_BLOCK 1//增加一个信号.
#define SIG_UNBLOCK 2//解除一个信号.
#define SIG_SETMASK 3//该进程的信号将被set信号集取代.
typedef void (*sighandler_t)(int);
#define SIG_ERR ((sighandler_t)-1) //错误
#define SIG_IGN  ((sighandler_t)1) // 忽略该信号.
#define SIG_HOLD ((sighandler_t)2) //将 sig 添加到进程的信号掩码.
#define SIG_DEF ((sighandler_t)3) // 默认操作.
struct siginfo_t
{
    int           si_signo;  //Signal number.
    int           si_code;  //Signal code.
    int           si_errno;  //If non-zero, an errno value associated with this signal, as described in <errno.h>.
    pid_t         si_pid;    //Sending process ID.
    uid_t         si_uid;    //Real user ID of sending process.
    void         *si_addr;   //Address of faulting instruction.
    int           si_status; //Exit value or signal.
    long          si_band;   //Band event for SIGPOLL.
    union sigval  si_value;  //Signal value.
};
struct sigaction
{
    void   (*sa_handler)(int);  //Pointer to a signal-catching function or one of the SIG_IGN or SIG_DFL.
    sigset_t sa_mask;           //Set of signals to be blocked during execution of the signal handling function.
    int      sa_flags;          //Special flags.
    void   (*sa_sigaction)(int,struct siginfo_t *, void *); //Pointer to a signal-catching function.
};

struct stack_t
{
    void     *ss_sp;       //Stack base or pointer.
    size_t    ss_size;     //Stack size.
    int       ss_flags;    //Flags.
};

typedef struct mcontext_t
{
    struct ucontext_t *uc_link;     //Pointer to the context that is resumed when this context returns.
    sigset_t    uc_sigmask;  //The set of signals that are blocked when this context is active.
    struct stack_t     uc_stack;   //The stack used by this context.
}ucontext_t;

struct sighand_struct {
    int	count;
    struct sigaction	action[_NSIG];
};

struct sigpending {
    sigset_t signal;
};

int    sigkill(int, int);
int    killpg(int, int);
int    pthread_kill(pthread_t, int);
int    pthread_sigmask(int, const sigset_t *, sigset_t *);
int    raise(int);
int    sigaddset(sigset_t *, int);//
int    sigaltstack(const struct stack_t *restrict, struct stack_t *restrict);
int    sigdelset(sigset_t *, int);//
int    sigemptyset(sigset_t *);//
int    sigfillset(sigset_t *);//
int    sighold(int);
int    sigignore(int);
int    siginterrupt(int, int);//
int    sigismember(const sigset_t *, int);//
int    sigisemptyset(sigset_t *);//
void   sigdelsetmask(sigset_t *, unsigned long );//
void (*signal(int, void (*)(int)))(int);
int    sigpause(int);
int    sigpending(sigset_t *);
int    sigprocmask(int, const sigset_t *restrict, sigset_t *restrict);
int    sigqueue(int, int, const union sigval);
int    sigrelse(int);
void (*sigset(int, void (*)(int)))(int);
int    sigsuspend(const sigset_t *);
int    sigtimedwait(const sigset_t *restrict, struct siginfo_t *restrict,
                    const struct timespec *restrict);
int    sigwait(const sigset_t *restrict, int *restrict);
int    sigwaitinfo(const sigset_t *restrict, struct siginfo_t *restrict);
void sigdefhandler(int);
#endif //XV6_IMPROVED_SIGNAL_H
