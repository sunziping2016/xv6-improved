// Long-term locks for processes
struct rwlock {
  uint read_locked;       // Is the lock held in the read mode?
  uint write_locked;      // Is the lock held in the write mode?
  uint w_required;
  struct spinlock lk; // spinlock protecting this sleep lock
  
  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

