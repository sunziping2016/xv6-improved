#include "xv6/user.h"
#include "xv6/proc_fs.h"
#define NPDE 100

struct proc_dir_entry *_proc_mkdir(const char *name,enum pdetype type,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc)//插入顺序待改
{
  if(parent->type!=PDE_DIR)
    return 0;
  struct proc_dir_entry*newpde=parent;
  //search if exist
  while(newpde!=0)
  {
    if(strcmp(newpde->name,name)==0)
      return 0;
    newpde=newpde->next;
  }
  //alloc
  unsigned short len=strlen(name);
  void*p;
  if((p=malloc(sizeof(struct proc_dir_entry)+len+1))==0)
    return 0;
  newpde=(struct proc_dir_entry*)p;
  newpde->name=(char*)(p+sizeof(struct proc_dir_entry));
  //init
  strcpy(newpde->name,name);
  newpde->namelen=len;
  newpde->type=type;
  newpde->data=data;
  newpde->read_proc=read_proc;
  //newpde.write_proc=write_proc;
  //insert
  newpde->subdir=0;
  newpde->parent=parent;
  newpde->next=parent->subdir;
  acquire(&(parent->lock));
  parent->subdir=newpde;
  release(&(parent->lock));
  return newpde;
}

void _add_(struct proc_dir_entry *pde)
{
  if(pde->type==PDE_DIR&&strcmp(pde->name,".")!=0&&strcmp(pde->name,"..")!=0)
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
    if(strcmp(p->name,name)==0)
      goto found;
    q=p;
    p=p->next;
  }
  return;
  found:
  acquire(&(p->lock));
  if(q==0)
  {
    acquire(&(parent->lock));
    parent->subdir=p->next;
    release(&(parent->lock));
  }
  else {
    acquire(&(parent->lock));
    q->next=p->next;
    release(&(q->lock));
  }
  free(p->name);
  free(p);
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
      cprintf("file not found!");
      return 0;
    }
    currentIndex = nextIndex;
    nextIndex++;
  }
  return currentPDE;
}
//proc_root_lookup proc_lookup proc_pid_lookup 

