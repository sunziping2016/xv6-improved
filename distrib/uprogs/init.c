// init: The initial user-level program

#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fcntl.h"
#define NDEVHDA  2
#define MDEVHDA  1
#define NDEVHDAINFO 2
#define MDEVHDAINFO 2
#define NCONSOLE 1
#define MCONSOLE 1
#define NDEVSOUND 1
#define MDEVSOUND 2
#define NDEVNULL 1
#define MDEVNULL 3
#define NDEVZERO 1
#define MDEVZERO 4
#define NDEVRANDOM 1
#define MDEVRANDOM 5
#define NDEVURANDOM 1
#define MDEVURANDOM 6

char *argv[] = { "sh", 0 };

int
main(void)
{
    int pid, wpid;
    mkdir("dev");
    chdir("dev");
    mknod("hdainfo",NDEVHDAINFO,MDEVHDAINFO);
    mknod("hda",NDEVHDA,MDEVHDA);
    if (open("console", O_RDWR) < 0) {
        mknod("console", NCONSOLE, MCONSOLE);
        open("console", O_RDWR);
    }
    mknod("sound", NDEVSOUND, MDEVSOUND);
    mknod("null",NDEVNULL,MDEVNULL);
    mknod("zero",NDEVZERO,MDEVZERO);
    mknod("random",NDEVRANDOM,MDEVRANDOM);
    mknod("urandom",NDEVURANDOM,MDEVURANDOM);
    dup(0);  // stdout
    dup(0);  // stderr
    chdir("..");
    for (;;) {
        printf(1, "init: starting sh\n");
        pid = fork();
        if (pid < 0) {
            printf(1, "init: fork failed\n");
            exit();
        }
        if (pid == 0) {
            exec("sh", argv);
            printf(1, "init: exec sh failed\n");
            exit();
        }
        while ((wpid = wait()) >= 0 && wpid != pid)
            printf(1, "zombie!\n");
    }
}
