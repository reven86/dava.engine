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

#include <unordered_map>
#include <unordered_set>

#include "Thread/Spinlock.h"
#include "Thread/LockGuard.h"

#include "AllocPools.h"
#include "MemoryManagerTypes.h"

namespace DAVA
{

template<typename T>
class WhatsUp;

class MemoryManager final
{
    struct MemoryBlock;
    struct Backtrace;

    static const uint32 BLOCK_MARK = 0xBA0BAB;
    static const uint32 BLOCK_DELETED = 0xACCA;
    static const size_t BLOCK_ALIGN = 16;

public:
    static const size_t MAX_TAG_DEPTH = 8;              // Maximum depth of tag stack
    static const size_t DEFAULT_TAG = 0;                // Default tag which corresponds to whole application time line

    static const size_t MAX_ALLOC_POOL_COUNT = 6;       // Max supported count of allocation pools
    static const size_t MAX_TAG_COUNT = 4;              // Max supported count of tags
    static const size_t MAX_NAME_LENGTH = 16;           // Max length of name: tag, allocation type, counter

    typedef void (*TagCallback)(void* arg, uint32 tag, uint32 tagBegin, uint32 tagEnd);

public:
    MemoryManager() = default;
    ~MemoryManager() = default;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator = (const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator = (MemoryManager&&) = delete;

    static void RegisterAllocPoolName(size_t index, const char8* name);
    static void RegisterTagName(size_t index, const char8* name);

    static MemoryManager* Instance();

    static void InstallTagCallback(TagCallback callback, void* arg);

    static void* Allocate(size_t size, uint32 poolIndex);
    static void Deallocate(void* ptr);
    
    static void* Reallocate(void * ptr, size_t size);

    static size_t BlockSize(void* ptr);

    static void EnterTagScope(uint32 tag);
    static void LeaveTagScope();
    
    static size_t CalcStatConfigSize();
    static void GetStatConfig(MMStatConfig* config);
    
    static size_t CalcStatSize();
    static void GetStat(MMStat* stat);

    static size_t GetDump(size_t userSize, void** buf, uint32 blockRangeBegin, uint32 blockRangeEnd);
    static void FreeDump(void* ptr);

private:
    void* Alloc(size_t size, uint32 poolIndex);
    void Dealloc(void* ptr);
    void * Realloc(void *ptr, size_t size);

    void EnterScope(uint32 tag);
    void LeaveScope();

    void InsertBlock(MemoryBlock* block);
    void RemoveBlock(MemoryBlock* block);
    MemoryBlock* IsProfiledBlock(void* ptr);
    size_t ProfiledBlockSize(void* ptr);
    MemoryBlock* FindBlockByOrderNo(uint32 orderNo);

    void UpdateStatAfterAlloc(MemoryBlock* block, uint32 poolIndex);
    void UpdateStatAfterDealloc(MemoryBlock* block, uint32 poolIndex);
    
    size_t CalcStatSizeInternal() const;
    void GetStatInternal(MMStat* stat);

    size_t GetDumpInternal(size_t userSize, void** buf, uint32 blockRangeBegin, uint32 blockRangeEnd);
    void FreeDumpInternal(void* ptr);
    size_t GetBlockRange(uint32 rangeBegin, uint32 rangeEnd, MemoryBlock** begin, MemoryBlock** end);

    void CollectBacktrace(MemoryBlock* block, size_t nskip);
    void ObtainBacktraceSymbols(const Backtrace* backtrace);
    void ObtainAllBacktraceSymbols();

private:
    MemoryBlock* head;                  // Linked list of memory blocks
    uint32 nextBlockNo;                 // Next assigned number to next allocated memory block
    MMTagStack tags;                    // Active tags
    GeneralAllocStat statGeneral;       // General statistics
    AllocPoolStat statAllocPool[MAX_TAG_DEPTH][MAX_ALLOC_POOL_COUNT];    // Statistics for each allocation pool divided by tags
    
    typedef DAVA::Spinlock MutexType;
    typedef DAVA::LockGuard<MutexType> LockType;
    MutexType mutex;
    
    TagCallback tagCallback;
    void* callbackArg;

    template<typename T>
    using InternalAllocator = MemoryManagerAllocator<T, ALLOC_POOL_INTERNAL>;

    typedef std::basic_string<char8, std::char_traits<char8>, InternalAllocator<char8>> InternalString;
    typedef std::unordered_map<void*, InternalString, std::hash<void*>, std::equal_to<void*>, InternalAllocator<std::pair<void* const, InternalString>>> SymbolMap;

    SymbolMap symbols;
    bool symInited;

private:
    static MMItemName tagNames[MAX_TAG_COUNT];                  // Names of tags
    static MMItemName allocPoolNames[MAX_ALLOC_POOL_COUNT];     // Names of allocation pools
    
    static size_t registeredTagCount;                           // Number of registered tags including predefined
    static size_t registeredAllocPoolCount;                     // Number of registered allocation pools including predefined
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
inline void* MemoryManager::Reallocate(void * ptr, size_t size)
{
    return Instance()->Realloc(ptr, size);
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

inline size_t MemoryManager::CalcStatSize()
{
    return Instance()->CalcStatSizeInternal();
}

inline void MemoryManager::GetStat(MMStat* stat)
{
    Instance()->GetStatInternal(stat);
}

inline size_t MemoryManager::GetDump(size_t userSize, void** buf, uint32 blockRangeBegin, uint32 blockRangeEnd)
{
    return Instance()->GetDumpInternal(userSize, buf, blockRangeBegin, blockRangeEnd);
}

inline void MemoryManager::FreeDump(void* ptr)
{
    Instance()->FreeDumpInternal(ptr);
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
