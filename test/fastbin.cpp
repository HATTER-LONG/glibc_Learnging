#include "fastbin.h"

#include "allocMacro.h"
#include "tcache.h"

#include <cstdlib>

const size_t fastChunkCommonDiff = MALLOC_ALIGNMENT;
const size_t fastBinsCount = NFASTBINS;
const size_t maxBinListLength = 16;

void* fastPtrArray[fastBinsCount][maxBinListLength] = {
    {
        nullptr,
    },
};

void allocFastBinMem()
{
    freeFastBinMem();
    for (size_t i = 0; i < fastBinsCount; i++)
    {
        auto allocSize = fastChunkCommonDiff * (i + 1);
        for (size_t j = 0; j < maxBinListLength; j++)
        {
            fastPtrArray[i][j] = malloc(allocSize);
            printf("malloc address is %p\n", fastPtrArray[i][j]);
        }
    }
}

void freeFastBinMem(bool needJumpLastChunk)
{
    for (size_t i = 0; i < fastBinsCount; i++)
    {
        for (size_t j = 0; j < maxBinListLength; j++)
        {
            if (fastPtrArray[i][j] != nullptr)
            {
                if (needJumpLastChunk && !((i == fastBinsCount - 1) && (j == maxBinListLength - 1)))
                    // 跳过最后一个申请的内存避免将 所有 chunk 合并到 top chunk 中
                    continue;
                free(fastPtrArray[i][j]);
                fastPtrArray[i][j] = nullptr;
            }
        }
    }
}

void fastBinAllocFreeTest()
{
    allocTcacheMem();
    allocFastBinMem();

    freeTcacheMem();
    freeFastBinMem(true);
}
