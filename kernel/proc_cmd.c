#include "xv6/proc_fs.h"
#include "xv6/user.h"

#define pList 0
#define pCd 1
#define pCat 2
#define pNothing 3

void exec_proc_cmd(char* buf)
{	
  struct proc_cmd cmd;
	parse_proc_cmd(buf,&cmd);
	switch(cmd.type)
	{
		case pList:
			plist_cmd(cmd.filepath);
			break;

		case pCd:
			pcd_cmd(cmd.filepath);
			break;

		case pCat:
			pcat_cmd(cmd.filepath);
			break;

		default:
			cprintf("Can not resolve cmd");
	}
}

void parse_proc_cmd(char* buf,struct proc_cmd*cmd)
{
  int i = 0, len = strlen(buf);
  while(buf[i] == ' ')
    i++;

  if(buf[i] == 'c' && buf[i+1] == 'd')
    cmd->type = pCd;
  else if(buf[i] == 'l' && buf[i+1] == 's')
    cmd->type = pList;
  else if(buf[i] == 'c' && buf[i+1] == 'a' && buf[i+2] == 't')
    cmd->type = pCat;
  else
  {
    cmd->type = pNothing;
    return;
  }

  while(buf[i] != ' ')
    i++;
  while(buf[i] == ' ')
    i++;
  int content_len = len - i;

  for(int j = 0; j < content_len; j++)
    cmd->filepath[j] = buf[i+j];
}

void plist_cmd(char* path)
{
	uint len = strlen(path);
	if(len == 0)
		path[0] = '.';
  char* page;
  page = kalloc();
	if(page)
	{
		if(read_proc(path, page)!=-1)
      cprintf("%s", page);
	}

}

void pcd_cmd(char* path)
{
	int len = strlen(path);
	if(len == 0)
		path[0] = '.';

}

void pcat_cmd(char* path)
{

}