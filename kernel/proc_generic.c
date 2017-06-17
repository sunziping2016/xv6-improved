#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/proc_fs.h"

struct proc_dir_entry*pde_alloc(enum pdetype type)
{
  int i;
  struct proc_dir_entry *p;
  acquire(&pdetable.lock);
  for(i=0;i<NPDE;i++)
  {
    if(pdetable.pde[i].type==PDE_NONE)
    {
      p=&pdetable.pde[i];
      p->type=type;
      p->id=i+1;
      release(&pdetable.lock);
      return p;
    }
  }
  release(&pdetable.lock);
  return 0;
}

struct proc_dir_entry *_proc_mkdir(const char *name,enum pdetype type,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc)//插入顺序待改
{
  if(parent->type!=PDE_DIR)
    return 0;
  struct proc_dir_entry*newpde,*p,*q;
  p=parent;
  //search if exist
  while(p!=0)
  {
    if(strncmp(p->name,name,p->namelen)==0&&p->type==type)
      return 0;
    p=p->next;
  }
  //alloc
  if((newpde=pde_alloc(type))==0)
    return 0;
  //init
  newpde->namelen=strlen(name);
  safestrcpy(newpde->name,name,newpde->namelen+1);
  newpde->type=type;
  newpde->data=data;
  newpde->read_proc=read_proc;
  newpde->subdir=0;
  newpde->parent=parent;
  //insert
  p=parent->subdir;
  q=parent;
  while(p!=0)
  {
    if((strncmp(p->name,newpde->name,p->namelen)>0&&p->type==newpde->type)
      ||(p->type==PDE_DIR&&newpde->type==PDE_FILE))
      break;
    p=p->next;
    q=p;
  }
  newpde->next=p;
  if(q==parent)
  {
    acquire(&pdetable.lock);
    parent->subdir=newpde;
    release(&pdetable.lock);
  }
  else {
    q->next=newpde;
  }

  return newpde;
}

void _add_(struct proc_dir_entry *pde)
{
  if(pde->type==PDE_DIR&&strncmp(pde->name,".",1)!=0&&strncmp(pde->name,"..",2)!=0)
  {
    struct proc_dir_entry *self=proc_mkdir(".",PDE_DIR,pde,pde,pde->read_proc);
    self->subdir=pde->subdir;
    struct proc_dir_entry *parent=proc_mkdir("..",PDE_DIR,pde,pde->parent,pde->parent->read_proc);
    parent->subdir=pde->parent->subdir;
  }
}

struct proc_dir_entry *proc_mkdir(const char *name,enum pdetype type,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc)
{
  struct proc_dir_entry *n=_proc_mkdir(name,type,parent,data,read_proc);
   if(type==PDE_DIR)
     ;//_add_(n);
}

void remove_proc_entry(const char *name, struct proc_dir_entry *parent)//非递归
{
  struct proc_dir_entry*p=parent->subdir;
  struct proc_dir_entry*q=0;
  while(p!=0)
  {
    if(strncmp(p->name,name,p->namelen)==0)
      goto found;
    q=p;
    p=p->next;
  }
  return;
  found:
  acquire(&pdetable.lock);
  if(q==0)
    parent->subdir=p->next;
  else q->next=p->next;
  release(&pdetable.lock);
}

