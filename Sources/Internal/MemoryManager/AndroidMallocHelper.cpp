//
//  AndroidMallocHelper.cpp
//  Framework
//
//  Created by Leonid Sinyakov on 2/20/15.
//
//

#include "AndroidMallocHelper.h"
#include <stddef.h>
//internal android memory structures taken from AOSP
struct malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               head;       /* Size and inuse bits. */
  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};
typedef  malloc_chunk* mchunkptr;
#define SIZE_T_ZERO         ((size_t)0)
#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZE_T_FOUR         ((size_t)4)
#define SIZE_T_SIZE         (sizeof(size_t))
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define PINUSE_BIT          (SIZE_T_ONE)
#define CINUSE_BIT          (SIZE_T_TWO)


#define FLAG4_BIT           (SIZE_T_FOUR)
#define FLAG_BITS           (PINUSE_BIT|CINUSE_BIT|FLAG4_BIT)
#define chunksize(p)        ((p)->head & ~(FLAG_BITS))
#define chunk_plus_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))
#define get_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot)
#define chunk2mem(p)        ((void*)((char*)(p)       + TWO_SIZE_T_SIZES))
#define mem2chunk(mem)      ((mchunkptr)((char*)(mem) - TWO_SIZE_T_SIZES))

size_t androidMallocSize(void * ptr)
{
    malloc_chunk*  mc = (malloc_chunk*)(((char*)ptr) - TWO_SIZE_T_SIZES);
    size_t sz = chunksize(mc);
    return sz;
}