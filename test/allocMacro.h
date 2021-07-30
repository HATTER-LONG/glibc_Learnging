#pragma once

#include <cstddef>
#include <cstdio>

// glibc_2.33

struct malloc_chunk
{
    size_t mchunk_prev_size; /* Size of previous chunk (if free).  */
    size_t mchunk_size;      /* Size in bytes, including overhead. */

    struct malloc_chunk* fd; /* double links -- used only if free. */
    struct malloc_chunk* bk;

    /* Only used for large blocks: pointer to next larger size.  */
    struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
    struct malloc_chunk* bk_nextsize;
};


#define SIZE_SZ sizeof(size_t)

#define MALLOC_ALIGNMENT (2 * SIZE_SZ < __alignof__(long double) ? __alignof__(long double) : 2 * SIZE_SZ)

#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)

#define MIN_CHUNK_SIZE (offsetof(struct malloc_chunk, fd_nextsize))

#define MINSIZE (unsigned long)(((MIN_CHUNK_SIZE + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

#define TCACHE_MAX_BINS 64

#define MAX_TCACHE_SIZE tidx2usize(TCACHE_MAX_BINS - 1)


/*将下标号转为对应 bin 大小*/
#define tidx2usize(idx) (((size_t)idx) * MALLOC_ALIGNMENT + MINSIZE - SIZE_SZ)

/*将想要申请的内存转化为 chunk size*/
#define request2size(req)                                                                                                        \
    (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE) ? MINSIZE : ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)
/*将 chunk size 转化位对应的 bins 下标， */
#define csize2tidx(x) (((x)-MINSIZE + MALLOC_ALIGNMENT - 1) / MALLOC_ALIGNMENT)
/*将想要申请的内存先转化位 chunk size 再求得对应 bins 下标*/
#define usize2tidx(x) csize2tidx(request2size(x))

/************************* fastbin ******************************/

/* offset 2 to use otherwise unindexable first 2 bins */
#define fastbin_index(sz) \
  ((((unsigned int) (sz)) >> (SIZE_SZ == 8 ? 4 : 3)) - 2)
/* The maximum fastbin request size we support */
#define MAX_FAST_SIZE     (80 * SIZE_SZ / 4)

#define NFASTBINS  (fastbin_index (request2size (MAX_FAST_SIZE)) + 1)
