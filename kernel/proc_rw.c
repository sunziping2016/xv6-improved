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

int 
num_to_str(char*str,unsigned int num,unsigned int offset)
{
  char numstr[20]="0";
  int len=0;
  while(num>0)
  {
    numstr[len]=num%10+'0';
    len++;
    num=num/10;
  }
  if(len==0) len++;
  for(int i=0;i<len;i++)
    str[offset+i]=numstr[len-1-i];
  str[offset+len]=0;
  return len;
}
/*int 
proc_dir_to_str(char*str,unsigned short slen,struct proc_dir_entry*dir,unsigned short offset)
{
  
}

int 
inode_dir_to_str(char*str,unsigned short slen,struct inode*dir,unsigned short offset)
{
  
}*/

int 
read_line_uint(char*page,const char*desc,unsigned int num,unsigned int off) 
{
  int len=0;
  int l=strlen(desc);
  strncpy(page+off+len,desc,l);
  len+=l;
  len+=num_to_str(page,num,off+len);
  page[off+len]='\n';
  len++;
  return len;
}

int read_line_char(char*page,const char*desc,char*data,unsigned int off)
{
  int l,len=0; 
  l=strlen(desc);
  strncpy(page+off+len,desc,l);len+=l;
  l=strlen(data);
  strncpy(page+off+len,desc,l);len+=l;
  page[off+len]='\n';len++;
  return len;
}

int 
read_proc_stat(char *page,void *data)
{
  struct proc*m_proc=(struct proc*)data;
  int off=0;
  off=off+read_line_uint(page,"process id: ",(uint)m_proc->pid,off);
  off=off+read_line_char(page,"process name: ",m_proc->name,off);
  off=off+read_line_uint(page,"process memory size: ",(uint)m_proc->state,off);
  off=off+read_line_uint(page,"Process state: ",(uint)m_proc->pid,off);
  off=off+read_line_uint(page,"process to be killed: ",(uint)m_proc->killed,off);
  page[off]=0;
  off++;
  return off;
}

int 
read_cpuinfo(char *page, void *data)
{
  struct cpu*m_cpu;
  int off=0;
  for(int i=0;i<ncpu;i++)
  {
    m_cpu=&(cpus[i]);
    off=off+read_line_uint(page,"CPU has been started:             ",m_cpu->started,off);
    off=off+read_line_uint(page,"CPU ID:",(int)(m_cpu->apicid),off);
    off=off+read_line_uint(page,"current PCB running on CPU:       ","",off);
    off=off+read_line_uint(page,"edi:                              ",m_cpu->scheduler->edi,off);
    off=off+read_line_uint(page,"esi:                              ",m_cpu->scheduler->esi,off);
    off=off+read_line_uint(page,"ebx:                              ",m_cpu->scheduler->ebx,off);
    off=off+read_line_uint(page,"ebp:                              ",m_cpu->scheduler->edi,off);
    off=off+read_line_uint(page,"eip:                              ",m_cpu->scheduler->eip,off);
    off=off+read_line_uint(page,"depth of pushcli nesting:         ",(uint)m_cpu->ncli,off);
    off=off+read_line_uint(page,"enable interrupts before pushcli: ",(uint)m_cpu->intena,off);  
  }
  page[off]=0;
  off++;
  return off;
}
//读文件夹
int 
read_dir_list(char *page,void *data)
{
  struct dirent de;
  int off=0;
  struct proc_dir_entry*p=(struct proc_dir_entry*)data;
  
  de.inum=p->id;
  strncpy(de.name,".",2);
  memmove(page+off,(char*)&de,sizeof(struct dirent));
  off+=sizeof(struct dirent);
  
  if(p->parent!=0)
    de.inum=p->parent->id;
  else de.inum=1000;
  strncpy(de.name,"..",3);
  memmove(page+off,(char*)&de,sizeof(struct dirent));
  off+=sizeof(struct dirent);
  
  p=p->subdir;
  while(p!=0)
  {
    de.inum=p->id;
    strncpy(de.name,p->name,p->namelen);
    de.name[p->namelen]='\0';
    memmove(page+off,(char*)&de,sizeof(struct dirent));
    off+=sizeof(struct dirent);
    p=p->next;
  }
  return off;
}
//读某proc文件
int 
read_proc_file(struct proc_dir_entry *f, char *page)
{
  int n;
  if(f->type==PDE_NONE)
    return -1;
  else return (f->read_proc)(page,f->data);
}
int getsize(struct proc_dir_entry *f)
{
  struct proc_dir_entry *p;
  int s=2*sizeof(struct dirent);
  if(f->type==PDE_FILE)
    return 0;
  p=f->subdir;
  while(p!=0)
  {
    s=s+sizeof(struct dirent);
    p=p->next;
  }
  return s;
}
int readproc(struct inode *ip, char *dst, unsigned int off, unsigned int n)
{
  struct proc_dir_entry *f;
  char page[4096];
  int size;
  if(ip->inum>=NPDE)
    panic("findproc");
  f=&pdetable.pde[ip->inum-1];
  size=read_proc_file(f,page);
  if(size<=off)
    return 0;
  if(n+off>size)
    n=size-off;
  memmove(dst,page+off,n);
  return n;
}

struct inode* getinode(unsigned int inum)
{
  return &(pdetable.pde[inum-1].pinode);
}





                            