#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"

char buf[512];
unsigned char devname[][20] = {
        "dev/full",
        "dev/zero",
        "dev/null",
        "dev/hdainfo",
        "dev/hda",
        "dev/random",
        "dev/urandom",
        "dev/sound",
        "dev/perfctr"
};
#define devnamenum 9
#define devtest_interval 20000000
char hdatext[] = {"kai hei ma? wo ya se zei 6.   "};
char hdainfotext[] = {"800"};
char soundtext[]={"1155665 4433221 5544332 5544332 1155665 4433221"};
void
testdev(int devnum, int fd)
{
      int n = 512;
      int sleep_devtest = 0;
      switch(devnum)
      {
        case 0:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/full\n");
        n = 512;
        if(write(fd, buf, n) != n)
        {
            printf(1, "write error\n");
            exit();
        }
        break;
        case 1:
        /*printf(1, "read from /dev/zero\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            printf(1, "read error\n");
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }*/
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nwrite to /dev/zero\n");
        n = 512;
        if(write(fd, buf, n) != n)
        {
            printf(1, "write error\n");
            exit();
        }
        break;
        case 2:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/null\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nwrite to /dev/null\n");
        n = 512;
        if(write(fd, buf, n) != n)
        {
            printf(1, "write error\n");
            exit();
        }
        break;
        case 3:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nwrite to /dev/hdainfo\n");
        n = sizeof(hdainfotext);
        n--;
        if(write(fd, hdainfotext, n) != n)
        {
            printf(1, "write error\n");
            exit();
        }
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/hdainfo\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        break;
        case 4:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/hda\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nwrite to /dev/hda\n");
        n = sizeof(hdatext);
        if(write(fd, hdatext, n) != n)
        {
            printf(1, "write error\n");
            exit();
        }
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/hda\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        break;
        case 5:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/random\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        break;
        case 6:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/urandom\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        break;
        case 7:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nwrite to /dev/sound\n");
        n = sizeof(soundtext);
        if(write(fd, soundtext, n) != n)
        {
            printf(1, "write error\n");
            exit();
        }
        break;
        case 8:
        sleep_devtest = 0;
        while (sleep_devtest < devtest_interval)sleep_devtest++;
        printf(1, "\nread from /dev/perfctr\n");
        n = 512;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (write(1, buf, n) != n) {
                printf(1, "read error\n");
                exit();
            }
        }
        break;
        default:
        break;
      }
}

int
main(int argc, char *argv[])
{
    int fd, i;
    for (i = 0; i < devnamenum; i++) {
        //printf(1, "%s \n", devname[i]);
        if (fd = open(devname[i], 2) < 0 ) {
            printf(1, "test: cannot execiute this instruction\n");
            exit();
        }
        testdev(i, fd);
        close(fd);
    }
    exit();
}
