//
//  StackAllocator.cpp
//  Framework
//
//  Created by Vladimir Bondarenko on 2/26/14.
//
//

#include "StackAllocator.h"
#include "Debug/DVAssert.h"

#if defined(__USE_OWN_ALLOCATORS__)
namespace DAVA
{

StackAllocator::StackAllocator(uint32 size)
    :bufferSize(size)
    ,currPos(0),
    maxSize(0)
{
    buffer = (uint8*)malloc(bufferSize);
}

StackAllocator::~StackAllocator()
{
    free(buffer);
}
    
void* StackAllocator::New(size_t size)
{
    if ((size & 15) != 0)
    {
        size += 16 - (size & 15);
    }
    DVASSERT(currPos + size < bufferSize);
    void *p = buffer + currPos;
    currPos += size;
    maxSize = Max(maxSize, currPos);
    return p;
}

void StackAllocator::Delete(void *ptr)
{

}
    
}

#endif
