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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "MemoryManager/MemoryProfiler.h"

DAVA_TESTCLASS(MemoryManagerTest)
{
    DEDUCE_COVERED_CLASS_FROM_TESTCLASS()

    volatile uint32 capturedTag = 0;
    volatile uint32 capturedCheckpoint = 0;

    DAVA_TEST(TestZeroAlloc)
    {
        void* ptr1 = MemoryManager::Instance()->Allocate(0, ALLOC_POOL_DEFAULT);
        void* ptr2 = MemoryManager::Instance()->Allocate(0, ALLOC_POOL_DEFAULT);

        // When allocating zero bytes MemoryManager returns unique pointer which can be safely deallocated
        TEST_VERIFY(ptr1 != nullptr);
        TEST_VERIFY(ptr1 != ptr2);

        MemoryManager::Instance()->Deallocate(ptr1);
        MemoryManager::Instance()->Deallocate(ptr2);
    }

    DAVA_TEST(TestAlignedAlloc)
    {
        // Alignment should be power of 2
        size_t align[] = {
            2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
        };
        for (auto x : align)
        {
            void* ptr = MemoryManager::Instance()->AlignedAllocate(128, x, ALLOC_POOL_DEFAULT);
            // On many platforms std::size_t can safely store the value of any non-member pointer, in which case it is synonymous with std::uintptr_t.
            size_t addr = reinterpret_cast<size_t>(ptr);
            TEST_VERIFY(addr % x == 0);
            MemoryManager::Instance()->Deallocate(ptr);
        }
    }

    DAVA_TEST(TestGPUTracking)
    {
        const size_t statSize = MemoryManager::Instance()->CalcCurStatSize();
        void* buffer = ::operator new(statSize);
        MMCurStat* stat = static_cast<MMCurStat*>(buffer);
        AllocPoolStat* poolStat = OffsetPointer<AllocPoolStat>(buffer, sizeof(MMCurStat));

        MemoryManager::Instance()->GetCurStat(0, buffer, statSize);
        uint32 oldAllocByApp = poolStat[ALLOC_GPU_TEXTURE].allocByApp;
        uint32 oldBlockCount = poolStat[ALLOC_GPU_TEXTURE].blockCount;

        DAVA_MEMORY_PROFILER_GPU_ALLOC(101, 100, ALLOC_GPU_TEXTURE);
        DAVA_MEMORY_PROFILER_GPU_ALLOC(101, 200, ALLOC_GPU_TEXTURE);

        MemoryManager::Instance()->GetCurStat(0, buffer, statSize);
        TEST_VERIFY(oldAllocByApp + 300 == poolStat[ALLOC_GPU_TEXTURE].allocByApp);
        TEST_VERIFY(oldBlockCount + 2 == poolStat[ALLOC_GPU_TEXTURE].blockCount);

        DAVA_MEMORY_PROFILER_GPU_DEALLOC(101, ALLOC_GPU_TEXTURE);
        MemoryManager::Instance()->GetCurStat(0, buffer, statSize);

        TEST_VERIFY(oldAllocByApp == poolStat[ALLOC_GPU_TEXTURE].allocByApp);
        TEST_VERIFY(oldBlockCount == poolStat[ALLOC_GPU_TEXTURE].blockCount);

        ::operator delete(buffer);
    }

    DAVA_TEST(TestAllocScope)
    {
        const size_t statSize = MemoryManager::Instance()->CalcCurStatSize();
        void* buffer = ::operator new(statSize);
        MMCurStat* stat = static_cast<MMCurStat*>(buffer);
        AllocPoolStat* poolStat = OffsetPointer<AllocPoolStat>(buffer, sizeof(MMCurStat));

        {
            MemoryManager::Instance()->GetCurStat(0, buffer, statSize);
            uint32 oldAllocByApp = poolStat[ALLOC_POOL_BULLET].allocByApp;
            uint32 oldBlockCount = poolStat[ALLOC_POOL_BULLET].blockCount;

            DAVA_MEMORY_PROFILER_ALLOC_SCOPE(ALLOC_POOL_BULLET);

            // Use volatile keyword to prevent optimizer to throw allocations out
            void* volatile ptr1 = malloc(222);
            char* volatile ptr2 = new char[111];

            MemoryManager::Instance()->GetCurStat(0, buffer, statSize);
            TEST_VERIFY(oldAllocByApp + 222 + 111 == poolStat[ALLOC_POOL_BULLET].allocByApp);
            TEST_VERIFY(oldBlockCount + 1 + 1 == poolStat[ALLOC_POOL_BULLET].blockCount);

            free(ptr1);
            delete[] ptr2;

            MemoryManager::Instance()->GetCurStat(0, buffer, statSize);
            TEST_VERIFY(oldAllocByApp == poolStat[ALLOC_POOL_BULLET].allocByApp);
            TEST_VERIFY(oldBlockCount == poolStat[ALLOC_POOL_BULLET].blockCount);
        }

        ::operator delete(buffer);
    }

    DAVA_TEST(TestCallback)
    {
        const uint32 TAG = 1;

        MemoryManager::Instance()->SetCallbacks(nullptr, MakeFunction(this, &MemoryManagerTest::TagCallback));

        DAVA_MEMORY_PROFILER_ENTER_TAG(TAG);
        DAVA_MEMORY_PROFILER_LEAVE_TAG(TAG);

        TEST_VERIFY(enteredTag == TAG);
        TEST_VERIFY(leftTag == TAG);
    }

    void TagCallback(uint32 tag, bool entering)
    {
        if (entering)
            enteredTag = tag;
        else
            leftTag = tag;
    }

    volatile uint32 enteredTag = 0;
    volatile uint32 leftTag = 0;
};

#endif  // DAVA_MEMORY_PROFILING_ENABLE
