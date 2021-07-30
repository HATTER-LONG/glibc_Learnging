#include "allocMacro.h"

#include <cstdlib>
#include <cstring>

void printAllocInfo(size_t allocSize)
{
    printf("Alloc size is %lu\n", allocSize);

    auto csize = request2size(allocSize);
    printf("request2size(%lu) = %lu\n", allocSize, csize);

    auto idex = csize2tidx(csize);
    auto csizeT = (csize)-MINSIZE + MALLOC_ALIGNMENT - 1;
    printf("csize2tidx(%lu) = %lu, csizeT is %lu\n", allocSize, idex, csizeT);

    printf("usize2tidx(%lu) = %lu\n", allocSize, usize2tidx(allocSize));

    auto usize = tidx2usize(idex);
    printf("tidx2usize(%lu) = %lu\n", idex, usize);
}

const size_t tcacheCommonDiff = MALLOC_ALIGNMENT;
const size_t tcacheBinsCount = TCACHE_MAX_BINS;
const size_t maxBinListLength = 7;

void* ptrArray[tcacheBinsCount][maxBinListLength] = {
    {
        nullptr,
    },
};


void allocTcacheMem()
{
    for (size_t i = 0; i < tcacheBinsCount; i++)
    {
        auto allocSize = tcacheCommonDiff * (i + 1);
        printf("-------[%lu]--------\n", i);
        printAllocInfo(allocSize);
        for (size_t j = 0; j < maxBinListLength; j++)
        {
            ptrArray[i][j] = malloc(allocSize);
            printf("malloc address is %p\n", ptrArray[i][j]);
        }
    }
}

void freeTcacheMem()
{
    for (size_t i = 0; i < tcacheBinsCount; i++)
    {
        for (size_t j = 0; j < maxBinListLength; j++)
        {
            if (ptrArray[i][j] != nullptr)
            {
                free(ptrArray[i][j]);
            }
        }
    }
}

void tcacheAllocFreeTest()
{
    allocTcacheMem();
    // const char* longString2test = "1234567890123456789012345678901234567890";
    // strncpy(static_cast<char*>(ptrArray[0][3]), longString2test, strlen(longString2test));
    freeTcacheMem();
}
