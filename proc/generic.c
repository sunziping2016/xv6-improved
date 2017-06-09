#include "xv6/user.h"

#include "internal.h"
#define NPDE 100

struct proc_dir_entry *proc_mkdir(const char *name,unsigned int mode,unsigned int pdetype,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc,write_proc_t *write_proc)
{
  if(parent.pdetype!=1)
    return 0;
  proc_dir_entry*newpde=parent;
  //search if exist
  while(newpde!=0)
  {
    if(strcmp(newpde.name,name)==0)
      return 0;
    newpde=newpde->next;
  }
  //alloc
  if((newpde=(proc_dir_entry*)malloc(sizeof(proc_dir_entry)))==0)
    return 0;
  acquire(&pde_table.lock);
  //init
  newpde.name=name;
  newpde.namelen=strlen(newpde.name);
  newpde.mode=mode;
  newpde.pdetype=pdetype;
  newpde.data=data;
  newpde.read_proc=read_proc;
  newpde.write_proc=write_proc;
  //insert
  newpde.subdir=0;
  newpde.parent=parent;
  newpde.next=parent.subdir;
  parent.subdir=newpde;
  
  release(&pde_table.lock);
  return newpde;
}
void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
{
  proc_dir_entry*p=parent.subdir;
  proc_dir_entry*q=0;
  while(p!=0)
  {
    if(strcmp(p.name,name)==0)
      goto found;
    q=p;
    p=p->next;
  }
  return;
  found:
  acquire(&pde_table.lock);
  p->pdestate=UNUSED;
  if(q==0)
    parent.subdir=p->next;
  else q->next=p->next;
  free(p);
  release(&pde_table.lock);
}

struct proc_dir_entry*proc_lookup(const char *name)//识别绝对和相对路径
{
  
}
//proc_root_lookup proc_lookup proc_pid_lookup 

