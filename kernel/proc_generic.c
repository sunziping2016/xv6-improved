#include "xv6/defs.h"
#include "xv6/proc_fs.h"

struct proc_dir_entry*pde_alloc(enum pdetype type)
{
  struct proc_dir_entry *p;
  acquire(&pdetable.lock);
  for(p=pdetable.pde;p<pdetable.pde+NPDE;p++)
  {
    if(p->type==PDE_NONE)
    {
      p->type=type;
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
  struct proc_dir_entry*newpde=parent;
  //search if exist
  while(newpde!=0)
  {
    if(strncmp(newpde->name,name,newpde->namelen)==0&&newpde->type==type)
      return 0;
    newpde=newpde->next;
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
  //insert
  newpde->subdir=0;
  newpde->parent=parent;
  newpde->next=parent->subdir;
  acquire(&pdetable.lock);
  parent->subdir=newpde;
  release(&pdetable.lock);
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
    _add_(n);
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

struct proc_dir_entry *proc_lookup(const char *name)//识别绝对和相对路径
{
  if(!name)
    return;
  int currentIndex = 0;
  int nextIndex = 1;
  struct proc_dir_entry* currentPDE = root;
  unsigned short findFlag = 0;
  while(name[currentIndex] == '/')
  {
    while(name[nextIndex] != '/' && name[nextIndex] != '\0')
      nextIndex++;
    while(currentPDE != 0)
    {
      if(currentPDE->namelen != nextIndex - currentIndex - 1)
      {
        currentPDE = currentPDE->next;
        continue;
      }
      for(int i = currentIndex + 1; i < nextIndex; ++i)
      {
        if(name[i] != currentPDE->name[i - currentIndex - 1])
        {
          currentPDE = currentPDE->next;
          continue;
        }
      }
      findFlag = 1;
      currentPDE = currentPDE->subdir;
      break;
    }
    if(!findFlag)
    {
      return 0;
    }
    currentIndex = nextIndex;
    nextIndex++;
  }
  return currentPDE;
}
//proc_root_lookup proc_lookup proc_pid_lookup 

