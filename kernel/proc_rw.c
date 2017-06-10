#include "xv6/defs.h"
#include "xv6/proc.h"
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
  return len;
}

int 
proc_dir_to_str(char*str,unsigned short slen,struct proc_dir_entry*dir,unsigned short offset)
{
  
}

int 
inode_dir_to_str(char*str,unsigned short slen,struct inode*dir,unsigned short offset)
{
  
}

int 
read_line(char*page,const char*desc,unsigned int num,unsigned int off) 
{
  int len=0;
  int l=strlen(desc);
  len+=l;
  safestrcpy(page+off,desc,l);
  len+=num_to_str(page,num,off+len);
  safestrcpy(page+off+len,"\n",1);
  len++;
  return len;
}

int 
read_proc_stat(char *page,void *data)
{
  int n=0;
  /*
  ProcessID:
  Process state:
  Parent process:
  Current directory:
  Process name:
  ...
  
  */
  return n;
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
}
//读文件夹
int 
read_dir_list(char *page,void *data)
{
  int off=0;
  struct proc_dir_entry*p=((struct proc_dir_entry*)data)->subdir;
  while(p!=0)
  {
    safestrcpy(page+off,p->name,p->namelen);
    off+=p->namelen;
    if(p->type==PDE_DIR)
    {
      safestrcpy(page+off,"  DIR\n",6);
      off+=6;
    }
    else 
    {
      safestrcpy(page+off,"  FILE\n",7);
      off+=7;
    }
  }
  page[off]=0;
  off++;
  return off;
}
//读某proc文件
int 
read_proc_file(struct proc_dir_entry *f, char *page)
{
  if(f->type==PDE_NONE)
    return -1;
  else return (*(f->read_proc))(page,f->data);
}

int read_proc(char*name, char *page)
{
  //proc_update();
  struct proc_dir_entry*s=proc_lookup(name);
  if(s==0)
  {
    cprintf("proc file not exist\n");
    return -1;
  }
  read_proc_file(s,page);
  /*if(n==-1)
  {
    cprintf("proc file read failed");
    return -1;
  }*/
}







                            