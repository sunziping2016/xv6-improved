#include "xv6/defs.h"
#include "xv6/proc.h"
#include "internal.h"

int num_to_str(char*str,unsigned short slen,unsigned int num,unsigned short offset)
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
  if(offset+len>slen) return 0;
  for(int i=0;i<len;i++)
    str[offset+i]=numstr[len-1-i];
  return len;
}

int proc_dir_to_str(char*str,unsigned short slen,proc_dir_entry*dir,unsigned short offset)
{
  
}

int inode_dir_to_str(char*str,unsigned short slen,inode*dir,unsigned short offset)
{
  
}

int read_proc_stat(char *page, int count, void *data)
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
}

int read_cpuinfo(char *page, int count, void *data)
{
  
}
//读文件夹
int read_dir_list(char *page, int count, void *data)
{
  
}
//读某proc文件
int read_proc_file(struct proc_dir_entry *f, char *page, int count)
{
  if(f.pdetype==PDE_NONE)
    return -1;
  else return f.read_proc(page,count,f.data);
}

int read_proc(char*name,char *page, int count)
{
  proc_update();
  proc_dir_entry*s=proc_lookup(name);
  if(s==0)
  {
    cprintf("proc file not exist");
    return -1;
  }
  int n=read_proc_file(s, char *page, int count);
  if(s==-1)
  {
    cprintf("proc file read failed");
    return -1;
  }
}







                            