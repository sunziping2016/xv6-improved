// System call numbers
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21

#define SYS_thread_create 22
#define SYS_thread_exit   23
#define SYS_thread_wait   24

#define SYS_lock_create  31
#define SYS_lock_acquire 32
#define SYS_lock_release 33
#define SYS_lock_holding 34
#define SYS_lock_free    35

#define SYS_semaphore_create      41
#define SYS_semaphore_acquire     42
#define SYS_semaphore_release     43
#define SYS_semaphore_getcounter  44
#define SYS_semaphore_free        45

#define SYS_rwlock_create  		51
#define SYS_rwlock_acquire_read 	52
#define SYS_rwlock_acquire_write 	53
#define SYS_rwlock_release_read 	54
#define SYS_rwlock_release_write 	55
#define SYS_rwlock_holding_read 	56
#define SYS_rwlock_holding_write 	57
#define SYS_rwlock_free    		58
