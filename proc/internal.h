typedef int(*read_proc_t)(char *page, void *data)

enum pdetype{PDE_DIR,PDE_FILE};
struct proc_dir_entry
{
	unsigned short namelen;
  const char *name;
  enum pdetype;
  struct proc_dir_entry *next, *parent, *subdir;
	void *data;
  read_proc_t *read_proc;
	//write_proc_t *write_proc;
  struct spinlock lock;
};

struct proc_dir_entry*root;
struct proc_dir_entry*now;
//generic.c
struct proc_dir_entry *proc_mkdir(const char *name,enum pdetype,struct struct proc_dir_entry *parent, void *data, read_proc_t *read_proc);
void remove_proc_entry(const char *name, struct struct proc_dir_entry *parent);
struct proc_dir_entry *proc_lookup(const char *name);

//rw.c
int num_to_str(char*str,unsigned short slen,unsigned int num,unsigned short offset);
int read_proc_stat(char *page, void *data);
int read_cpuinfo(char *page, void *data);
int read_dir_list(char *page, void *data);

//init.c
void proc_update()；