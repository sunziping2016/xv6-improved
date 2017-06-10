#include "xv6/proc.h"
#include "xv6/proc_fs.h"

void  proc_root_init(void)
{
  procfs = 0;
  root=pde_alloc(PDE_DIR);
  safestrcpy(root->name,"proc",5);
  root->namelen=4;
  root->type=PDE_DIR;
  root->data=0;
  root->read_proc=read_dir_list;
  //newpde.write_proc=write_proc;
  //insert
  root->subdir=0;
  root->parent=root;
  root->next=0;
}
void  proc_cpuinfo_init(void)
{
  proc_mkdir("cpuinfo",PDE_FILE,root, 0, read_cpuinfo);
}
void  proc_process_init(struct proc*process)
{
  char numstr[10];
  num_to_str(numstr,process->pid,0);
  struct proc_dir_entry*proc_dir=proc_mkdir(numstr,PDE_DIR,root,process,read_dir_list);
  proc_mkdir("status",PDE_FILE,proc_dir,process,read_proc_stat);
}
/*
void proc_update()
{
  
}
*/
void  proc_init(void)
{
  proc_root_init();
  proc_cpuinfo_init();
  //proc_update()；
}
