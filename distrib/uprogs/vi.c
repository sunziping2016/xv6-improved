/*
 This is a tiny vim,
 In NORMAL mode, you can move cursor using direction buttons,
 press Ctrl + I to enter INSERT mode, or press q to quit.
 when the cursor reaches the top or the bottom of the screen,
 it will scroll down or up if there are lines beyond the screen.
 In INSERT mode, instead of moving cursor, and insert text, and delete
 character within a line.

 ZHU Hongyu
*/
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

void*
charmove(void *vdst, void *vsrc, int n)
{
    char *dst, *src;

    dst = vdst;
    src = vsrc;
    if(vdst <= vsrc)
    {
        dst = vdst;
        src = vsrc;
        while (n-- > 0)
            *dst++ = *src++;
    }
    else
    {
        dst = vdst + n - 1;
        src = vsrc + n - 1;
        while (n-- > 0)
            *dst-- = *src--;
    }
    return vdst;
}

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
    if(80 - cur_lbp->size <= 0 || cur_lbp->buf[80] == '\n')
    {
        //printf(1, "%s\n", cur_lbp->buf);
        struct linebuffer *new_lbp;
        new_lbp = create_linebuffer();
        link_linebuffer(cur_lbp, new_lbp);
        link_linebuffer(new_lbp, &linebuffer_tail);
        //printf(1, "%d, %d\n", new_lbp->prev, new_lbp->next);
        cur_lbp = new_lbp;

    }
    if(c == '\n')
    {
        cur_lbp->buf[80] = '\n';
    }
    else
    {
        cur_lbp->buf[cur_lbp->size] = c;
        cur_lbp->size++;
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
    if(linebuffer_tail.prev != &linebuffer_head)
        linebuffer_tail.prev->buf[80] = '\n';
}

void
insert(char c, int pos)
{
    int m_pos = pos;
    char m_tmp;
    struct linebuffer * m_lbp = upper_lbp;
    while(m_pos >= 80)
    {
        if(m_lbp != &linebuffer_tail)
        {
            m_pos -= 80;
            m_lbp = m_lbp->next;
        }
    }

    if(((m_lbp->size == 79 && m_pos == 79) || m_lbp->size == 80) && m_lbp->buf[80] == '\n')
    {
        struct linebuffer *new_lbp;
        new_lbp = create_linebuffer();
        link_linebuffer(new_lbp, m_lbp->next);
        link_linebuffer(m_lbp, new_lbp);

        if(m_lbp->size == 80)
        {
            m_lbp->next->buf[0] = m_lbp->buf[79];
            m_lbp->next->size++;
        }
        else
        {
            m_lbp->size++;
        }

        m_lbp->buf[80] = 0;
        m_lbp->next->buf[80] = '\n';

        charmove(m_lbp->buf + m_pos + 1, m_lbp->buf + m_pos, 79 - m_pos);
        m_lbp->buf[m_pos] = c;

    }
    else if(m_lbp->size < 80 && m_lbp->buf[80] == '\n')
    {
        charmove(m_lbp->buf + m_pos + 1, m_lbp->buf + m_pos, 79 - m_pos);
        m_lbp->buf[m_pos] = c;
        m_lbp->size++;
    }
    else
    {
        char tmpchar = m_lbp->buf[79];
        charmove(m_lbp->buf + m_pos + 1, m_lbp->buf + m_pos, 79 - m_pos);
        m_lbp->buf[m_pos] = c;
        insert(tmpchar, pos / 80 * 80 + 80);
    }

}

