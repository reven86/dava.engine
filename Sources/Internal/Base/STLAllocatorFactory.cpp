//
//  STLAllocatorFactory.cpp
//  Framework
//
//  Created by Sergey Bogdanov on 3/11/14.
//
//

#include "STLAllocatorFactory.h"


namespace DAVA
{
    
    STLAllocatorFactory::STLAllocatorFactory():LastAlloc(NULL)
    {
        
    }
    
    STLAllocatorFactory::~STLAllocatorFactory()
    {
#ifdef __DAVAENGINE_DEBUG__
        Dump();
#endif
        std::map<unsigned int,FixedSizePoolAllocator*>::iterator itEnd = allocators.end();
        for (std::map<unsigned int,FixedSizePoolAllocator*>::iterator it = allocators.begin(); it != itEnd; ++it)
        {
            delete((*it).second);
        }
        allocators.clear();
    }
    
    void STLAllocatorFactory::Dump()
    {
//#ifdef __DAVAENGINE_DEBUG__
        printf("\nSTLAllocatorFactory::Dump (Max item count) ================");
        std::map<unsigned int, FixedSizePoolAllocator*>::iterator itEnd = allocators.end();
        uint32 allover = 0;
        for(std::map<unsigned int, FixedSizePoolAllocator*>::iterator it = allocators.begin(); it != itEnd; ++it)
        {
            FixedSizePoolAllocator * alloc = (*it).second;
            printf("\n  maxItemCount=%u blockSize=%u blockArraySize=%u over=%u freeItemCount=%u", alloc->maxItemCount, alloc->blockSize, alloc->blockArraySize, alloc->freeItemCount*alloc->blockSize, alloc->freeItemCount);
            allover+=alloc->freeItemCount*alloc->blockSize;
        }
       printf("\nEnd of STLAllocatorFactory::Dump ========================== allover= %ubyte",allover);
//#endif //__DAVAENGINE_DEBUG__
    }
    
    DAVA::FixedSizePoolAllocator * STLAllocatorFactory::GetAllocator(unsigned int classSize)
    {
        if (LastAlloc)
        {
            if (LastAlloc->blockSize == classSize)
            {
                return LastAlloc;
            }
        }
        
        DAVA::FixedSizePoolAllocator * alloc = allocators[classSize];
        if ( 0 == alloc )
        {
            alloc = new FixedSizePoolAllocator(classSize, 128);
            allocators[classSize] = alloc;
            LastAlloc = alloc;
        }
        return alloc;
    }
 
}