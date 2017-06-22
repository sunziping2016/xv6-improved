#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/stat.h"
#include "xv6/fs.h"
#include "xv6/mount.h"

struct mountsw mountsw[NDEV];
struct mountsw *mntswend;
struct fstable fstable[NDEV];

int
regfs(int fsid, struct fstable *fs)
{
    fstable[fsid] = *fs;
    return 0;
}

struct fstable*
getfs(int fsid)
{
    return fstable + fsid;
}

int
mount(int dev, char *path, int fs)
{
    if (mntswend - mountsw >= NDEV)
        return -1;
    mntswend->dev = dev;
    mntswend->dp = namei(path);
    mntswend->fsid = fs;
    mntswend++;
    return 0;
}

int unmount(int dev)
{
    struct mountsw *mp;
    for (mp = mountsw; mp != mntswend; mp++)
        if (mp->dev == dev)
        {
            *mp = *(--mntswend);
            return 0;
        }
    return -1;
}

int
sys_mount(void)
{
    int dev, fs;
    char *path;
    argint(0, &dev);
    argstr(1, &path);
    argint(2, &fs);
    mount(dev, path, fs);
    return 0;
}

int
sys_unmount(void)
{
    int dev;
    argint(0, &dev);
    unmount(dev);
    return 0;
}
