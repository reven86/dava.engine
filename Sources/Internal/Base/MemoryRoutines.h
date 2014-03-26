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
#include "Base/Singleton.h"
#include <pthread.h>
#if defined(__USE_OWN_ALLOCATORS__)
namespace DAVA {

class Allocator
{
public:
    virtual void* New(size_t size) = 0;
    virtual void Delete(void *ptr) = 0;
};
    
class AllocatorsStack : public Singleton<AllocatorsStack>
{
public:
    AllocatorsStack();
    void PushAllocator(Allocator *allocator);
    void PopAllocator();
    
    void * Allocate(size_t _size);
    bool Deallocate(void* ptr);
    inline void SetInitialized(bool bInitialized) {this->bInitialized = bInitialized;};
    
private:
    Map<pthread_t, Stack<Allocator*>*>allocatorsStack;
    static bool bInitialized;
};

}
#endif //__USE_OWN_ALLOCATORS__
#endif /* defined(__Framework__MemoryRoutines__) */

