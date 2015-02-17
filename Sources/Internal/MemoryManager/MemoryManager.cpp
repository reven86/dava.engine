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

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cassert>

#if defined(MEMPROF_WIN32)
#include <windows.h>
#include <dbghelp.h>
#elif defined(MEMPROF_MACOS) || defined(MEMPROF_IOS)
#include <execinfo.h>
#include <malloc/malloc.h>
#elif defined(MEMPROF_ANDROID)
#endif

#include "Debug/DVAssert.h"

#include "MallocHook.h"
#include "MemoryManager.h"

namespace DAVA
{

struct MemoryManager::MemoryBlock
{
    size_t mark;            // Mark to distinguish profiled memory blocks
    size_t pool;            // Allocation pool block belongs to
    MemoryBlock* prev;      // Pointer to previous block
    MemoryBlock* next;      // Pointer to next block
    size_t allocByApp;      // Size requested by application
    size_t allocTotal;      // Total allocated size
    size_t orderNo;         // Block order number
    size_t padding;
};
    
MMItemName MemoryManager::tagNames[MAX_TAG_COUNT] = {
    {"application"}
};

MMItemName MemoryManager::allocPoolNames[MAX_ALLOC_POOL_COUNT] = {
    {"internal"},
    {"application"}
};

size_t MemoryManager::registeredTagCount = 1;
size_t MemoryManager::registeredAllocPoolCount = 2;
    
void MemoryManager::RegisterAllocPoolName(size_t index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MMItemName::NAME_LENGTH);
    DVASSERT(FIRST_CUSTOM_ALLOC_POOL <= index && index < MAX_ALLOC_POOL_COUNT);
    DVASSERT(allocPoolNames[index - 1].name[0] != '\0');     // Names should be registered sequentially with no gap

    strncpy(allocPoolNames[index].name, name, MMItemName::NAME_LENGTH);
    allocPoolNames[index].name[MMItemName::NAME_LENGTH - 1] = '\0';
    registeredAllocPoolCount += 1;
}

void MemoryManager::RegisterTagName(size_t index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MMItemName::NAME_LENGTH);
    DVASSERT(DEFAULT_TAG != index && index < MAX_TAG_COUNT);
    DVASSERT(tagNames[index - 1].name[0] != '\0');       // Names should be registered sequentially with no gap

    strncpy(tagNames[index].name, name, MMItemName::NAME_LENGTH);
    tagNames[index].name[MMItemName::NAME_LENGTH - 1] = '\0';
    registeredTagCount += 1;
}

MemoryManager* MemoryManager::Instance()
{
    static_assert(sizeof(MemoryManager::MemoryBlock) % 16 == 0, "sizeof(MemoryManager::MemoryBlock) % 16 == 0");
    static MallocHook hook;
    static MemoryManager mm;
    return &mm;
}

void* MemoryManager::Alloc(size_t size, uint32 poolIndex)
{
    // TODO: what should be done when size is 0
    assert(0 <= poolIndex && poolIndex < MAX_ALLOC_POOL_COUNT);

    size_t totalSize = sizeof(MemoryBlock) + size;
    if (totalSize & (BLOCK_ALIGN - 1))
        totalSize += (BLOCK_ALIGN - (totalSize & (BLOCK_ALIGN - 1)));

    LockType lock(mutex);
    MemoryBlock* block = static_cast<MemoryBlock*>(MallocHook::Malloc(totalSize));
    if (block != nullptr)
    {
        block->mark = BLOCK_MARK;
        block->pool = poolIndex;
        block->allocByApp = size;
        block->allocTotal = totalSize;
        block->orderNo = nextBlockNo++;

        InsertBlock(block);
        UpdateStatAfterAlloc(block, poolIndex);
        return static_cast<void*>(block + 1);
    }
    return nullptr;
}

void MemoryManager::Dealloc(void* ptr)
{
    if (ptr != nullptr)
    {
        LockType lock(mutex);
        MemoryBlock* block = IsProfiledBlock(ptr);
        if (block != nullptr)
        {
            block->mark = BLOCK_DELETED;
            RemoveBlock(block);
            UpdateStatAfterDealloc(block, block->pool);
            MallocHook::Free(block);
        }
        else
        {
            statGeneral.ghostBlockCount += 1;
            statGeneral.ghostSize += 0;       // TODO: get memory block size from pointer
            MallocHook::Free(ptr);
        }
    }
}

void MemoryManager::EnterScope(uint32 tag)
{
    assert(tag != DEFAULT_TAG);
    assert(tags.depth < MMTagStack::MAX_DEPTH - 1);

    LockType lock(mutex);
    tags.depth += 1;
    tags.stack[tags.depth] = tag;
    tags.begin[tags.depth] = nextBlockNo;
}

void MemoryManager::LeaveScope()
{
    assert(tags.depth > 0);

    LockType lock(mutex);
    // TODO: perform action on tag leave
    for (size_t i = 0;i < MAX_ALLOC_POOL_COUNT;++i)
    {
        statAllocPool[tags.depth][i] = AllocPoolStat();
        // TODO: clear additional counters on tag leave
    }
    tags.depth -= 1;
}

