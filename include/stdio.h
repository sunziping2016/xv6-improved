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
#define	__sgetc(p) __srget(p)
int	__srget (FILE *);
int __sputc (int _c, FILE *_p);

#define	getc(fp) __sgetc(fp)
#define fgetc(fp) __sgetc(fp)

#define putc(x, fp)	__sputc(x, fp)
#define fputc(x, fp) __sputc(x, fp)

#define	getchar()	getc(stdin)

#define	putchar(x)	putc(x, stdout)

int myprintf(char const *fmt, ...);
int fprintf(FILE *fp, const char *fmt, ...);
int puts(char const *s);
int fputs(const char *s, FILE *fp);
char *mygets(char *buf);
char *fgets(char *buf, int max, FILE *fp);
FILE *fopen(const char *file, const char *mode);
//int	 scanf(const char *, ...);

#endif