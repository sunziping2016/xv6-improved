//[ Xv6 Networking ] Socket declaration
struct socket;

struct file {
  //[ Xv6 Networking ]
  //* Add socket file descriptor type
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_SOCK } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  //[ Xv6 Networking ] Add socket reference
  struct socket* sock;
  uint off;
};


// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  int flags;          // I_BUSY, I_VALID

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};
#define I_BUSY 0x1
#define I_VALID 0x2

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int, int);
  int (*write)(struct inode*, char*, int, int);
};

extern struct devsw devsw[][MDEV];



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
#define NDEVPERFCTR 1
#define MDEVPERFCTR 7
#define NDEVFULL 1
#define MDEVFULL 8
//PAGEBREAK!
// Blank page.
