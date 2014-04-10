//
//  MemoryRoutines.cpp
//  Framework
//
//  Created by Vladimir Bondarenko on 2/26/14.
//
//

#include "Base/MemoryRoutines.h"

#if defined(__USE_OWN_ALLOCATORS__)
/*void * operator new(size_t _size) throw(std::bad_alloc )
{
    void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = malloc(_size);
        if (p == NULL) {
            throw std::bad_alloc();
        }
    }
    return p;
}

void * operator new(size_t _size, const std::nothrow_t &) throw()
{
    void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = malloc(_size);
    }
    return p;
}

void   operator delete(void * _ptr) throw()
{
	bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        free(_ptr);
    }
}

void   operator delete(void * _ptr, const std::nothrow_t &) throw()
{
	bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        free(_ptr);
    }
}


void * operator new[](size_t _size) throw(std::bad_alloc)
{
	void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = malloc(_size);
        if (p == NULL) {
            throw std::bad_alloc();
        }
    }
    return p;
}

void * operator new[](size_t _size, const std::nothrow_t &) throw()
{
	void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = malloc(_size);
    }
    return p;
}

void   operator delete[](void * _ptr) throw()
{
	bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        free(_ptr);
    }
}

void   operator delete[](void * _ptr, const std::nothrow_t &) throw()
{
	bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        free(_ptr);
    }
}
*/
namespace DAVA {

AllocatorsStack::AllocatorsStack()
{
}
    
void *AllocatorsStack::Allocate(size_t _size)
{
    if(allocatorsStack.size() > 0)
    {
        pthread_t threadID = pthread_self();
        Map<pthread_t, Stack<AllocatorBase*>*>::iterator it = allocatorsStack.find(threadID);
        if(it != allocatorsStack.end())
        {
            Stack<AllocatorBase*>* stack = it->second;
            if(stack && stack->size() > 0 )
            {
                AllocatorBase *allocator = stack->top();
                return allocator->New(_size);
            }
        }
    }
    return NULL;
}
    
bool AllocatorsStack::Deallocate(void* ptr)
{
    if(allocatorsStack.size() > 0)
    {
        pthread_t threadID = pthread_self();
        Map<pthread_t, Stack<AllocatorBase*>*>::iterator it = allocatorsStack.find(threadID);
        if(it != allocatorsStack.end())
        {
            Stack<AllocatorBase*>* stack = it->second;
            if( stack && stack->size() > 0 )
            {
                AllocatorBase *allocator = stack->top();
                allocator->Delete(ptr);
                return true;
            }
        }
    }
    return false;
}

void AllocatorsStack::PushAllocator(AllocatorBase *allocator)
{
    pthread_t threadID = pthread_self();
    Stack<AllocatorBase*>* stack = NULL;
    Map<pthread_t, Stack<AllocatorBase*>*>::iterator it = allocatorsStack.find(threadID);
    if(it == allocatorsStack.end())
    {
        stack = new Stack<AllocatorBase*>();
        allocatorsStack[threadID] = stack;
    }
    else
    {
        stack = it->second;
    }
    if(stack)
    {
        stack->push(allocator);
    }
}

void AllocatorsStack::PopAllocator()
{
    pthread_t threadID = pthread_self();
    Map<pthread_t, Stack<AllocatorBase*>*>::iterator it = allocatorsStack.find(threadID);
    if(it != allocatorsStack.end())
    {
        Stack<AllocatorBase*>* stack = it->second;
        stack->pop();
    }
}
    
}
#endif