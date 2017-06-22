// Node of process list
struct procnode {
  struct proc *proc;
  int next;       // index of next node
};

// Process list
struct proclist {
  int head, tail; // index of head & tail nodes
                  // -1 is null node
};

// Semaphore
struct semaphore {
  int count;             // Counter.
  struct proclist queue; // Waiting queue.
  struct spinlock lk;    // Protect this semaphore.
  
  // For debugging:
  char *name;            // Name of lock.
};