void
del(int pos)
{
    int m_pos = pos;
    char m_tmp;
    struct linebuffer * m_lbp = upper_lbp;
    while(m_pos >= 80)
    {
        if(m_lbp != &linebuffer_tail)
        {
            m_pos -= 80;
            m_lbp = m_lbp->next;
        }
    }
    if(m_pos > 0 && m_pos < m_lbp->size + 1)
    {
        m_lbp->buf[80] = '\n';
        charmove(m_lbp->buf + m_pos - 1, m_lbp->buf + m_pos, m_lbp->size-m_pos - 1);
        m_lbp->buf[m_lbp->size - 1] = 0;
        m_lbp->size--;

        setcurpos(pos - 1);

    }
    else if(m_pos == 0 && m_lbp->size == 0)
    {
        if(m_lbp->next != &linebuffer_head)
        {
            struct linebuffer *tmp = m_lbp;
            m_lbp = m_lbp->prev;
            link_linebuffer(m_lbp, tmp->next);
            free(tmp->buf);
            free(tmp);

            m_lbp->buf[80] = '\n';

            setcurpos(m_lbp->prev->size - 1);
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
                if(i == lbp->size)
                    setcrtc(line_num * 80 + i, (' ' & 0xff) | 0x0700);
                else
                    setcrtc(line_num * 80 + i, (lbp->buf[i] & 0xff) | 0x0700);
            }
            lbp = lbp->next;
        }

    }
}

void statusDisplay()
{
    char NORMAL_STATUS[] = "NORMAL:";
    char INSERT_STATUS[] = "INSERT:";
    int status = geteditstatus();
    if(status == -1)
        for(int i = 80 * 23 ; i < 80 * 23 + 7; i++)
        {
            setcrtc(i, (NORMAL_STATUS[i % 80] & 0xff) | 0x0700);
        }
    else
    {
        for(int i = 80 * 23 ; i < 80 * 23 + 7; i++)
        {
            setcrtc(i, (INSERT_STATUS[i % 80] & 0xff) | 0x0700);
        }
    }
}

void
clear()
{
    for(int i = 0; i < 80 * 24; i++)
        setcrtc(i, 0x0700 | ' ');
}

void
cleanup()
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
    init();
    seteditstatus(-1);
    read_file(fd);
    upper_lbp = linebuffer_head.next;
    struct linebuffer * m_lbp = upper_lbp;
    int m_editstatus;
    int quitFlag = 0;
    do
    {
        m_lbp = upper_lbp;
        statusDisplay();
        display(upper_lbp);
        m_editstatus = geteditstatus();
        if(m_editstatus >= 256)
        {
            int c = m_editstatus - 256;
            int pos = getcurpos();
            switch(c)
            {
            case '\b':
                del(pos);
                break;
            default:
                insert(c, pos);
                setcurpos(pos + 1);
            }
            seteditstatus(-2);
        }
        else
        {
            int pos = getcurpos();
            int m_pos = pos;
            switch (m_editstatus)
            {
                case -3:
                    quitFlag = 1;
                    break;
                case -4:
                    if(pos / 80 == 0 && upper_lbp->prev != & linebuffer_head)
                        upper_lbp = upper_lbp->prev;
                    else if (pos / 80 > 0)
                    {
                        pos = pos - 80;
                        setcurpos(pos);
                    }
                    seteditstatus(-1);
                    break;
                case -5:
                    if(pos / 80 == 22 && upper_lbp->next != & linebuffer_tail)
                        upper_lbp = upper_lbp->next;
                    else if (pos / 80 < 23)
                    {
                        pos = pos + 80;
                        setcurpos(pos);
                    }
                    seteditstatus(-1);
                    break;
                case -6:
                    if(pos % 80 > 0)
                    {
                        setcurpos(pos - 1);
                    }
                    seteditstatus(-1);
                    break;
                case -7:

                    while(m_pos >= 80)
                    {
                        if(m_lbp != &linebuffer_tail)
                        {
                            m_pos -= 80;
                            m_lbp = m_lbp->next;
                        }
                    }
                    if(pos % 80 < m_lbp->size && pos % 80 < 79)
                    {
                        setcurpos(pos + 1);
                    }
                    seteditstatus(-1);

                    break;

            }
        }
    }while(quitFlag == 0);
    clear();
    cleanup();
    setcurpos(0);
    seteditstatus(0);
}

int
main(int argc, char *argv[])
{
    int fd;

    if (argc <= 1) {
        printf(1, "vi: no file to read\n");
        exit();
    }

    else if (argc > 2){
        printf(1, "vi: one file a time, please\n");
        exit();
    }

    else
    {
        if ((fd = open(argv[1], 0)) < 0) {
            printf(1, "vi: cannot open %s\n", argv[1]);
            exit();
        }
        reader(fd);
        close(fd);

    }
    char a[10];
    read(0, a, 2);
    exit();

}