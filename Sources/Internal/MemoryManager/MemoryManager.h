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

#ifndef __DAVAENGINE_MEMORYMANAGER_H__
#define __DAVAENGINE_MEMORYMANAGER_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "Thread/Spinlock.h"
#include "Thread/LockGuard.h"

#include "AllocPools.h"
#include "MemoryManagerTypes.h"

namespace DAVA
{

class MemoryManager final
{
    struct MemoryBlock;

    static const uint32 BLOCK_MARK = 0xBA0BAB;
    static const uint32 BLOCK_DELETED = 0xACCA;
    static const size_t BLOCK_ALIGN = 16;

    static const size_t COUNTERS_COUNT = 0;
    static const size_t POOL_COUNTER_COUNT = 0;

    struct GeneralStat
    {
        GeneralAllocStat fixedStat;
        uint32 counters[1];
    };

    struct PoolStat
    {
        AllocPoolStat fixedStat;
        uint32 counters[1];
    };

public:
    static const size_t MAX_TAG_DEPTH = 8;              // Maximum depth of tag stack
    static const size_t DEFAULT_TAG = 0;                // Default tag which corresponds to whole application time line

    static const size_t MAX_ALLOC_POOL_COUNT = 16;      // Max supported count of allocation pools
    static const size_t MAX_TAG_COUNT = 16;             // Max supported count of tags
    static const size_t MAX_NAME_LENGTH = 16;           // Max length of name: tag, allocation type, counter

public:
    MemoryManager() = default;
    ~MemoryManager() = default;

    static void RegisterAllocPoolName(size_t index, const char8* name);
    static void RegisterTagName(size_t index, const char8* name);

    static MemoryManager* Instance();

    static void* Allocate(size_t size, uint32 poolIndex);
    static void Deallocate(void* ptr);
    static size_t BlockSize(void* ptr);

    static void EnterTagScope(uint32 tag);
    static void LeaveTagScope();

    static size_t GetTagDescription();
    static size_t GetAllocTypeDescription();
    static size_t GetCounterDescription();
    static size_t GetAllocTypeCounterDescription();
    
private:
    void* Alloc(size_t size, uint32 poolIndex);
    void Dealloc(void* ptr);

    void EnterScope(uint32 tag);
    void LeaveScope();

    void InsertBlock(MemoryBlock* block);
    void RemoveBlock(MemoryBlock* block);
    MemoryBlock* IsProfiledBlock(void* ptr);
    size_t ProfiledBlockSize(void* ptr);
    MemoryBlock* FindBlockByOrderNo(uint32 orderNo);

    void UpdateStatAfterAlloc(MemoryBlock* block, uint32 poolIndex);
    void UpdateStatAfterDealloc(MemoryBlock* block, uint32 poolIndex);

#if 0
    void collect_backtrace(mem_block_t* block, size_t nskip);

    void internal_dump(FILE* file);
    void internal_dump_memory_type(FILE* file, size_t index);
    void internal_dump_tag(const bookmark_t& bookmark);
    void internal_dump_backtrace(FILE* file, mem_block_t* block);
#endif

private:
    MemoryBlock* head;                  // Linked list of memory blocks
    uint32 nextBlockNo;                 // Next assigned number to next allocated memory block
    size_t tagDepth;                    // Current tag depth
    uint32 tagStack[MAX_TAG_DEPTH];     // Current active tags
    uint32 tagBegin[MAX_TAG_DEPTH];     // Block numbers from which each tag begins
    GeneralStat statGeneral;            // General statistics
    PoolStat statAllocPool[MAX_TAG_DEPTH][MAX_ALLOC_POOL_COUNT];    // Statistics for each allocation pool divided by tags
    
    typedef DAVA::Spinlock MutexType;
    typedef DAVA::LockGuard<MutexType> LockType;
    MutexType mutex;
    
private:
    static char8 tagNames[MAX_TAG_COUNT][MAX_NAME_LENGTH];               // Names of tags
    static char8 allocPoolNames[MAX_ALLOC_POOL_COUNT][MAX_NAME_LENGTH];  // Names of allocation pools
};

//////////////////////////////////////////////////////////////////////////
inline void* MemoryManager::Allocate(size_t size, uint32 poolIndex)
{
    return Instance()->Alloc(size, poolIndex);
}

inline void MemoryManager::Deallocate(void* ptr)
{
    Instance()->Dealloc(ptr);
}

inline size_t MemoryManager::BlockSize(void* ptr)
{
    return Instance()->ProfiledBlockSize(ptr);
}

inline void MemoryManager::EnterTagScope(uint32 tag)
{
    Instance()->EnterScope(tag);
}

inline void MemoryManager::LeaveTagScope()
{
    Instance()->LeaveScope();
}

}   // namespace DAVA

#define MEMPROF_INIT()      mem_profiler::instance()
#define MEMPROF_ENTER(tag)  mem_profiler::instance()->enter_scope(tag)
#define MEMPROF_LEAVE()     mem_profiler::instance()->leave_scope()
#define MEMPROF_DUMP(file)  mem_profiler::instance()->dump(file)

#else   // defined(DAVA_MEMORY_PROFILING_ENABLE)

#define MEMPROF_INIT()
#define MEMPROF_ENTER(tag)
#define MEMPROF_LEAVE()
#define MEMPROF_DUMP(file)

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MEMORYMANAGER_H__
