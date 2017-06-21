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

struct proc_dir_entry *proc_mkdir(const char *name,enum pdetype type,struct proc_dir_entry *parent, void *data, read_proc_t *read_proc)
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
  strncpy(newpde->name,name,newpde->namelen+1);
  newpde->type=type;
  newpde->data=data;
  if(type==PDE_DIR&&data==0)
    newpde->data=newpde;
  newpde->read_proc=read_proc;
  newpde->subdir=0;
  newpde->parent=parent;
  //insert
  p=parent->subdir;
  q=0;
  while(p!=0)
  {
    if((strncmp(p->name,newpde->name,p->namelen)>0&&p->type==newpde->type)
      ||(p->type==PDE_FILE&&newpde->type==PDE_DIR))
        break;
    q=p;
    p=p->next;
  }
  newpde->next=p;
  newpde->pre=q;
  acquire(&pdetable.lock);
  if(q==0)
    parent->subdir=newpde;
  else q->next=newpde;
  if(p!=0)
    p->pre=newpde;
  
  updatepinode(newpde);
  updatepinode(parent);
  release(&pdetable.lock);
  return newpde;
}

void updatepinode(struct proc_dir_entry *p)
{  
  (p->pinode).dev=PROCFSDEV;
  (p->pinode).inum=p->id;
  if(p->type==PDE_DIR)
    (p->pinode).type=1;
  else if(p->type==PDE_FILE)
    (p->pinode).type=2;
  (p->pinode).size=getsize(p);
}

void _remove_proc_entry(struct proc_dir_entry *pde)//非递归
{
  //removeproci(pde);
  acquire(&pdetable.lock);
  if(pde->pre==0)
    pde->parent->subdir=pde->next;
  else pde->pre->next=pde->next;
  if(pde->next!=0)
    pde->next->pre=pde->pre;
  pde->type=PDE_NONE;
  release(&pdetable.lock);
}

void remove_proc_entry_recursive(struct proc_dir_entry *pde)
{
  struct proc_dir_entry *p;
  if(pde->type==PDE_DIR)
  {
    p=pde->subdir;
    while(p!=0)
    {
      remove_proc_entry_recursive(p);
      p=p->next;
    }
  }
  _remove_proc_entry(pde);
}

void remove_proc_entry(const char *name, struct proc_dir_entry *parent)//非递归
{
  struct proc_dir_entry*p=parent->subdir;
  while(p!=0)
  {
    if(strncmp(p->name,name,p->namelen)==0)
    {
      remove_proc_entry_recursive(p);
      break;
    }
    p=p->next;
  }
}

