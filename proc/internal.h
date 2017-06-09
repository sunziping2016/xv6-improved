//copy from Linux
struct proc_dir_entry
{
	unsigned short namelen;
  const char *name;
  unsigned int mode;
  unsigned int pdetype;//0 None 1 Dir 2 File
  struct proc_dir_entry *next, *parent, *subdir;
	void *data;
  int (*read_proc)(char *page, int count, void *data)
  //read_proc_t *read_proc;
	//write_proc_t *write_proc;
  struct spinlock lock;
};

struct proc_dir_entry*root;

/*struct proc_dir_entry
{
  unsigned int low_ino;	//inode号
	unsigned short namelen;
	const char *name;
	mode_t mode;
	nlink_t nlink;//子目录和软链接的数目
	uid_t uid;
	gid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	get_info_t *get_info;
	struct module *owner;
	struct proc_dir_entry *next, *parent, *subdir;
	void *data;
	read_proc_t *read_proc;
	write_proc_t *write_proc;
	atomic_t count; //use count
	int deleted; //delete flag
	kdev_t rdev; 
};
struct proc_inode {
	struct pid *pid;
	unsigned int fd;
	union proc_op op;
	struct proc_dir_entry *pde;
	struct ctl_table_header *sysctl;
	struct ctl_table *sysctl_entry;
	struct list_head sysctl_inodes;
	const struct proc_ns_operations *ns_ops;
	struct inode vfs_inode;
};*/
/*
//创建普通文件
struct proc_dir_entry* create_proc_entry(const char* name,mode_t mode,struct proc_dir_entry* parent);
//创建链接
struct proc_dir_entry* proc_symlink(const char* name, struct proc_dir_entry* parent, const char* dest);
//创建设备文件
struct proc_dir_entry* proc_mknod(const char* name, mode_t mode, struct proc_dir_entry* parent, kdev_t rdev);
//创建目录
struct proc_dir_entry* proc_mkdir(const char* name, struct proc_dir_entry* parent);
struct proc_dir_entry *proc_mkdir(const char *name,struct proc_dir_entry *parent)
{
       return proc_mkdir_mode(name, S_IRUGO | S_IXUGO, parent);
}
//移除
void remove_proc_entry(const char* name, struct proc_dir_entry*parent);

//回调函数
int read_func(char* page, char** start, off_t off, int count,int* eof, void* data);


//


struct proc_dir_entry* create_proc_read_entry(const char* name,mode_t mode, struct proc_dir_entry* parent,read_proc_t* read_proc, void* data);

static inline struct proc_dir_entry *create_proc_read_entry(const char *name,mode_t mode, struct proc_dir_entry *base,read_proc_t *read_proc, void * data)
{
    struct proc_dir_entry *res=create_proc_entry(name,mode,base);
    if(res)
    {
        res->read_proc=read_proc;
        res->data=data;
    }
      return res;
}
*/