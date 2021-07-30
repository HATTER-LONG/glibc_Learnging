#pragma once
void allocFastBinMem();
void freeFastBinMem(bool needJumpLastChunk = false);
void fastBinAllocFreeTest();