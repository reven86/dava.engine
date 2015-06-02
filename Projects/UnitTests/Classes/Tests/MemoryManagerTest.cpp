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


#include "MemoryManagerTest.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "MemoryManager/MemoryProfiler.h"

using namespace DAVA;

MemoryManagerTest::MemoryManagerTest()
    : TestTemplate<MemoryManagerTest>("MemoryManagerTest")
    , capturedTag(0)
    , capturedCheckpoint(0)
{
    RegisterFunction(this, &MemoryManagerTest::TestZeroAlloc, "TestZeroAlloc", NULL);
    RegisterFunction(this, &MemoryManagerTest::TestAlignedAlloc, "TestAlignedAlloc", NULL);
    RegisterFunction(this, &MemoryManagerTest::TestCallback, "TestCallback", NULL);
}

void MemoryManagerTest::TestZeroAlloc(PerfFuncData* data)
{
    void* ptr1 = MemoryManager::Instance()->Allocate(0, ALLOC_POOL_APP);
    void* ptr2 = MemoryManager::Instance()->Allocate(0, ALLOC_POOL_APP);
    void* ptr3 = MemoryManager::Instance()->AlignedAllocate(0, 16, ALLOC_POOL_APP);
    void* ptr4 = MemoryManager::Instance()->AlignedAllocate(0, 16, ALLOC_POOL_APP);

    // When allocating zero bytes MemoryManager returns unique pointer which can be safely deallocated
    TEST_VERIFY(ptr1 != nullptr);
    TEST_VERIFY(ptr1 != ptr2);
    TEST_VERIFY(ptr2 != ptr3);
    TEST_VERIFY(ptr3 != ptr4);

    MemoryManager::Instance()->Deallocate(ptr1);
    MemoryManager::Instance()->Deallocate(ptr2);
    MemoryManager::Instance()->Deallocate(ptr3);
    MemoryManager::Instance()->Deallocate(ptr4);
}

void MemoryManagerTest::TestAlignedAlloc(PerfFuncData* data)
{
    // Alignment should be power of 2
    size_t align[] = {
        2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
    };
    for (auto x : align)
    {
        void* ptr = MemoryManager::Instance()->AlignedAllocate(128, x, ALLOC_POOL_APP);
        // On many platforms std::size_t can safely store the value of any non-member pointer, in which case it is synonymous with std::uintptr_t.
        size_t addr = reinterpret_cast<size_t>(ptr);
        TEST_VERIFY(addr % x == 0);
        MemoryManager::Instance()->Deallocate(ptr);
    }
}

void MemoryManagerTest::TestCallback(PerfFuncData* data)
{
    const size_t TAG = 1;
    const size_t CHECKPOINT = 100;

    MemoryManager::InstallDumpCallback(&DumpRequestCallback, this);

    MEMORY_PROFILER_ENTER_TAG(TAG);
    MEMORY_PROFILER_LEAVE_TAG(TAG);

    MEMORY_PROFILER_CHECKPOINT(CHECKPOINT);

    TEST_VERIFY(capturedTag == TAG);
    TEST_VERIFY(capturedCheckpoint == CHECKPOINT);

    MemoryManager::InstallDumpCallback(nullptr, nullptr);
}

void MemoryManagerTest::DumpRequestCallback(void* arg, int32 type, uint32 tagOrCheckpoint, uint32 blockBegin, uint32 blockEnd)
{
    MemoryManagerTest* self = static_cast<MemoryManagerTest*>(arg);
    self->OnDumpRequest(type, tagOrCheckpoint, blockBegin, blockEnd);
}

void MemoryManagerTest::OnDumpRequest(int32 type, uint32 tagOrCheckpoint, uint32 blockBegin, uint32 blockEnd)
{
    if (MMConst::DUMP_REQUEST_TAG == type)
    {
        capturedTag = tagOrCheckpoint;
    }
    else if (MMConst::DUMP_REQUEST_CHECKPOINT == type)
    {
        capturedCheckpoint = tagOrCheckpoint;
    }
}

#endif  // DAVA_MEMORY_PROFILING_ENABLE
