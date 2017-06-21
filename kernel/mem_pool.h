#pragma once

#include "xv6/types.h"
#include "xv6/spinlock.h"

typedef char byte;

struct memblock
{
    byte status;
    byte *start;
    byte *end;
    struct memblock *nextblock;
    struct memblock *prevblock;
};

typedef struct memblock* memlist;

struct mem_pool
{
    byte *mem;
    memlist list;
    uint size;
    struct spinlock lock;
};

void pool_init(struct mem_pool *pool, byte *m, uint s);

void* pool_alloc(struct mem_pool *pool, int size);

int pool_free(struct mem_pool *pool, void *p);
