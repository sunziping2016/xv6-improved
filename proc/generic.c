#include "internal.h"
#include "xv6/user.h"
#include "internal.h"
#define NPDE 100

struct proc_dir_entry *proc_mkdir(const char *name,enum pdetype,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc)//插入顺序待改
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
  unsigned short len=strlen(newpde.name);
  if((newpde=(proc_dir_entry*)malloc(sizeof(struct proc_dir_entry)+len+1))==0)
    return 0;
  newpde.name=newpde+sizeof(struct proc_dir_entry);
  //init
  strcpy(newpde.name,name);
  newpde.namelen=len
  newpde.pdetype=pdetype;
  newpde.data=data;
  newpde.read_proc=read_proc;
  //newpde.write_proc=write_proc;
  //insert
  newpde.subdir=0;
  newpde.parent=parent;
  newpde.next=parent.subdir;
  acquire(&parent.lock);
  parent.subdir=newpde;
  release(&parent.lock);
  return newpde;
}

void remove_proc_entry(const char *name, struct proc_dir_entry *parent)//非递归
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
  acquire(&p.lock);
  if(q==0)
  {
    acquire(&parent.lock);
    parent.subdir=p->next;
    release(&parent.lock);
  }
  else {
    acquire(&parent.lock);
    q->next=p->next;
    release(&q.lock);
  }
  free(p.name);
  free(p);
}

struct proc_dir_entry *proc_lookup(const char *name)//识别绝对和相对路径
{
  if(!name)
    return;
  int currentIndex = 0;
  int nextIndex = 1;
  proc_dir_entry* currentPDE = proc_root;
  bool findFlag = false;
  while(name[currentIndex] == '/')
  {
    while(name[nextIndex] != '/' && name[nextIndex] != '\0')
      nextIndex++;
    while(currentPDE != NULL)
    {
      if(currentPDE->namelen != nextIndex - currentIndex - 1)
      {
        currentPDE = currentPDE->next;
        continue;
      }
      for(int i = currentIndex + 1; i < nextIndex; ++i)
      {
        if(path[i] != currentPDE->name[i - currentIndex - 1])
        {
          currentPDE = currentPDE->next;
          continue;
        }
      }
      findFlag = true;
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

