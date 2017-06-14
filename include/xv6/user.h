struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

// ulib.c
int stat(char*, struct stat*);
void printf(int, char*, ...);
char* gets(char*, int max);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

// string.h
uint strlen(const char *str);
char * strchr(register const char *p, register const char ch);
char *strrchr(register const char *p, register const char *ch);
char *strcat(register char *s, register const char *append);
char *strncat(char *dst, const char *src, register uint n);
int strcmp(register const char *s1, register const char *s2);
int strncmp(register const char *s1, register const char *s2, register uint n);
char* strcpy(register char *to, register const char *from);
char *strncpy(char *dst, const char *src, register uint n);
char *strstr(register const char *s, register const char *find);
void *memcpy(void *dst0, const void *src0, register uint length);
void *memccpy(void *t, const void *f, int c, register uint n);
void *memchr(const void *s, register unsigned char c, register uint n);
int memcmp(const void *s1, const void *s2, uint n);

#define NULL (0)
#define EOF  (-1)

// stdio.h
typedef struct {
    int fd;
} FILE;

extern FILE _std_files[];

#define stdin (&_std_files[0])
#define stdout (&_std_files[1])

extern int myprintf(char const *fmt, ...);

// stdarg.h
typedef char *va_list;
#define	va_arg(ap, type) ((type *)(ap += sizeof(type)))[-1]
#define	va_end(ap)
#define	__va_promote(type) (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define	va_start(ap, last) (ap = ((char *)&(last) + __va_promote(last)))

#define	HUGE_VAL	1.701411834604692294E+38