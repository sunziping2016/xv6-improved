#ifndef STDIO_H
#define STDIO_H

#ifndef EOF
#define EOF  (-1)
#endif

typedef struct {
    int fd;
} FILE;

extern FILE _std_files[];
#define stdin (&_std_files[0])
#define stdout (&_std_files[1])
#define stderr (&_std_files[2])

int myprintf(char const *fmt, ...);

#endif