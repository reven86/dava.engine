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

#include <type_traits>

#include "Thread/Spinlock.h"
#include "Thread/LockGuard.h"
#include "Thread/ThreadLocalPtr.h"
#include "Platform/ConditionalVariable.h"
#include "Platform/Mutex.h"

#include "MemoryManager/MemoryManagerConfig.h"
#include "MemoryManager/AllocPools.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MemoryManager/InternalAllocator.h"

namespace DAVA
{

class Thread;
class BaseObject;

/*
 MemoryManager
*/
class MemoryManager final
{
    static const uint32 BLOCK_MARK = 0xBA0BAB;
    static const uint32 INTERNAL_BLOCK_MARK = 0x55AACC11;
    static const size_t BLOCK_ALIGN = 16;
    static const size_t BACKTRACE_DEPTH = DAVA_MEMORY_MANAGER_BACKTRACE_DEPTH;

public:
    static const uint32 MAX_ALLOC_POOL_COUNT = 24;
    static const size_t MAX_TAG_COUNT = 32;

    struct MemoryBlock;
    struct InternalMemoryBlock;
    struct Backtrace;
    struct AllocScopeItem;

public:
    class AllocPoolScope final
    {
    public:
        explicit AllocPoolScope(int32 aPool) : allocPool(aPool) { MemoryManager::Instance()->EnterAllocScope(allocPool); }
        ~AllocPoolScope() { MemoryManager::Instance()->LeaveAllocScope(allocPool); }
    private:
        int32 allocPool;
    };

public:
    static MemoryManager* Instance();

    static void RegisterAllocPoolName(uint32 index, const char8* name);
    static void RegisterTagName(uint32 tagMask, const char8* name);

    void SetCallbacks(void (*onUpdate)(void*), void (*onTag)(uint32, bool, void*), void* arg);
    void Update();

    DAVA_NOINLINE void* Allocate(size_t size, uint32 poolIndex);
    DAVA_NOINLINE void* AlignedAllocate(size_t size, size_t align, uint32 poolIndex);
    void* Reallocate(void* ptr, size_t newSize);
    void Deallocate(void* ptr);
    
    void* InternalAllocate(size_t size) DAVA_NOEXCEPT;
    void InternalDeallocate(void* ptr) DAVA_NOEXCEPT;

    void EnterTagScope(uint32 tag);
    void LeaveTagScope(uint32 tag);

    void EnterAllocScope(uint32 allocPool);
    void LeaveAllocScope(uint32 allocPool);

    void TrackGpuAlloc(uint32 id, size_t size, uint32 gpuPoolIndex);
    void TrackGpuDealloc(uint32 id, uint32 gpuPoolIndex);

    std::pair<size_t, size_t> BktraceStat() const;

    MMStatConfig* GetStatConfig();
    MMCurStat* GetCurStat();
    MMDump* GetMemoryDump();

    bool GetMemoryDump(FILE* file, size_t& dumpSize);

