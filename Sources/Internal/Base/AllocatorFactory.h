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



#ifndef __DAVAENGINE_ALLOCATOR_FACTORY_H__
#define __DAVAENGINE_ALLOCATOR_FACTORY_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include "Base/FixedSizePoolAllocator.h"

#if defined(__USE_OWN_ALLOCATORS__)
#include "Base/StackAllocator.h"
#endif

#define IMPLEMENT_POOL_ALLOCATOR(TYPE, poolSize) \
	void * operator new(std::size_t size) \
	{ \
        DVASSERT(size == sizeof(TYPE)); /*probably you are allocating child class*/ \
		static FixedSizePoolAllocator * alloc = AllocatorFactory::Instance()->GetAllocator(typeid(TYPE).name(), sizeof(TYPE), poolSize); \
		return alloc->New(); \
	} \
	 \
	void operator delete(void * ptr) \
	{ \
		static FixedSizePoolAllocator * alloc = AllocatorFactory::Instance()->GetAllocator(typeid(TYPE).name(), sizeof(TYPE), poolSize); \
		alloc->Delete(ptr); \
	} \

#define IMPLEMENT_NATIVE_ALLOCATOR \
    void * operator new(std::size_t size) \
    { \
        return ::malloc(size); \
    } \
    \
    void operator delete(void * ptr) \
    {\
        ::free(ptr);\
    }\

#if defined(__USE_OWN_ALLOCATORS__)
#define AUTO_STACK_ALLOCATOR(pStackPos, bInit, bWipe) \
    AutoStackAllocator autoStack(pStackPos, bInit, bWipe); \

#endif

namespace DAVA
{

class AllocatorFactory : public StaticSingleton<AllocatorFactory>
{
public:
	AllocatorFactory();
	virtual ~AllocatorFactory();

	FixedSizePoolAllocator * GetAllocator(const String& className, uint32 classSize, int32 poolLength);
    
#if defined(__USE_OWN_ALLOCATORS__)
    StackAllocator *CreateStackAllocator(uint32 size);
    StackAllocator * GetAllocator();
    void FreeAllocator(StackAllocator *);
#endif

	void Dump();

private:
	Map<String, FixedSizePoolAllocator*> allocators;
#if defined(__USE_OWN_ALLOCATORS__)
    Map<pthread_t, StackAllocator*> stackAllocators;
#endif
};

#if defined(__USE_OWN_ALLOCATORS__)
class AutoStackAllocator
{
public:
    AutoStackAllocator(uint32 *pStackPos, bool bInitPos, bool bWipe)
    :bWipe(bWipe)
    ,bInitPos(bInitPos)
    ,wipeTo(0)
    {
        pAlloc = AllocatorFactory::Instance()->GetAllocator();
        if(pAlloc)
        {
            if(bInitPos)
            {
                *pStackPos = pAlloc->GetCurrPos();
            }
            if(bWipe)
            {
                wipeTo = *pStackPos;
            }
            AllocatorsStack::Instance()->PushAllocator(pAlloc);
        }
    }
    
    ~AutoStackAllocator()
    {
        if(pAlloc)
        {
            if(bWipe)
            {
                pAlloc->SetCurrPos(wipeTo);
            }
            AllocatorsStack::Instance()->PopAllocator();
        }
    }
    
private:
    StackAllocator *pAlloc;
    bool bInitPos;
    bool bWipe;
    uint32 wipeTo;
};
#endif

};

#endif //__DAVAENGINE_ALLOCATOR_FACTORY_H__