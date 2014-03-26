//
//  MemoryRoutines.cpp
//  Framework
//
//  Created by Vladimir Bondarenko on 2/26/14.
//
//

#include "Base/MemoryRoutines.h"

#if defined(__USE_OWN_ALLOCATORS__)
bool DAVA::AllocatorsStack::bInitialized = false;
void * operator new(size_t _size) throw(std::bad_alloc )
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

namespace DAVA {

AllocatorsStack::AllocatorsStack()
{
}
    
void *AllocatorsStack::Allocate(size_t _size)
{
    if(bInitialized && allocatorsStack.size() > 0)
    {
        pthread_t threadID = pthread_self();
        Map<pthread_t, Stack<Allocator*>*>::iterator it = allocatorsStack.find(threadID);
        if(it != allocatorsStack.end())
        {
            Stack<Allocator*>* stack = it->second;
            if(stack && stack->size() > 0 )
            {
                Allocator *allocator = stack->top();
                return allocator->New(_size);
            }
        }
    }
    return NULL;
}
    
bool AllocatorsStack::Deallocate(void* ptr)
{
    if(bInitialized && allocatorsStack.size() > 0)
    {
        pthread_t threadID = pthread_self();
        Map<pthread_t, Stack<Allocator*>*>::iterator it = allocatorsStack.find(threadID);
        if(it != allocatorsStack.end())
        {
            Stack<Allocator*>* stack = it->second;
            if( stack && stack->size() > 0 )
            {
                Allocator *allocator = stack->top();
                allocator->Delete(ptr);
                return true;
            }
        }
    }
    return false;
}

void AllocatorsStack::PushAllocator(Allocator *allocator)
{
    if(bInitialized)
    {
        pthread_t threadID = pthread_self();
        Stack<Allocator*>* stack = NULL;
        Map<pthread_t, Stack<Allocator*>*>::iterator it = allocatorsStack.find(threadID);
        if(it == allocatorsStack.end())
        {
            stack = new Stack<Allocator*>();
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
}

void AllocatorsStack::PopAllocator()
{
    if(bInitialized)
    {
        pthread_t threadID = pthread_self();
        Map<pthread_t, Stack<Allocator*>*>::iterator it = allocatorsStack.find(threadID);
        if(it != allocatorsStack.end())
        {
            Stack<Allocator*>* stack = it->second;
            stack->pop();
        }
    }
}
    
}
#endif