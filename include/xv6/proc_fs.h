#ifndef _XV6_PROC_FS_H
#define _XV6_PROC_FS_H

#define NPDE 100
typedef int (*read_proc_t)(char *page,void *data);

enum pdetype{PDE_NONE,PDE_DIR,PDE_FILE};

struct proc_dir_entry
{
  unsigned int id;
  unsigned short namelen;
  char name[20];
  enum pdetype type;
  struct proc_dir_entry *next,*pre, *parent, *subdir;
  void *data;
  struct inode pinode;
  read_proc_t read_proc;
  //write_proc_t *write_proc;
};

struct proc_dir_entry*root;

struct {
    struct spinlock lock;
    struct proc_dir_entry pde[NPDE];
} pdetable;

//generic.c
struct proc_dir_entry *proc_mkdir(const char *name,enum pdetype type,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc);
void _remove_proc_entry(struct proc_dir_entry *pde);
void remove_proc_entry_recursive(struct proc_dir_entry *pde);
void remove_proc_entry(const char *name, struct proc_dir_entry *parent);
void updatepinode(struct proc_dir_entry *p);

//proc_rw.c
int num_to_str(char*str,unsigned int num,unsigned int offset);
int read_proc_stat(char *page,void *data);
int read_cpuinfo(char *page,void *data);
int read_dir_list(char *page,void *data);
int read_proc_file(struct proc_dir_entry *f, char *page);
int getsize(struct proc_dir_entry *f);
struct inode* getinode(unsigned int inum);

//proc_init.c
void  proc_root_init(void);
void  proc_cpuinfo_init(void);
void  proc_add_process(struct proc*process);
void  proc_del_process(unsigned int pid);
//void  proc_update(void);

#endif