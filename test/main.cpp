
#include "ThreadPool.h"
#include "fastbin.h"
#include "tcache.h"
#include "assert.h"

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
    ThreadPool thread(1);
    thread.enqueue(allocThreadMem);
}
