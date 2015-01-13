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



#include "MemoryAllocationTraverseTest.h"


FixedSizePoolAllocator EntityCustomAllocator::allocator(sizeof(EntityCustomAllocator), MemoryAllocationTraverseTest::TEST_ITEM_COUNT);

MemoryAllocationTraverseTest::MemoryAllocationTraverseTest(const String &testName)
    :   TestTemplate<MemoryAllocationTraverseTest>(testName)
{
    RegisterFunction(this, &MemoryAllocationTraverseTest::MemoryAllocationTest_Default, "MemoryAllocationTest_DefaultAllocator", 50,  NULL);
    RegisterFunction(this, &MemoryAllocationTraverseTest::MemoryAllocationTest_Custom, "MemoryAllocationTest_CustomAllocator", 50,  NULL);

    RegisterFunction(this, &MemoryAllocationTraverseTest::MemoryAllocationDeallocationTest_Default, "MemoryAllocationDeallocationTest_DefaultAllocator", 50,  NULL);
    RegisterFunction(this, &MemoryAllocationTraverseTest::MemoryAllocationDeallocationTest_Custom, "MemoryAllocationDeallocationTest_CustomAllocator", 50,  NULL);

    RegisterFunction(this, &MemoryAllocationTraverseTest::MemoryTraverseTest_Default, "MemoryTraverseTest_Default", 50,  NULL);
    RegisterFunction(this, &MemoryAllocationTraverseTest::MemoryTraverseTest_Custom, "MemoryTraverseTest_Custom", 50,  NULL);
}


void MemoryAllocationTraverseTest::LoadResources()
{
    Logger::Debug("size default: %d", sizeof(EntityDefaultAllocator));
    Logger::Debug("size custom: %d", sizeof(EntityCustomAllocator));
    
    int32 defaultMaxDiffBytes = 0;
    int32 defaultMinDiffBytes = 0;
    
    defaultTree = new EntityDefaultAllocator;
    EntityDefaultAllocator * defaultPrevAllocation = defaultTree;

    for (uint32 k = 0; k < 600; ++k)
    {
        EntityDefaultAllocator * item = new EntityDefaultAllocator;
        int32 diffBytes = (uint8*)item - (uint8*)defaultPrevAllocation;
        if (diffBytes > defaultMaxDiffBytes)defaultMaxDiffBytes = diffBytes;
        if (diffBytes < defaultMinDiffBytes)defaultMinDiffBytes = diffBytes;
        defaultPrevAllocation = item;
        
        defaultTree->children.push_back(item);
    
        for (uint32 k = 0; k < 5; ++k)
        {
            EntityDefaultAllocator * subItem = new EntityDefaultAllocator;
            int32 diffBytes = (uint8*)subItem - (uint8*)defaultPrevAllocation;
            if (diffBytes > defaultMaxDiffBytes)defaultMaxDiffBytes = diffBytes;
            if (diffBytes < defaultMinDiffBytes)defaultMinDiffBytes = diffBytes;
            defaultPrevAllocation = subItem;
            
            item->children.push_back(subItem);
        }
    }
    Logger::Debug("DefaultAllocated: %d %d", defaultMaxDiffBytes, defaultMinDiffBytes);
    
    
    int32 customMaxDiffBytes = 0;
    int32 customMinDiffBytes = 0;

    customTree = new EntityCustomAllocator;
    EntityCustomAllocator * customPrevAllocation = customTree;
    
    for (uint32 k = 0; k < 600; ++k)
    {
        EntityCustomAllocator * item = new EntityCustomAllocator;
        int32 diffBytes = (uint8*)item - (uint8*)customPrevAllocation;
        if (diffBytes > customMaxDiffBytes)customMaxDiffBytes = diffBytes;
        if (diffBytes < customMinDiffBytes)customMinDiffBytes = diffBytes;
        customPrevAllocation = item;
        
        customTree->children.push_back(item);
        
        for (uint32 k = 0; k < 5; ++k)
        {
            EntityCustomAllocator * subItem = new EntityCustomAllocator;
            int32 diffBytes = (uint8*)subItem - (uint8*)customPrevAllocation;
            if (diffBytes > customMaxDiffBytes)customMaxDiffBytes = diffBytes;
            if (diffBytes < customMinDiffBytes)customMinDiffBytes = diffBytes;
            customPrevAllocation = subItem;
            
            item->children.push_back(subItem);
        }
    }
    Logger::Debug("CustomAllocated: %d %d", customMaxDiffBytes, customMinDiffBytes);
}

void MemoryAllocationTraverseTest::UnloadResources()
{    
}

void MemoryAllocationTraverseTest::MemoryAllocationTest_Default(PerfFuncData * data)
{
    for (uint32 k = 0; k < TEST_ITEM_COUNT; ++k)
    {
        defaultEntityArray[k] = new EntityDefaultAllocator();
    }
}

void MemoryAllocationTraverseTest::MemoryAllocationTest_Custom(PerfFuncData * data)
{
    for (uint32 k = 0; k < TEST_ITEM_COUNT; ++k)
    {
        customEntityArray[k] = new EntityCustomAllocator();
    }
}

void MemoryAllocationTraverseTest::MemoryAllocationDeallocationTest_Default(PerfFuncData * data)
{
    for (uint32 k = 0; k < TEST_ITEM_COUNT; ++k)
    {
        defaultEntityArray[k] = new EntityDefaultAllocator();
    }
    for (uint32 k = 0; k < TEST_ITEM_COUNT; ++k)
    {
        SafeRelease(defaultEntityArray[k]);
    }
}

void MemoryAllocationTraverseTest::MemoryAllocationDeallocationTest_Custom(PerfFuncData * data)
{
    for (uint32 k = 0; k < TEST_ITEM_COUNT; ++k)
    {
        customEntityArray[k] = new EntityCustomAllocator();
    }
    for (uint32 k = 0; k < TEST_ITEM_COUNT; ++k)
    {
        SafeRelease(customEntityArray[k]);
    }
}

void MemoryAllocationTraverseTest::TraverseDefault(EntityDefaultAllocator * node)
{
    node->components[0] = 0;
    uint32 size = node->children.size();
    for (uint32 k = 0; k < size; ++k)
    {
        TraverseDefault(node->children[k]);
    }
}

void MemoryAllocationTraverseTest::TraverseCustom(EntityCustomAllocator * node)
{
    node->components[0] = 0;
    uint32 size = node->children.size();
    for (uint32 k = 0; k < size; ++k)
    {
        TraverseCustom(node->children[k]);
    }
}

void MemoryAllocationTraverseTest::MemoryTraverseTest_Default(PerfFuncData * data)
{
    TraverseDefault(defaultTree);
}

void MemoryAllocationTraverseTest::MemoryTraverseTest_Custom(PerfFuncData * data)
{
    TraverseCustom(customTree);
}

