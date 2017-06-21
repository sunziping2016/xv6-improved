/** @file
 * @brief Memory pool definitions
 * @author Mengyang Lv
 * @date June 22, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Xv6 kernel
#include "xv6/types.h"
#include "xv6/spinlock.h"

//[ Types ]
/**
 * Byte
 */
typedef char byte;

/**
 * Memory block type
 */
struct memblock
{
    // 0 for free, 1 for used
    byte status;
    byte *start;
    byte *end;
    struct memblock *nextblock;
    struct memblock *prevblock;
};

/**
 * Memory list type
 */
typedef struct memblock* memlist;

/**
 * Memory pool type
 */
struct mem_pool
{
    byte *mem;
    memlist list;
    uint size;
    struct spinlock lock;
};

//[ Functions ]
/**
 * Initialize memory pool
 *
 * @param pool Memory pool
 * @param m Memory
 * @param s Memory size
 */
void pool_init(struct mem_pool *pool, byte *m, uint s);

/**
 * Allocate memory from pool
 *
 * @param pool Memory pool
 * @param size Size of memory to be allocated
 */
void* pool_alloc(struct mem_pool *pool, int size);

/**
 * Release allocated memory
 *
 * @param pool Memory pool
 * @param p Reference to allocated memory
 * @return 0 for success and 1 for error
 */
int pool_free(struct mem_pool *pool, void *p);
