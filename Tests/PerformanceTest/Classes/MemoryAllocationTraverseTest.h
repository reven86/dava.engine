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



#ifndef __MEMORY_ALLOCATION_TRAVERSE_TEST_H__
#define __MEMORY_ALLOCATION_TRAVERSE_TEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class EntityDefaultAllocator : public BaseObject
{
protected:
    ~EntityDefaultAllocator(){}
public:
    Component * components[Component::COMPONENT_COUNT];
    Vector<EntityDefaultAllocator*> children;
};

class EntityCustomAllocator : public BaseObject
{
protected:
    ~EntityCustomAllocator(){}
public:
    Component * components[Component::COMPONENT_COUNT];
    Vector<EntityCustomAllocator*> children;
    
    void * operator new(size_t size)
    {
        return allocator.New();
    }
    
    void operator delete(void * p)
    {
        return allocator.Delete(p);
    }
    
    static FixedSizePoolAllocator allocator;
};


class EntityCustomSmallAllocator : public BaseObject
{
protected:
    ~EntityCustomSmallAllocator(){}
public:
    uint32 flags;
    void * dataPtr;
    Vector<EntityCustomAllocator*> children;
    
    void * operator new(size_t size)
    {
        return allocator.New();
    }
    
    void operator delete(void * p)
    {
        return allocator.Delete(p);
    }
    
    static FixedSizePoolAllocator allocator;
};

//class EntityCustomAllocator : public BaseObject
//{
//public:
//    Component * components[Component::COMPONENT_COUNT];
//    Vector<EntityCustomAllocator*> children;
//    
//    void * operator new(size_t size)
//    {
//        return allocator.New();
//    }
//    
//    void operator delete(void * p)
//    {
//        return allocator.Delete(p);
//    }
//    
//    static FixedSizePoolAllocator allocator;
//};
//
//
//class EntityFactory
//{
//public:
//    Vector<Entity
//    
//    
//    
//    
//};


class MemoryAllocationTraverseTest: public TestTemplate<MemoryAllocationTraverseTest>
{
protected:
    ~MemoryAllocationTraverseTest(){}
public:
	MemoryAllocationTraverseTest(const String &testName);
    
	virtual void LoadResources();
	virtual void UnloadResources();
    
    static const uint32 TEST_ITEM_COUNT = 10000;
    
    EntityDefaultAllocator * defaultEntityArray[TEST_ITEM_COUNT];
    EntityCustomAllocator * customEntityArray[TEST_ITEM_COUNT];
    
    EntityDefaultAllocator * defaultTree;
    EntityCustomAllocator * customTree;
    EntityCustomSmallAllocator * customSmallTree;
    
protected:
    void MemoryAllocationTest_Default(PerfFuncData * data);
    void MemoryAllocationTest_Custom(PerfFuncData * data);
    void MemoryAllocationDeallocationTest_Default(PerfFuncData * data);
    void MemoryAllocationDeallocationTest_Custom(PerfFuncData * data);
    void MemoryDeallocationTest(PerfFuncData * data);
    
    void MemoryTraverseTest_Default(PerfFuncData * data);
    void MemoryTraverseTest_Custom(PerfFuncData * data);
    void MemoryTraverseTest_CustomSmall(PerfFuncData * data);
    
    void TraverseDefault(EntityDefaultAllocator * node);
    void TraverseCustom(EntityCustomAllocator * node);
    void TraverseCustomSmall(EntityCustomAllocator * node);
    
    

};


#endif // __CACHE_TEST_H__
