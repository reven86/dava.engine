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


#include "Base/FixedSizePoolAllocator.h"
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <stdlib.h>
#endif

#include <cstdlib>

namespace DAVA 
{
FixedSizePoolAllocator::FixedSizePoolAllocator(uint32 _blockSize, uint32 _blockArraySize)
{
    DVASSERT(_blockSize >= sizeof(uint8*));
    
    blockSize = _blockSize;
    blockArraySize = _blockArraySize;
    allocatedBlockArrays = 0;
    nextFreeBlock = 0;
#ifdef __DAVAENGINE_DEBUG__
	totalBlockCount = 0;
	freeItemCount = 0;
	maxItemCount = 0;
#endif
    CreateNewDataBlock();
}

void FixedSizePoolAllocator::CreateNewDataBlock()
{
    DVASSERT(blockSize >= sizeof(uint8*));
    void * block = ::malloc(blockArraySize * blockSize + sizeof(uint8*));
    //Logger::FrameworkDebug("Allocated new data block: %p pointer size: %d", block, sizeof(uint8*));
    // insert to list
    *(uint8**)block = (uint8*)allocatedBlockArrays;
    allocatedBlockArrays = block;
#ifdef __DAVAENGINE_DEBUG__
	totalBlockCount++;
	freeItemCount += blockArraySize;
#endif 
    
    InsertBlockToFreeNodes(block);
}

void FixedSizePoolAllocator::InsertBlockToFreeNodes(void * block)
{
    uint8 * blockItem = (uint8*)block + sizeof(uint8*) + blockSize * (blockArraySize - 1);
    for (uint32 k = 0; k < blockArraySize; ++k)
    {
        //Logger::FrameworkDebug("Free block added: %p", blockItem);

        *(uint8**)blockItem = (uint8*)nextFreeBlock;
        nextFreeBlock = blockItem;
        blockItem -= blockSize;
    }    
}

void FixedSizePoolAllocator::Reset()
{
    nextFreeBlock = 0;
    void * currentBlock = allocatedBlockArrays;
    while(currentBlock)
    {
        uint8 * next = *(uint8**)currentBlock;
        InsertBlockToFreeNodes(currentBlock);  
        currentBlock = next;
    }
}

void FixedSizePoolAllocator::DeallocateMemory()
{
#ifdef __DAVAENGINE_DEBUG__
//	DVASSERT(freeItemCount == (totalBlockCount*blockArraySize));
#endif

    while(allocatedBlockArrays)
    {
        uint8 * next = *(uint8**)allocatedBlockArrays;
        ::free(allocatedBlockArrays);
#ifdef __DAVAENGINE_DEBUG__
		totalBlockCount--;
#endif
        //Logger::FrameworkDebug("Deallocated data block: %p pointer size: %d", allocatedBlockArrays, sizeof(uint8*));
        allocatedBlockArrays = next;
    }

#ifdef __DAVAENGINE_DEBUG__
	DVASSERT(0 == totalBlockCount);
	freeItemCount = 0;
#endif
}

FixedSizePoolAllocator::~FixedSizePoolAllocator()
{
    DeallocateMemory();
}

void * FixedSizePoolAllocator::New()
{
    void * object = 0;
    if (nextFreeBlock == 0)
	{
        CreateNewDataBlock();
	}
       
#ifdef __DAVAENGINE_DEBUG__
	freeItemCount--;
	maxItemCount = Max(maxItemCount, (blockArraySize * blockSize)-freeItemCount);
#endif
    object = nextFreeBlock;
    nextFreeBlock = *(uint8**)nextFreeBlock;
    return object;
}

void FixedSizePoolAllocator::Delete(void * block)
{
	*(uint8**)block = (uint8*)nextFreeBlock;
	nextFreeBlock = block;

#ifdef __DAVAENGINE_DEBUG__
	freeItemCount++;
#endif
}
    
bool FixedSizePoolAllocator::CheckIsPointerValid(void * blockvoid)
{
    uint8 * block = (uint8*)blockvoid;
    uint8 * currentAllocatedBlockArray = (uint8*)allocatedBlockArrays; 
    while(currentAllocatedBlockArray)
    {
        uint8 * next = *(uint8**)currentAllocatedBlockArray;
        
        if ((block >= (uint8*)currentAllocatedBlockArray + sizeof(uint8*)) && (block < (uint8*)currentAllocatedBlockArray + sizeof(uint8*) + blockSize * blockArraySize))
        {
            // we are inside check is block correct.
            uint32 shift = static_cast<uint32>(block - ((uint8*)currentAllocatedBlockArray + sizeof(uint8*)));
            uint32 mod = shift % blockSize;
            if (mod == 0)
            {
                return true;
            }
            else 
            {
                return false;
            }
        }
        currentAllocatedBlockArray = next;
    }
    return false;
}

	
}
