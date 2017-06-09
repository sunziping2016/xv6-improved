//copy from Linux
struct proc_dir_entry
{
  unsigned int low_ino;
	umode_t mode;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	struct proc_dir_entry *parent;
	struct rb_root subdir;
	struct rb_node subdir_node;
	void *data;
	atomic_t count;		/* use count */
	atomic_t in_use;	/* number of callers into module in progress; */
			/* negative -" it's going away RSN */
	struct completion *pde_unload_completion;
	struct list_head pde_openers;	/* who did -"open, but not -"release */
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	u8 namelen;
	char name[];
};
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