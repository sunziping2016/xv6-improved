#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"
#include "xv6/param.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/fs.h"
#include "xv6/file.h"
#include "xv6/proc_fs.h"

void  proc_root_init(void)
{
  root=pde_alloc(PDE_DIR);
  safestrcpy(root->name,"proc",5);
  root->namelen=4;
  root->type=PDE_DIR;
  root->data=root;
  root->read_proc=read_dir_list;
  //insert
  root->subdir=0;
  root->parent=0;
  root->next=0;
  root->pre=0;
  updatepinode(root);
  //allocproci(root);
}
void  proc_cpuinfo_init(void)
{
  proc_mkdir("cpuinfo",PDE_FILE,root, 0, read_cpuinfo);
}
void  proc_add_process(struct proc*process)
{
  if(root==0) return;
  char numstr[10];
  num_to_str(numstr,process->pid,0);
  struct proc_dir_entry*proc_dir=proc_mkdir(numstr,PDE_DIR,root,0,read_dir_list);
  proc_mkdir("status",PDE_FILE,proc_dir,process,read_proc_stat);
}

void proc_del_process(unsigned int pid)
{
  int i,n;
  struct proc_dir_entry*p;
  if(root==0) return;
  p=root->subdir;
  while(p!=0)
  {
    if(p->type==PDE_DIR)
    {
      i=0;n=0;
      while(i<p->namelen)
      {
        n=n*10+p->name[i]-'0';
        i++;
      }
      if(n==pid)
      {
        remove_proc_entry_recursive(p);
        return;
      }
    }
    p=p->next;
  }
}

/*void proc_update()
{
  
}*/

void  proc_init(void)
{
  //cprintf("procinit\n");
  proc_root_init();
  proc_cpuinfo_init();
  //proc_update()；
}
