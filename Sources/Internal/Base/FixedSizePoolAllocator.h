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


#ifndef __DAVAENGINE_FIXED_SIZE_POOL_ALLOCATOR_H__
#define __DAVAENGINE_FIXED_SIZE_POOL_ALLOCATOR_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
/**
    \brief Fast pool based allocator almost without memory overhead. Can be used to allocate small objects.
    
    Example of usage: 
 */
    
//class MallocAllocator
//{
//public:
//    Allocator()
//    {
//    }
//    
//    ~Allocator()
//    {
//    }
//    
//    ITEM * Alloc()
//    {
//        void * p = ::malloc(sizeof(ITEM));
//        //ITEM * item = new (p) ITEM;
//        return (ITEM*)p;
//    }
//    
//    void Dealloc(ITEM * node)
//    {
//        //node->~ITEM();
//        ::free(node);
//    }
//};
    
    
class FixedSizePoolAllocator
{
public:
	FixedSizePoolAllocator(uint32 _blockSize, uint32 _blockArraySize);
	~FixedSizePoolAllocator();
	
    void * New();
	void Delete(void * _item);
    void Reset();

    bool CheckIsPointerValid(void * block);
    void InsertBlockToFreeNodes(void * block);
	void CreateNewDataBlock();
	void DeallocateMemory();
	uint32 blockSize;
    uint32 blockArraySize;
    void * allocatedBlockArrays;    // ptr to last allocated block
    void * nextFreeBlock;      // next free 

	//debug stats
	uint32 totalBlockCount;
	uint32 freeItemCount;
	uint32 maxItemCount;
};
    
		
};

#endif // __DAVAENGINE_FIXED_SIZE_POOL_ALLOCATOR_H__