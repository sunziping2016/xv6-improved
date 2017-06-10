#include "xv6/user.h"
#include "xv6/proc_fs.h"

#define pList 0
#define pCd 1
#define pCat 2
#define pNothing 3

struct proc_cmd{
  int type;
  char *filepath;
};

void proccmd(char* buf)
{

}

proc_cmd* parse_proc_cmd(char* buf)
{
  proc_cmd *cmd = malloc(sizeof(proc_cmd));
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
    cmd->filepath = NULL;
    return cmd;
  }

  while(buf[i] != ' ')
    i++;
  while(buf[i] == ' ')
    i++;
  int content_len = len - i;
  cmd->filepath = malloc(content_len);
  for(int j = 0; j < content_len; j++)
    cmd->filepath[j] = buf[i+j];

  return cmd;
}
