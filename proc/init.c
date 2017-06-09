#include "xv6/proc.h"
#include "internal.h"
void  proc_root_init(void)
{
  root=(proc_dir_entry*)malloc(sizeof(struct proc_dir_entry)+len+1);
  
}
void  proc_cpuinfo_init(void)
{
  proc_mkdir("cpuinfo",PDE_FILE,root, 0, read_cpuinfo);
}
void  proc_process_init(proc*process)
{
  char numstr[10];
  num_to_str(numstr,10,process.pid,0);
  proc_dir_entry*proc_dir=proc_mkdir(numstr,PDE_DIR,root,process,read_dir_list);
  proc_mkdir("status",PDE_FILE,proc_dir,process,read_proc_stat);
}

void proc_update()
{
  
}

void __init proc_init(void)
{
  proc_root_init();
  proc_cpuinfo_init();
  proc_update()；
}
