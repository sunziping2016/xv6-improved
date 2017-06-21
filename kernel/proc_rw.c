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

void strcopy(char *s,char *t)
{
    int lens = strlen(s),lent = strlen(t);
    int i = 0;
    for(i=lent;i<lent + lens;i++)
    {
        s[i] = t[i-lent];
    }
    s[i] = '\0';
}


void Inttostring(uint num,char *str)
{
    char temp[33];
    int i = 0 ;
    while(1)
    {
        temp[i] = num % 10;
        i =  i + 1;
        if( num/10<=0)
           break;
        num = num/10;
    }
    int j =0;
    for(j;j < i;j++)
       str[i] = temp[i-j-1];
    str[j] = '\0';
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
read_line(char*page,const char*desc,unsigned int num,unsigned int off) 
{
  int len=0;
  int l=strlen(desc);
  len+=l;
  strncpy(page+off,desc,l);
  len+=num_to_str(page,num,off+len);
  strncpy(page+off+len,"\n",1);
  len++;
  return len;
}

int 
read_proc_stat(char *page,void *data)
{
  /*
  ProcessID:
  Process state:
  Parent process:
  Current directory:
  Process name:
  ...
  
  */
  int off=0;
  struct proc*p=(struct proc*)data;
  off=off+read_line(page,"Process ID:",p->pid,off);
  return off;
  /*struct proc*m_proc=(struct proc*)data;
  char *name,*str;
     str = kalloc();
    
     //process
     name = "process id: ";
     uint s = (uint)m_proc->pid;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     name = "process name: ";
     str = m_proc->name;
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     name = "process memory size: ";
     s = m_proc->sz;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);

     name = "Process state: ";
     s = m_proc->state;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     name = "bottom of kernel stack for this process: ";
     str = m_proc->kstack;
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
    
     name = "process to be killed: ";
     s = (uint)m_proc->killed;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
    
     strcopy(page, "\n");

     kfree(str);
     return strlen(page);*/
}

int 
read_cpuinfo(char *page, void *data)
{
  /*
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?

  // Cpu-local storage variables; see below
  struct cpu *cpu;
  struct proc *proc;           // The currently-running process.
  */
  int off=0;
  for(int i=0;i<ncpu;i++)
  {
    off=off+read_line(page,"Local APIC ID:",(int)(cpus[i].apicid),off);
    off=off+read_line(page,"The currently-running process ID:",(cpus[i].proc)->pid,off);
  }
  page[off]=0;
  off++;
  return off;
  /*cprintf("read cpu\n");
  struct cpu*m_cpu=(struct cpu*)data;
  
  char *name,*str;

     str = (char*)kalloc();

     //CPU
     name = "CPU has been started:";
     uint s = m_cpu->started;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
  
     name = "CPU ID: ";
     str = (char)m_cpu->apicid;
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     name = "current PCB running on CPU:";
     strcopy(page, name);
     strcopy(page, "\n");
     
     name = "edi: ";
     s = m_cpu->scheduler->edi;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
    
     name = "esi: ";
     s = m_cpu->scheduler->esi;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);

     name = "ebx: ";
     s = m_cpu->scheduler->ebx;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     name = "ebp: ";
     s = m_cpu->scheduler->ebp;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);

     name = "eip: ";
     s = m_cpu->scheduler->eip;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
    
     name = "depth of pushcli nesting: ";
     s = (uint)m_cpu->ncli;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     name = "enable interrupts before pushcli: ";
     s = (uint)m_cpu->intena;
     Inttostring(s,str);
     strcopy(str,"\n");
     strcopy(page, name);
     strcopy(page, str);
     
     strcopy(page, "\n");
     
     kfree(str);
       cprintf("read cpu end\n");
     return strlen(page); */
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





                            