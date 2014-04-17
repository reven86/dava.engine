//
//  MemoryRoutines.h
//  Framework
//
//  Created by Vladimir Bondarenko on 2/26/14.
//
//


#ifndef __Framework__MemoryRoutines__
#define __Framework__MemoryRoutines__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include <pthread.h>
#if defined(__USE_OWN_ALLOCATORS__)
namespace DAVA {

class AllocatorBase
{
public:
    virtual void* New(size_t size) = 0;
    virtual void Delete(void *ptr) = 0;
};
    
class AllocatorsStack : public StaticSingleton<AllocatorsStack>
{
public:
    AllocatorsStack();
    void PushAllocator(AllocatorBase *allocator);
    void PopAllocator();
    
    void * Allocate(size_t _size);
    bool Deallocate(void* ptr);
    
private:
    Map<pthread_t, Stack<AllocatorBase*>*>allocatorsStack;
};

}
#endif //__USE_OWN_ALLOCATORS__
#endif /* defined(__Framework__MemoryRoutines__) */

