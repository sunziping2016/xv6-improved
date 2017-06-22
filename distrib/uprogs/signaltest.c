
#include "xv6/types.h"
#include "xv6/signal.h"
#include "xv6/user.h"
#include "xv6/fcntl.h"
#include "xv6/kbd.h"

int main()
{
    static char buf[100];
    setsignal();
    while (getcmd(buf, sizeof(buf)) >= 0) {
        if (buf[0] == C('C')) {
            sigkill(getpid(), SIGINT);
        }/*发送终止进程的信号*/
        else if (buf[0] == C('E')) {
            sigset(SIGINT, SIG_HOLD);
        }/*设置阻止信号SIGINT*/
        else if (buf[0] == C('F')) {
            sigrelse(SIGINT);
        }/*解除阻止信号SIGINT*/
        else if(buf[0] == C('G')){
            raise(SIGINT);
        }/*向本进程发送终止进程的信号*/
        else if(buf[0] == C('K')){
            fork();
        }/*创建子程序*/
    }
}

int
getcmd(char *buf, int nbuf)
{
    printf(2, "$ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

void
setsignal(void)
{
    sigset(SIGSTOP, SIG_DEF);
    sigset(SIGCONT, SIG_DEF);
    sigset(SIGINT, SIG_DEF);
    sigset(SIGCHLD, SIG_DEF);
}