    void FreeStatMemory(void* ptr);

private:
    // Make construtor and destructor private to disallow external creation of MemoryManager
    MemoryManager();
    ~MemoryManager() = default;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator = (const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator = (MemoryManager&&) = delete;

private:
    void InsertBlock(MemoryBlock* block);
    void RemoveBlock(MemoryBlock* block);

    void UpdateStatAfterAlloc(MemoryBlock* block);
    void UpdateStatAfterDealloc(MemoryBlock* block);

    void InsertBacktrace(Backtrace& backtrace);
    void RemoveBacktrace(uint32 hash);

    DAVA_NOINLINE void CollectBacktrace(Backtrace* backtrace, size_t nskip);
    void ObtainAllBacktraceSymbols();
    void ObtainBacktraceSymbols(const Backtrace* backtrace);

    size_t CalcCurStatSize() const;
    MMCurStat* FillCurStat(void* buffer, size_t size) const;

    template<typename T>
    static size_t BitIndex(T value);
    template<typename T>
    static bool IsPowerOf2(T value);

    void SymbolCollectorThread(BaseObject*, void*, void*);

private:
    MemoryBlock* head = nullptr;                        // Linked list of tracked memory blocks

    GeneralAllocStat statGeneral;                       // General statistics
    AllocPoolStat statAllocPool[MAX_ALLOC_POOL_COUNT];  // Statistics by allocation pools
    TagAllocStat statTag[MAX_TAG_COUNT];                // Statistics by tags

    using MutexType = Spinlock;
    using LockType = LockGuard<MutexType>;

    mutable MutexType allocMutex;       // Mutex for managing list of allocated memory blocks
    mutable MutexType statMutex;        // Mutex for updating memory statistics
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
    mutable MutexType bktraceMutex;     // Mutex for working with backtraces
#endif
#if defined(DAVA_MEMORY_MANAGER_TRACK_GPU)
    mutable MutexType gpuMutex;         // Mutex for managing GPU allocations
#endif

    using InternalString = std::basic_string<char8, std::char_traits<char8>, InternalAllocator<char8>>;

    // Function object for using as hash template in BacktraceMap
    // It simply returns backtrace hash without any modification as hash value has been already calculated using DAVA::HashValue_N
    struct KeyHash
    {
        size_t operator () (const uint32 key) const { return key; }
    };

    using BacktraceMap = std::unordered_map<uint32, Backtrace, KeyHash, std::equal_to<uint32>, InternalAllocator<std::pair<const uint32, Backtrace>>>;
    using SymbolMap = std::unordered_map<void*, InternalString, std::hash<void*>, std::equal_to<void*>, InternalAllocator<std::pair<void* const, InternalString>>>;

    BacktraceMap* bktraceMap = nullptr;
    SymbolMap* symbolMap = nullptr;

    using GpuBlockList = std::list<MemoryBlock, InternalAllocator<MemoryBlock>>;
    using GpuBlockMap = std::unordered_map<int32, GpuBlockList, std::hash<int32>, std::equal_to<int32>, InternalAllocator<std::pair<const int32, GpuBlockList>>>;

    GpuBlockMap* gpuBlockMap = nullptr;

    Thread* symbolCollectorThread = nullptr;
    ConditionalVariable symbolCollectorCondVar;
    Mutex symbolCollectorMutex;
    size_t bktraceGrowDelta = 0;

    void* callbackArg = nullptr;
    void (*updateCallback)(void* arg) = nullptr;
    void (*tagCallback)(uint32 tags, bool entering, void* arg) = nullptr;

private:
    // Make the following data members static to allow initialization of predefined values not in constructor
    static size_t registeredTagCount;                               // Number of registered tags
    static size_t registeredAllocPoolCount;                         // Number of registered allocation pools including predefined

    static MMItemName tagNames[MAX_TAG_COUNT];                // Names of tags
    static MMItemName allocPoolNames[MAX_ALLOC_POOL_COUNT];   // Names of allocation pools

#if defined(DAVA_MEMORY_MANAGER_USE_ALLOC_SCOPE)
    static ThreadLocalPtr<AllocScopeItem> tlsAllocScopeStack;
#endif
};

//////////////////////////////////////////////////////////////////////////
inline size_t MemoryManager::CalcCurStatSize() const
{
    return sizeof(MMCurStat)
         + sizeof(AllocPoolStat) * registeredAllocPoolCount
         + sizeof(TagAllocStat) * registeredTagCount;
}

template<typename T>
inline size_t MemoryManager::BitIndex(T value)
{
    static_assert(std::is_integral<T>::value, "BitIndex works only with integral types");
    size_t index = 0;
    for (value -= 1;value != 0;++index)
    {
        value >>= 1;
    }
    return index;
}

template<typename T>
inline bool MemoryManager::IsPowerOf2(T value)
{
    static_assert(std::is_integral<T>::value, "IsPowerOf2 works only with integral types");
    return (value & (value - 1)) == 0;
}

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MEMORYMANAGER_H__
