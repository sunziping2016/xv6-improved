#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fs.h"
#include <string.h>

char*
curname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    if (strlen(p) >= DIRSIZ)
        return p;
    memcpy(buf, p, strlen(p));
    return buf;
}

void recursiveRm(char* path);

int
main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        printf(2, "Usage: rm files...\n");
        exit();
    }

    for (i = 1; i < argc; i++) {
        if (unlink(argv[i]) < 0) {
            recursiveRm(argv[i]);
            break;
        }
    }

    exit();
}

void recursiveRm(char *path)
{
    char buf[512], *p;
    char *filename;
    int fd, k = 2; // k for filter . & ..
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        printf(2, "rm: %s failed to delete\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf(2, "rm: %s failed to stat\n", path);
        return;
    }

    switch (st.type) {
        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf(1, "ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;
                memcpy(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if (stat(buf, &st) < 0) {
                    printf(1, "cannot stat %s\n", buf);
                    continue;
                }
                if (k)
                    --k;
                else
                    recursiveRm(buf);

            }
            close(fd);
            if (unlink(path) < 0) {
                printf(2, "rm: %s failed to delete\n", path);
                break;
            }
            break;

        default:
            close(fd);
            if (unlink(path) < 0) {
                printf(2, "rm: %s failed to delete\n", path);
                break;
            }
            break;
    }
}
