#ifndef STDIO_H
#define STDIO_H

#ifndef EOF
#define EOF  (-1)
#endif

#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
/* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	BUFSIZ	1024

typedef struct {
    int fd;
    int buffer_size;
    int buffer_end;
    int cur;
    int reserve;
    char *buffer;
    short flags;
} FILE;

extern FILE _std_files[];
#define stdin (&_std_files[0])
#define stdout (&_std_files[1])
#define stderr (&_std_files[2])
#define	__sgetc(p) __srget(p)
int	__srget (FILE *);
int __sputc (int _c, FILE *_p);



#define putc(x, fp)	__sputc(x, fp)
#define fputc(x, fp) __sputc(x, fp)

#define	getchar()	getc(stdin)

#define	putchar(x)	putc(x, stdout)

#define	getc(fp) __sgetc(fp)
#define fgetc(fp) __sgetc(fp)

int myprintf(char const *fmt, ...);
int fprintf(FILE *fp, const char *fmt, ...);
int puts(char const *s);
int fputs(const char *s, FILE *fp);
char *mygets(char *buf);
char *fgets(char *buf, int max, FILE *fp);
FILE *fopen(const char *file, const char *mode);
int fclose(FILE *fp);
int ungetc(int ch, FILE *stream);
int	 scanf(const char *, ...);
int fscanf(FILE *fp, char *fmt, ...);

double strtod(const char *s00, char **se);

#endif