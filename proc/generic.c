#define NPDE 100

struct {
    struct spinlock lock;
    struct proc_dir_entry pde[NPDE];
} proc_table;

struct proc_dir_entry *proc_alloc(void)
{
  acquire(&proc_table.lock);
  for(int i=0; i<NPDE;i++)
  {
    if(proc_table[i].pdestate==UNUSED)
    {
      proc_table[i].pdestate=USED;
      release(&proc_table.lock);
      return &proc_table[i];
    }
  }
  release(&proc_table.lock);
  return 0;
}
struct proc_dir_entry *proc_mkdir(const char *name,unsigned int mode,struct proc_dir_entry *parent, void *data)
{
  
}
void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
{
  
}

struct proc_dir_entry*proc_lookup(const char *name)//识别绝对和相对路径
{
  
}
//proc—_root_lookup proc_lookup proc_pid_lookup 

