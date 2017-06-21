/** @file
 * @brief Memory pool implementation
 * @author Mengyang Lv
 * @date June 22, 2016
 * @version 1.0.0
 */

//[ Header Files ]
//Xv6 kernel extra
#include "mem_pool.h"
#include "edefs.h"
//Xv6 kernel
#include "xv6/defs.h"

//[ Constants ]
//Memory pool parameters
#define BLOCK_POINTER_NUMBER 4
#define BLOCK_BYTE_NUMBER 1
#define SIZE_OF_BLOCK (sizeof(struct memblock))

//[ Functions ]
//Initialize memory pool
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

//Allocate memory from pool
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

//Release allocated memory
int pool_free(struct mem_pool *pool, void *p)
{
    acquire(&pool->lock);

    if (p >= pool->mem + SIZE_OF_BLOCK && p < (pool->mem + pool->size)) {
        struct memblock *block = (struct memblock*)(p - SIZE_OF_BLOCK);
        if (block->start == p && block->status == 1) {
            block->status = 0;
            // merge next block
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
            //merge prev block
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
