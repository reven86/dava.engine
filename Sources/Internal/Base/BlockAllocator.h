/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_BLOCKALLOCATOR_H__
#define __DAVAENGINE_BLOCKALLOCATOR_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
class BlockAllocatorBase
{
public:
    static char* AllocateTracked(uint32 size);
    static void FreeTracked(void* ptr);
    static bool ContainsPointer(void* ptr);
    static uint32 AllocationSizeForPointer(void* ptr);
    static void ReleaseTrackedMemory();
};

template <uint32 blockSize, uint32 numBlocks>
class BlockAllocator : public BlockAllocatorBase
{
public:
    BlockAllocator();
    inline char* Alloc(uint32 sz);
    inline void Free(void* ptr);
    inline bool ContainsAddress(void* ptr);

private:
    struct Block
    {
        uint32 allocated = 0;
        char data[blockSize];
    };

private:
    Block blocks[numBlocks];
    Block* currentBlock = nullptr;
    Block* lastBlock = nullptr;
    char* firstPossibleAddress = nullptr;
    char* lastPossibleAddress = nullptr;
    uint32 allocationsCount = 0;
    uint32 deallocationsCount = 0;
    int32 livignBlocks = 0;
};

template <uint32 blockSize, uint32 numBlocks>
BlockAllocator<blockSize, numBlocks>::BlockAllocator()
{
    lastBlock = blocks + numBlocks - 1;
    firstPossibleAddress = reinterpret_cast<char*>(blocks) + sizeof(int);
    lastPossibleAddress = reinterpret_cast<char*>(lastBlock) + sizeof(int);
    currentBlock = blocks;
    memset(blocks, 0, sizeof(blocks));
}

template <uint32 blockSize, uint32 numBlocks>
char* BlockAllocator<blockSize, numBlocks>::Alloc(uint32 sz)
{
    DVASSERT(sz <= blockSize);

    Block* startSearchBlock = currentBlock;
    while (currentBlock->allocated)
    {
        ++currentBlock;

        if (currentBlock == startSearchBlock)
        {
            // no free blocks available
            return AllocateTracked(sz);
        }

        if (currentBlock > lastBlock)
        {
            currentBlock = blocks;
        };
    }

    currentBlock->allocated = 1;
    char* result = currentBlock->data;

    Block* nextBlock = currentBlock + 1;
    currentBlock = (nextBlock > lastBlock) ? blocks : nextBlock;
    ++allocationsCount;
    ++livignBlocks;
    return result;
}

template <uint32 blockSize, uint32 numBlocks>
bool BlockAllocator<blockSize, numBlocks>::ContainsAddress(void* ptr)
{
    char* cptr = reinterpret_cast<char*>(ptr);
    return (cptr >= firstPossibleAddress) && (cptr <= lastPossibleAddress);
}

template <uint32 blockSize, uint32 numBlocks>
void BlockAllocator<blockSize, numBlocks>::Free(void* ptr)
{
    DVASSERT(ContainsAddress(ptr));
    char* cptr = reinterpret_cast<char*>(ptr);
    Block* block = reinterpret_cast<Block*>(cptr - sizeof(int));
    block->allocated = 0;
    ++deallocationsCount;
    --livignBlocks;
}
}

#endif