void MemoryManager::InsertBlock(MemoryBlock* block)
{
    if (head != nullptr)
    {
        block->next = head;
        block->prev = nullptr;
        head->prev = block;
        head = block;
    }
    else
    {
        block->next = nullptr;
        block->prev = nullptr;
        head = block;
    }
}

void MemoryManager::RemoveBlock(MemoryBlock* block)
{
    if (block->prev != nullptr)
        block->prev->next = block->next;
    if (block->next != nullptr)
        block->next->prev = block->prev;
    if (block == head)
        head = head->next;
}

MemoryManager::MemoryBlock* MemoryManager::IsProfiledBlock(void* ptr)
{
    MemoryBlock* block = static_cast<MemoryBlock*>(ptr) - 1;
    return BLOCK_MARK == block->mark ? block : nullptr;
}

size_t MemoryManager::ProfiledBlockSize(void* ptr)
{
    if (ptr != nullptr)
    {
        // TODO: is lock necessary here
        LockType lock(mutex);
        MemoryBlock* block = IsProfiledBlock(ptr);
        return block != nullptr ? block->allocByApp : 0;
    }
    return 0;
}

MemoryManager::MemoryBlock* MemoryManager::FindBlockByOrderNo(uint32 orderNo)
{
    MemoryBlock* cur = head;
    while (cur != nullptr)
    {
        if (cur->orderNo == orderNo)
            return cur;
        cur = cur->next;
    }
    return nullptr;
}

void MemoryManager::UpdateStatAfterAlloc(MemoryBlock* block, uint32 poolIndex)
{
    for (size_t i = 0;i <= tags.depth;++i)
    {
        // Calculate fixed statistics for allocation pool
        statAllocPool[i][poolIndex].allocByApp += block->allocByApp;
        statAllocPool[i][poolIndex].allocTotal += block->allocTotal;
        statAllocPool[i][poolIndex].blockCount += 1;

        if (block->allocByApp > statAllocPool[i][poolIndex].maxBlockSize)
            statAllocPool[i][poolIndex].maxBlockSize = block->allocByApp;

        // Compute additional counters for allocation pool
        // ...
    }
}

void MemoryManager::UpdateStatAfterDealloc(MemoryBlock* block, uint32 poolIndex)
{
    for (size_t i = 0;i <= tags.depth;++i)
    {
        assert(statAllocPool[i][poolIndex].blockCount >= 1);
        assert(statAllocPool[i][poolIndex].allocByApp >= block->allocByApp);
        assert(statAllocPool[i][poolIndex].allocTotal >= block->allocTotal);

        // Calculate fixed statistics for allocation pool
        statAllocPool[i][poolIndex].allocByApp -= block->allocByApp;
        statAllocPool[i][poolIndex].allocTotal -= block->allocTotal;
        statAllocPool[i][poolIndex].blockCount -= 1;

        // Compute additional counters for allocation pool
        // ...
    }
}

size_t MemoryManager::CalcStatConfigSize()
{
    return sizeof(MMStatConfig) + sizeof(MMItemName) * (registeredTagCount + registeredAllocPoolCount - 1);
}

void MemoryManager::GetStatConfig(MMStatConfig* config)
{
    DVASSERT(config != nullptr);
    
    config->maxTagCount = MAX_TAG_COUNT;
    config->maxAllocPoolCount = MAX_ALLOC_POOL_COUNT;
    config->tagCount = static_cast<uint32>(registeredTagCount);
    config->allocPoolCount = static_cast<uint32>(registeredAllocPoolCount);
    
    size_t k = 0;
    for (size_t i = 0;i < registeredTagCount;++i, ++k)
        config->names[k] = tagNames[i];
    for (size_t i = 0;i < registeredAllocPoolCount;++i, ++k)
        config->names[k] = allocPoolNames[i];
}

size_t MemoryManager::CalcStatSizeInternal() const
{
    return sizeof(MMStat) + sizeof(AllocPoolStat) * (tags.depth + registeredAllocPoolCount - 1);
}

void MemoryManager::GetStatInternal(MMStat* stat)
{
    DVASSERT(stat != nullptr);
    
    LockType lock(mutex);
    stat->timestamp = 0;
    stat->allocCount = nextBlockNo;
    stat->allocPoolCount = static_cast<uint32>(registeredAllocPoolCount);
    stat->tags = tags;
    stat->generalStat = statGeneral;
    
    size_t k = 0;
    for (uint32 i = 0;i <= tags.depth;++i)
    {
        for (uint32 j = 0;j < stat->allocPoolCount;++j, ++k)
        {
            stat->poolStat[k] = statAllocPool[i][j];
        }
    }
}

#if 0
void mem_profiler::collect_backtrace(mem_block_t* block, size_t nskip)
{
#if defined(MEMPROF_WIN32)
    USHORT n = CaptureStackBackTrace(nskip + 0, backtrace_t::MAX_FRAMES, block->backtrace.frames, nullptr);
    block->backtrace.depth = static_cast<size_t>(n); 
#elif defined(MEMPROF_MACOS) || defined(MEMPROF_IOS)
    int n = backtrace(block->backtrace.frames, backtrace_t::MAX_FRAMES);
    block->backtrace.depth = static_cast<size_t>(n);
#elif defined(MEMPROF_ANDROID)
    block->backtrace.depth = 0;
#endif
}
#endif

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
