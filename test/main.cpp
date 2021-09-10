
#include "ThreadPool.h"
#include "assert.h"
#include "fastbin.h"
#include "tcache.h"

#include <cstdio>
#include <cstdlib>

void allocThreadMem()
{
    tcacheAllocFreeTest();
    fastBinAllocFreeTest();
}


int main(void)
{
    tcacheAllocFreeTest();
    fastBinAllocFreeTest();
    ThreadPool thread(2);
    thread.enqueue(allocThreadMem);
    thread.enqueue(allocThreadMem);

    return 0;
}
