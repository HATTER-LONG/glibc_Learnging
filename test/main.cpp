
#include "fastbin.h"
#include "tcache.h"

#include <cstdio>
#include <cstdlib>


int main(void)
{
    tcacheAllocFreeTest();
    fastBinAllocFreeTest();
}
