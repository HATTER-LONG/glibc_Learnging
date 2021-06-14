
#include "gnu/libc-version.h"
#include "spdlog/spdlog.h"

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif
struct malloc_chunk
{

    INTERNAL_SIZE_T mchunk_prev_size; /* Size of previous chunk (if free).  */
    INTERNAL_SIZE_T mchunk_size;      /* Size in bytes, including overhead. */

    struct malloc_chunk* fd; /* double links -- used only if free. */
    struct malloc_chunk* bk;

    /* Only used for large blocks: pointer to next larger size.  */
    struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
    struct malloc_chunk* bk_nextsize;
};

#define MIN_CHUNK_SIZE (offsetof(struct malloc_chunk, fd_nextsize))

void checkPoint() { }

int main(void)
{
    checkPoint();

    int* intPtr = static_cast<int*>(malloc(sizeof(int)));
    checkPoint();
    *intPtr = 20;
    checkPoint();
    free(intPtr);

    intPtr = static_cast<int*>(malloc(sizeof(int) * 30));
    free(intPtr);
    intPtr = static_cast<int*>(malloc(sizeof(int) * 1024));
    free(intPtr);
    intPtr = static_cast<int*>(malloc(sizeof(int) * 30));
    free(intPtr);
    spdlog::info("Ptr info is {}", fmt::ptr(intPtr));
    spdlog::info("Glibc Version is {}", gnu_get_libc_version());

    spdlog::info("MIN_CHUNK_SIZE = {}", MIN_CHUNK_SIZE);
}
