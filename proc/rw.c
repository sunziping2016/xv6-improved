#include "xv6/defs.h"
#include "xv6/proc.h"
#include "internal.h"

int 
num_to_str(char*str,unsigned short slen,unsigned int num,unsigned short offset)
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

static int 
proc_read_data(char *page, int count, void *data)
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
                            