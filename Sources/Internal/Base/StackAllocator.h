//
//  StackAllocator.h
//  Framework
//
//  Created by Vladimir Bondarenko on 2/26/14.
//
//

#ifndef __Framework__StackAllocator__
#define __Framework__StackAllocator__

#include "Base/MemoryRoutines.h"

#if defined(__USE_OWN_ALLOCATORS__)

namespace DAVA {

struct StackEntry
{
    uint8* data;
    uint32 size;
};
    
class StackAllocator : public AllocatorBase
{
public:
    StackAllocator(uint32 size);
    ~StackAllocator();
    
    virtual void* New(size_t size);
    virtual void Delete(void *ptr);
    inline uint32 GetCurrPos(){ return currPos;}
    inline void SetCurrPos(uint32 newPos){ currPos = newPos;}
    
private:
    
    uint32 bufferSize;
    uint32 currPos;
    uint32 maxSize;
    uint8 *buffer;
};
}

#endif //__USE_OWN_ALLOCATORS__

#endif /* defined(__Framework__StackAllocator__) */
