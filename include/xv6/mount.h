struct inode;
struct mountsw {
  uint dev;
  struct inode *dp;
  uint fsid;
};

struct fstable {
  int (*read)(struct inode*, char*, uint off, uint n);
  int (*write)(struct inode*, char*, uint off, uint n);
};

extern struct mountsw mountsw[];
extern struct mountsw *mntswend;

int mount(int dev, char *path, int fs);
int unmount(int dev);

struct fstable *getfs(int fsid);
uint getindev(struct inode*);

struct node;
int deffsread(struct inode*, char*, uint, uint);
int deffswrite(struct inode*, char*, uint, uint);
