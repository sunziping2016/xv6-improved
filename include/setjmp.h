#ifndef SETJMP_H
#define SETJMP_H

typedef int jmp_buf[10];

int	setjmp(jmp_buf);
void longjmp(jmp_buf, int);

#endif