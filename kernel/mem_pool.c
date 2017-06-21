#include "mem_pool.h"
#include "edefs.h"
#include "xv6/defs.h"

#define BLOCK_POINTER_NUMBER 4
#define BLOCK_BYTE_NUMBER 1
#define SIZE_OF_BLOCK (sizeof(struct memblock))

void pool_init(struct mem_pool *pool, byte *m, uint s)
{
    acquire(&pool->lock);

    pool->mem = m;
    pool->size = s;

    memlist firstfree = (memlist)m;

    firstfree->start = m + SIZE_OF_BLOCK;
    firstfree->end = m + s;
    firstfree->nextblock = 0;
    firstfree->prevblock = 0;
    firstfree->status = 0;
    pool->list = firstfree;

    release(&pool->lock);
}

void* pool_alloc(struct mem_pool *pool, int size)
{
    acquire(&pool->lock);

    struct memblock *block = pool->list;
    void* allocmem = 0;
    while (block != 0) {
        if (block->status == 0) {
            if (block->end - block->start > size) {
                if((block->end - block->start - size) > (2 * SIZE_OF_BLOCK)) {
                    block->status = 1;
                    struct memblock *newblock = (struct memblock*)(block->start + size);
                    newblock->status = 0;
                    newblock->start = block->start + size + SIZE_OF_BLOCK;
                    newblock->end = block->end;
                    block->end = block->start + size;

                    if (block->nextblock != 0) {
                        newblock->prevblock = block;
                        newblock->nextblock = block->nextblock;
                        block->nextblock->prevblock = newblock;
                        block->nextblock = newblock;
                    } else {
                        newblock->prevblock = block;
                        newblock->nextblock = 0;
                        block->nextblock = newblock;
                    }


                    allocmem = block->start;
                    break;
                } else {
                    block->status = 1;
                    allocmem = block->start;
                    break;
                }
            }
        }
        block = block->nextblock;
    }

    release(&pool->lock);
    return allocmem;
}

int pool_free(struct mem_pool *pool, void *p)
{
    acquire(&pool->lock);

    if (p >= pool->mem + SIZE_OF_BLOCK && p < (pool->mem + pool->size)) {
        struct memblock *block = (struct memblock*)(p - SIZE_OF_BLOCK);
        if (block->start == p && block->status == 1) {
            block->status = 0;
            if (block->nextblock != 0 && block->nextblock->status == 0) {
                if (block->nextblock->nextblock == 0) {
                    block->end = block->nextblock->end;
                    block->nextblock = 0;
                } else {
                    block->end = block->nextblock->end;
                    block->nextblock->nextblock->prevblock = block;
                    block->nextblock = block->nextblock->nextblock;
                }
            }
            if (block->prevblock != 0 && block->prevblock->status == 0) {
                if (block->nextblock == 0) {
                    block->prevblock->end = block->end;
                    block->prevblock->nextblock = 0;
                } else {
                    block->prevblock->end = block->end;
                    block->nextblock->prevblock = block->prevblock;
                    block->prevblock->nextblock = block->nextblock;
                }
            }

            release(&pool->lock);
            return 0;
        }
    }

    release(&pool->lock);
    return 1;
}
