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
#include <unordered_map>
#include <unordered_set>

#include "Thread/Spinlock.h"
#include "Thread/LockGuard.h"

#include "AllocPools.h"
#include "MemoryManagerTypes.h"

namespace DAVA
{

/*
 MemoryManager
*/
class MemoryManager final
{
    static const uint32 BLOCK_MARK = 0xBA0BAB;
    static const size_t BLOCK_ALIGN = 16;

    struct MemoryBlock;
    struct Backtrace
    {
        size_t nref;
        size_t hash;
        size_t depth;
        bool symbolsCollected;
        void* frames[MMConst::BACKTRACE_DEPTH];
    };

public:
    typedef void(*DumpRequestCallback)(void* arg, int32 type, uint32 tagOrCheckpoint, uint32 blockBegin, uint32 blockEnd);

public:
    static MemoryManager* Instance();

    static void RegisterAllocPoolName(size_t index, const char8* name);
    static void RegisterTagName(size_t index, const char8* name);
    static void RegisterLabelName(size_t index, const char8* name);

    static void InstallDumpCallback(DumpRequestCallback callback, void* arg);

    DAVA_NOINLINE void* Allocate(size_t size, size_t poolIndex);
    DAVA_NOINLINE void* AlignedAllocate(size_t size, size_t align, size_t poolIndex);
    void* Reallocate(void * ptr, size_t size);
    void Deallocate(void* ptr);

    void EnterTagScope(uint32 tag);
    void LeaveTagScope(uint32 tag);
    void Checkpoint(uint32 checkpoint);

    size_t CalcStatConfigSize() const;
    void GetStatConfig(MMStatConfig* config) const;

    size_t CalcStatSize() const;
    void GetStat(MMStat* stat) const;

    size_t GetDump(size_t userSize, void** buf, uint32 blockRangeBegin, uint32 blockRangeEnd);
    void FreeDump(void* ptr);

private:
    // Make construtor and destructor private to disallow external creation of MemoryManager
    MemoryManager() = default;
    ~MemoryManager() = default;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator = (const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator = (MemoryManager&&) = delete;

private:
    static bool IsInternalAllocationPool(size_t poolIndex);

    void InsertBlock(MemoryBlock* block);
    void RemoveBlock(MemoryBlock* block);
    MemoryBlock* IsTrackedBlock(void* ptr);
    MemoryBlock* FindBlockByOrderNo(uint32 orderNo);

    void UpdateStatAfterAlloc(MemoryBlock* block, size_t poolIndex);
    void UpdateStatAfterDealloc(MemoryBlock* block, size_t poolIndex);

    void InsertBacktrace(Backtrace& backtrace);
    void RemoveBacktrace(size_t hash);
    
    size_t GetBlockRange(uint32 rangeBegin, uint32 rangeEnd, MemoryBlock** begin, MemoryBlock** end);

    DAVA_NOINLINE void CollectBacktrace(Backtrace* backtrace, size_t nskip);
    void ObtainBacktraceSymbols(const Backtrace* backtrace);
    void ObtainAllBacktraceSymbols();

private:
    MemoryBlock* head;                  // Linked list of memory blocks
    uint32 nextBlockNo;                 // Next assigned number to next allocated memory block
    MMTagStack tags;                    // Active tags
    GeneralAllocStat statGeneral;       // General statistics
    AllocPoolStat statAllocPool[MMConst::MAX_TAG_DEPTH][MMConst::MAX_ALLOC_POOL_COUNT];    // Statistics for each allocation pool divided by tags

    using MutexType = Spinlock;
    using LockType = LockGuard<MutexType>;
    mutable MutexType mutex;
    mutable MutexType backtraceMutex;
    
    DumpRequestCallback dumpCallback;   // Callback that called on checkpoint or tag leave
    void* callbackArg;

    template<typename T>
    using InternalAllocator = MemoryManagerAllocator<T, size_t(-1)>;

    using InternalString = std::basic_string<char8, std::char_traits<char8>, InternalAllocator<char8>>;
    using SymbolMap = std::unordered_map<void*, InternalString, std::hash<void*>, std::equal_to<void*>, InternalAllocator<std::pair<void* const, InternalString>>>;
    struct KeyHash { size_t operator () (const size_t key) const { return key; } };
    using BacktraceMap = std::unordered_map<size_t, Backtrace, KeyHash, std::equal_to<size_t>, InternalAllocator<std::pair<const size_t, Backtrace>>>;

    // Room for symbol table map and backtrace set for placement new
    // Use std::aligned_storage<...>::type as std::aligned_storage_t<...> doesn't work on Android
    std::aligned_storage<sizeof(BacktraceMap), DAVA_ALIGNOF(BacktraceMap)>::type backtraceStorage;
    std::aligned_storage<sizeof(SymbolMap), DAVA_ALIGNOF(SymbolMap)>::type symbolStorage;
    BacktraceMap* backtraces;
    SymbolMap* symbols;
#if defined(__DAVAENGINE_WINDOWS__)
    bool symInited;     // Flag indicating that SymInitialize has been called on Win32
#endif

private:
    static MMItemName tagNames[MMConst::MAX_TAG_COUNT];                  // Names of tags
    static MMItemName allocPoolNames[MMConst::MAX_ALLOC_POOL_COUNT];     // Names of allocation pools

    static size_t registeredTagCount;                           // Number of registered tags including predefined
    static size_t registeredAllocPoolCount;                     // Number of registered allocation pools including predefined
};

//////////////////////////////////////////////////////////////////////////
inline bool MemoryManager::IsInternalAllocationPool(size_t poolIndex)
{
    return static_cast<size_t>(-1) == poolIndex;
}

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MEMORYMANAGER_H__
