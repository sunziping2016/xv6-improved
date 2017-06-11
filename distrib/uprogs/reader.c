#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fs.h"
#include "xv6/memlayout.h"

#define LINE_BUFFER_LENGTH 81
#define BUFFER_LENGTH 1024

struct linebuffer
{
    char* buf;
    int  size;
    struct linebuffer *prev;
    struct linebuffer *next;
} linebuffer_head, linebuffer_tail;


struct linebuffer *cur_lbp;
struct linebuffer *upper_lbp;
int remain_space = 0;

void link_linebuffer(struct linebuffer *l, struct linebuffer *r){
    l->next = r;
    r->prev = l;
}
void alloc_linebuffer(struct linebuffer *lb){
    lb->buf  = malloc(LINE_BUFFER_LENGTH);
    memset(lb->buf, 0, LINE_BUFFER_LENGTH);
    lb->size = 0;
    lb->prev = 0;
    lb->next = 0;
}
struct linebuffer *create_linebuffer(){
    struct linebuffer *lbp;
    lbp = malloc(sizeof(struct linebuffer));
    alloc_linebuffer(lbp);
    return lbp;
}

void init()
{
    alloc_linebuffer(&linebuffer_head);
    alloc_linebuffer(&linebuffer_tail);
    link_linebuffer(&linebuffer_head, &linebuffer_tail);
    cur_lbp = &linebuffer_head;
    upper_lbp = &linebuffer_head;
}

void
read_char(char c)
{
    if(remain_space <= 0)
    {
        //printf(1, "%s\n", cur_lbp->buf);
        struct linebuffer *new_lbp;
        new_lbp = create_linebuffer();
        link_linebuffer(cur_lbp, new_lbp);
        link_linebuffer(new_lbp, &linebuffer_tail);
        //printf(1, "%d, %d\n", new_lbp->prev, new_lbp->next);
        cur_lbp = new_lbp;
        remain_space = 80;
    }
    if(c == '\n')
    {
        remain_space = 0;
        cur_lbp->buf[80] = '\n';

    }
    else
    {
        cur_lbp->buf[80 - remain_space] = c;
        remain_space --;
    }
}

void
read_file(int fd)
{
    char buf[BUFFER_LENGTH];
    int n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < n; i++)
        {
            read_char(buf[i]);
        }
    }
}

void
display(struct linebuffer* lbp)
{
    for(int line_num = 0; line_num < 23; line_num++)
    {
        if(lbp == &linebuffer_tail)
        {
            for(int i = 0; i < 80; i++)
                setcrtc(line_num * 80 + i, (' ' & 0xff) | 0x0700);
        }
        else
        {
            for(int i = 0; i < 80; i++)
            {
                if(lbp->buf[i] == 0)
                    setcrtc(line_num * 80 + i, (' ' & 0xff) | 0x0700);
                else
                    setcrtc(line_num * 80 + i, (lbp->buf[i] & 0xff) | 0x0700);
            }
            lbp = lbp->next;
        }

    }
}

void
clear()
{
    for(int i = 0; i < 80 * 24; i++)
        setcrtc(i, 0x0700 | ' ');
}

void cleanup()
{
    struct linebuffer *lbp, *lbptmp;
    lbp = (&linebuffer_tail)->prev;
    while(lbp != &linebuffer_head)
    {
        lbptmp = lbp;
        lbp    = lbp->prev;
        free(lbptmp->buf);
        free(lbptmp);
    }
    free(linebuffer_head.buf);
    free(linebuffer_tail.buf);
}

void
reader(int fd)
{
    //int old_pos = getcurpos();
    char a[2];
    a[0] = 0;
    init();
    read_file(fd);
    upper_lbp = linebuffer_head.next;
    do
    {
        setcurpos(23 * 80);
        if(a[0] == 's' && upper_lbp->next != &linebuffer_tail)
            upper_lbp = upper_lbp->next;
        else if(a[0] == 'w' && upper_lbp->prev != &linebuffer_head)
            upper_lbp = upper_lbp->prev;
        display(upper_lbp);
        read(0, a, 2);
    }while(a[0] != 'q');
    clear();
    cleanup();
    setcurpos(0);
}

int
main(int argc, char *argv[])
{
    int fd;

    if (argc <= 1) {
        printf(1, "reader: no file to read\n");
        exit();
    }

    else if (argc > 2){
        printf(1, "reader: one file a time, please\n");
        exit();
    }

    else
    {
        if ((fd = open(argv[1], 0)) < 0) {
            printf(1, "cat: cannot open %s\n", argv[1]);
            exit();
        }
        reader(fd);
        close(fd);

    }
    char a[10];
    read(0, a, 2);
    exit();

}