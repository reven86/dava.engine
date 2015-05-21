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

#if defined(__DAVAENGINE_WIN32__)
#include <dbghelp.h>
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#endif

#include "Base/Hash.h"
#include "Debug/DVAssert.h"
#include "Platform/Thread.h"

#include "MemoryManager/MallocHook.h"
#include "MemoryManager/MemoryManager.h"

namespace DAVA
{

struct MemoryManager::MemoryBlock
{
    MemoryBlock* prev;      // Pointer to previous block
    MemoryBlock* next;      // Pointer to next block
    void* realBlockStart;   // Pointer to real block start
    void* padding1;         // Padding to make sure that
    uint32 padding2;        //          struct size is integral multiple of 16 bytes
    uint32 orderNo;         // Block order number
    uint32 allocByApp;      // Size requested by application
    uint32 allocTotal;      // Total allocated size
    uint32 bktraceHash;     // Unique hash number to identify block backtrace
    uint32 pool;            // Allocation pool block belongs to
    uint32 tags;            // Tags block belongs to
    uint32 mark;            // Mark to distinguish tracked memory blocks
};
static_assert(sizeof(MemoryManager::MemoryBlock) % 16 == 0, "sizeof(MemoryManager::MemoryBlock) % 16 != 0");

struct MemoryManager::InternalMemoryBlock
{
    uint32 allocByApp;      // Size requested by application
    uint32 allocTotal;      // Total allocated size
    uint32 padding;
    uint32 mark;            // Mark to distinguish internal memory blocks
};
static_assert(sizeof(MemoryManager::InternalMemoryBlock) % 16 == 0, "sizeof(MemoryManager::InternalMemoryBlock) % 16 != 0");

struct MemoryManager::Backtrace
{
    size_t nref;
    uint32 hash;
    bool symbolsCollected;
    void* frames[BACKTRACE_DEPTH];
};

struct MemoryManager::AllocScopeItem
{
    AllocScopeItem* next;
    int32 allocPool;
};

//////////////////////////////////////////////////////////////////////////

MMItemName MemoryManager::tagNames[MAX_TAG_COUNT];

MMItemName MemoryManager::allocPoolNames[MAX_ALLOC_POOL_COUNT] = {
    { "total"          },
    { "default"        },
    { "gpu texture"    },
    { "gpu rdo vertex" },
    { "gpu rdo index"  },
    { "fmod"           },
    { "bullet"         },
    { "base object"    },
    { "polygon group"  },
    { "render dataobj" },
    { "component"      },
    { "entity"         },
    { "landscape"      },
    { "image"          },
    { "texture"        }
};

size_t MemoryManager::registeredTagCount = 0;
size_t MemoryManager::registeredAllocPoolCount = PREDEF_POOL_COUNT;

#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
ThreadLocalPtr<MemoryManager::AllocScopeItem> MemoryManager::tlsAllocScopeStack;
#endif

void MemoryManager::RegisterAllocPoolName(uint32 index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MMItemName::MAX_NAME_LENGTH);
    DVASSERT(FIRST_CUSTOM_ALLOC_POOL <= index && index < MAX_ALLOC_POOL_COUNT);
    DVASSERT(allocPoolNames[index - 1].name[0] != '\0');     // Names should be registered sequentially with no gap

    strncpy(allocPoolNames[index].name, name, MMItemName::MAX_NAME_LENGTH);
    allocPoolNames[index].name[MMItemName::MAX_NAME_LENGTH - 1] = '\0';
    registeredAllocPoolCount += 1;
}

void MemoryManager::RegisterTagName(uint32 tagMask, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MMItemName::MAX_NAME_LENGTH);
    DVASSERT(tagMask != 0 && IsPowerOf2(tagMask));

    const size_t index = BitIndex(tagMask);
    DVASSERT(index < MAX_TAG_COUNT);
    DVASSERT(0 == index || tagNames[index - 1].name[0] != '\0');     // Names should be registered sequentially with no gap

    strncpy(tagNames[index].name, name, MMItemName::MAX_NAME_LENGTH);
    tagNames[index].name[MMItemName::MAX_NAME_LENGTH - 1] = '\0';
    registeredTagCount += 1;
}

//////////////////////////////////////////////////////////////////////////

MemoryManager* MemoryManager::Instance()
{
    static MallocHook hook;
    static MemoryManager mm;
    return &mm;
}

void MemoryManager::SetCallbacks(void (*onUpdate)(void*), void (*onTag)(uint32, bool, void*), void* arg)
{
    updateCallback = onUpdate;
    tagCallback = onTag;
    callbackArg = arg;
}

void MemoryManager::Update()
{
    if (nullptr == symbolCollectorThread)
    {
        //symbolCollectorThread = Thread::Create();
    }

    if (updateCallback != nullptr)
    {
        updateCallback(callbackArg);
    }
}

DAVA_NOINLINE void* MemoryManager::Allocate(size_t size, uint32 poolIndex)
{
    assert(ALLOC_POOL_TOTAL < poolIndex && poolIndex < MAX_ALLOC_POOL_COUNT);

    // On zero-sized allocation request allocate 1 byte to return unique memory block
    size_t totalSize = sizeof(MemoryBlock) + (size != 0 ? size : 1);
    if (totalSize & (BLOCK_ALIGN - 1))
    {
        totalSize += (BLOCK_ALIGN - (totalSize & (BLOCK_ALIGN - 1)));
    }

    MemoryBlock* block = static_cast<MemoryBlock*>(MallocHook::Malloc(totalSize));
    if (block != nullptr)
    {
#if defined(DAVA_MEMORY_MANAGER_USE_ALLOC_SCOPE)
        if (tlsAllocScopeStack.IsCreated())
        {
            AllocScopeItem* scopeItem = tlsAllocScopeStack.Get();
            if (scopeItem != nullptr)
            {
                block->pool = scopeItem->allocPool;
            }
        }
#endif

#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
        Backtrace backtrace;
        CollectBacktrace(&backtrace, 1);
#endif

        block->mark = BLOCK_MARK;
        block->pool = poolIndex;
        block->realBlockStart = static_cast<void*>(block);
        block->allocByApp = static_cast<uint32>(size);
        block->allocTotal = static_cast<uint32>(MallocHook::MallocSize(block->realBlockStart));
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
        block->bktraceHash = backtrace.hash;
#else
        block->bktraceHash = 0;
#endif

        {
            LockType lock(allocMutex);
            block->tags = statGeneral.activeTags;
            block->orderNo = statGeneral.nextBlockNo++;
            InsertBlock(block);
        }
        {
            LockType lock(statMutex);
            UpdateStatAfterAlloc(block);
        }
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
        {
            LockType lock(bktraceMutex);
            InsertBacktrace(backtrace);
        }
#endif
        return static_cast<void*>(block + 1);
    }
    return nullptr;
}

DAVA_NOINLINE void* MemoryManager::AlignedAllocate(size_t size, size_t align, uint32 poolIndex)
{
    // TODO: check whether size is integral multiple of align
    assert(size > 0);
    assert(align > 0 && 0 == (align & (align - 1)));    // Check whether align is power of 2
    assert(ALLOC_POOL_TOTAL < poolIndex && poolIndex < MAX_ALLOC_POOL_COUNT);

    if (align < BLOCK_ALIGN)
    {
        align = BLOCK_ALIGN;
    }

    size_t totalSize = sizeof(MemoryBlock) + size + (align - 1);
    if (totalSize & (BLOCK_ALIGN - 1))
    {
        totalSize += (BLOCK_ALIGN - (totalSize & (BLOCK_ALIGN - 1)));
    }

    void* realPtr = MallocHook::Malloc(totalSize);
    if (realPtr != nullptr)
    {
#if defined(DAVA_MEMORY_MANAGER_USE_ALLOC_SCOPE)
        if (tlsAllocScopeStack.IsCreated())
        {
            AllocScopeItem* scopeItem = tlsAllocScopeStack.Get();
            if (scopeItem != nullptr)
            {
                block->pool = scopeItem->allocPool;
            }
        }
#endif

        // Some pointer arithmetics
        uintptr_t aligned = uintptr_t(realPtr) + sizeof(MemoryBlock);
        aligned += align - (aligned & (align - 1));

#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
        Backtrace backtrace;
        CollectBacktrace(&backtrace, 1);
#endif

        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(aligned - sizeof(MemoryBlock));
        block->mark = BLOCK_MARK;
        block->pool = poolIndex;
        block->realBlockStart = realPtr;
        block->allocByApp = static_cast<uint32>(size);
        block->allocTotal = static_cast<uint32>(MallocHook::MallocSize(block->realBlockStart));
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
        block->bktraceHash = backtrace.hash;
#else
        block->bktraceHash = 0;
#endif

        {
            LockType lock(allocMutex);
            block->tags = statGeneral.activeTags;
            block->orderNo = statGeneral.nextBlockNo++;
            InsertBlock(block);
        }
        {
            LockType lock(statMutex);
            UpdateStatAfterAlloc(block);
        }
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
        {
            LockType lock(bktraceMutex);
            InsertBacktrace(backtrace);
        }
#endif
        return reinterpret_cast<void*>(aligned);
    }
    return nullptr;
}

void* MemoryManager::Reallocate(void* ptr, size_t newSize)
{
    if (nullptr == ptr)
    {
        return malloc(newSize);
    }
    
    MemoryBlock* block = static_cast<MemoryBlock*>(ptr) - 1;
    if (BLOCK_MARK == block->mark)
    {
        void* newPtr = malloc(newSize);
        if (newPtr != nullptr)
        {
            size_t n = block->allocByApp > newSize ? newSize : block->allocByApp;
            memcpy(newPtr, ptr, n);
            free(ptr);
            return newPtr;
        }
        else
            return nullptr;
    }
    else
    {
        return MallocHook::Realloc(ptr, newSize);
    }
}
    
void MemoryManager::Deallocate(void* ptr)
{
    if (ptr != nullptr)
    {
        void* ptrToFree = nullptr;
        MemoryBlock* block = static_cast<MemoryBlock*>(ptr) - 1;
        if (BLOCK_MARK == block->mark)
        {
            {
                LockType lock(allocMutex);
                RemoveBlock(block);
            }
            {
                LockType lock(statMutex);
                UpdateStatAfterDealloc(block);
            }
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
            {
                LockType lock(bktraceMutex);
                RemoveBacktrace(block->bktraceHash);
            }
#endif
            // Tracked memory block consists of header (of type struct MemoryBlock) and data block that returned to app.
            // Tracked memory blocks are distinguished by special mark in header.
            // In some cases, especially on iOS, memory allocations bypass memory manager, but memory freeing goes through
            // memory manager. So I need to erase header to properly free bypassed allocations as system can allocate memory at the same address
            block->mark = 0xECECECEC;
            ptrToFree = block->realBlockStart;
        }
        else
        {
            LockType lock(statMutex);
            statGeneral.ghostBlockCount += 1;
            statGeneral.ghostSize += MallocHook::MallocSize(ptr);
            ptrToFree = ptr;
        }
        MallocHook::Free(ptrToFree);
    }
}

void* MemoryManager::InternalAllocate(size_t size) DAVA_NOEXCEPT
{
    const size_t totalSize = sizeof(InternalMemoryBlock) + size;
    InternalMemoryBlock* block = static_cast<InternalMemoryBlock*>(MallocHook::Malloc(totalSize));
    if (block != nullptr)
    {
        block->mark = INTERNAL_BLOCK_MARK;
        block->allocByApp = static_cast<uint32>(size);
        block->allocTotal = static_cast<uint32>(MallocHook::MallocSize(block));

        // Update stat
        {
            LockType lock(statMutex);
            statGeneral.allocInternal += block->allocByApp;
            statGeneral.allocInternalTotal += block->allocTotal;
            statGeneral.internalBlockCount += 1;
        }
        return static_cast<void*>(block + 1);
    }
    return nullptr;
}

void MemoryManager::InternalDeallocate(void* ptr) DAVA_NOEXCEPT
{
    if (ptr != nullptr)
    {
        InternalMemoryBlock* block = static_cast<InternalMemoryBlock*>(ptr) - 1;
        assert(INTERNAL_BLOCK_MARK == block->mark);
        
        // Update stat
        {
            LockType lock(statMutex);
            statGeneral.allocInternal -= block->allocByApp;
            statGeneral.allocInternalTotal -= block->allocTotal;
            statGeneral.internalBlockCount -= 1;
        }

        // Clear mark of deallocated block
        block->mark = 0xECECECEC;
        MallocHook::Free(block);
    }
}

void MemoryManager::EnterTagScope(uint32 tag)
{
    DVASSERT(tag != 0 && IsPowerOf2(tag));
    DVASSERT((statGeneral.activeTags & tag) == 0);  // Tag shouldn't be set earlier

    {
        LockType lock(allocMutex);
        statGeneral.activeTags |= tag;
        statGeneral.activeTagCount += 1;
    }
    if (tagCallback != nullptr)
    {
        tagCallback(tag, true, callbackArg);
    }
}

void MemoryManager::LeaveTagScope(uint32 tag)
{
    DVASSERT(tag != 0 && IsPowerOf2(tag));
    DVASSERT((statGeneral.activeTags & tag) == tag);    // Tag should be set earlier

    {
        LockType lock(allocMutex);
        statGeneral.activeTags &= ~tag;
        statGeneral.activeTagCount -= 1;
    }
    if (tagCallback != nullptr)
    {
        tagCallback(tag, false, callbackArg);
    }
}

void MemoryManager::EnterAllocScope(uint32 allocPool)
{
#if defined(DAVA_MEMORY_MANAGER_USE_ALLOC_SCOPE)
    AllocScopeItem* newItem = new AllocScopeItem;
    newItem->next = tlsAllocScopeStack.Release();
    newItem->allocPool = allocPool;

    tlsAllocScopeStack.Reset(newItem);
#endif
}

void MemoryManager::LeaveAllocScope(uint32 allocPool)
{
#if defined(DAVA_MEMORY_MANAGER_USE_ALLOC_SCOPE)
    AllocScopeItem* topItem = tlsAllocScopeStack.Release();
    assert(topItem != nullptr);
    assert(topItem->allocPool == allocPool);

    tlsAllocScopeStack.Reset(topItem->next);
    delete topItem;
#endif
}

void MemoryManager::TrackGpuAlloc(uint32 id, size_t size, uint32 gpuPoolIndex)
{
#if defined(DAVA_MEMORY_MANAGER_TRACK_GPU)
    LockType lock(gpuMutex);

    if (nullptr == gpuBlockMap)
    {
        static GpuBlockMap object;
        gpuBlockMap = &object;
    }

    auto iterToList = gpuBlockMap->find(gpuPoolIndex);
    if (iterToList == gpuBlockMap->end())
    {
        auto pair = gpuBlockMap->emplace(gpuPoolIndex, GpuBlockList());
        iterToList = pair.first;
    }

    GpuBlockList& blockList = iterToList->second;
    auto iterToBlock = std::find_if(blockList.begin(), blockList.end(), [id](const MemoryBlock& block) -> bool {
        return block.orderNo == id;
    });

    if (iterToBlock == blockList.end())
    {
        MemoryBlock newBlock{};
        newBlock.orderNo = id;
        newBlock.pool = gpuPoolIndex;

        blockList.emplace_back(newBlock);
        iterToBlock = --blockList.end();
    }

    MemoryBlock& block = *iterToBlock;
    block.allocByApp += static_cast<uint32>(size);
    block.allocTotal  = block.allocByApp;

    {
        LockType lock(mutex);
        {   // Update total statistics
            statAllocPool[ALLOC_POOL_TOTAL].allocByApp += static_cast<uint32>(size);
            statAllocPool[ALLOC_POOL_TOTAL].allocTotal += static_cast<uint32>(size);

            if (block.allocByApp > statAllocPool[ALLOC_POOL_TOTAL].maxBlockSize)
                statAllocPool[ALLOC_POOL_TOTAL].maxBlockSize = block.allocByApp;
        }
        {   // Update pool statistics
            statAllocPool[gpuPoolIndex].allocByApp += static_cast<uint32>(size);
            statAllocPool[gpuPoolIndex].allocTotal += static_cast<uint32>(size);
            statAllocPool[gpuPoolIndex].blockCount += 1;

            if (block.allocByApp > statAllocPool[gpuPoolIndex].maxBlockSize)
                statAllocPool[gpuPoolIndex].maxBlockSize = block.allocByApp;
        }
    }
#endif
}

void MemoryManager::TrackGpuDealloc(uint32 id, uint32 gpuPoolIndex)
{
#if defined(DAVA_MEMORY_MANAGER_TRACK_GPU)
    LockType lock(gpuMutex);

    auto iterToList = gpuBlockMap->find(gpuPoolIndex);
    DVASSERT(iterToList != gpuBlockMap->end());

    GpuBlockList& blockList = iterToList->second;
    auto iterToBlock = std::find_if(blockList.begin(), blockList.end(), [id](const MemoryBlock& block) -> bool {
        return block.orderNo == id;
    });
    DVASSERT(iterToBlock != blockList.end());

    MemoryBlock& block = *iterToBlock;
    {
        LockType lock(mutex);
        {   // Update total statistics
            statAllocPool[ALLOC_POOL_TOTAL].allocByApp -= block.allocByApp;
            statAllocPool[ALLOC_POOL_TOTAL].allocTotal -= block.allocTotal;
        }
        {   // Update pool statistics
            statAllocPool[gpuPoolIndex].allocByApp -= block.allocByApp;
            statAllocPool[gpuPoolIndex].allocTotal -= block.allocTotal;
            statAllocPool[gpuPoolIndex].blockCount -= 1;
        }
    }

    blockList.erase(iterToBlock);
#endif
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

void MemoryManager::UpdateStatAfterAlloc(MemoryBlock* block)
{
    {   // Update total statistics
        statAllocPool[ALLOC_POOL_TOTAL].allocByApp += block->allocByApp;
        statAllocPool[ALLOC_POOL_TOTAL].allocTotal += block->allocTotal;
        statAllocPool[ALLOC_POOL_TOTAL].blockCount += 1;

        if (block->allocByApp > statAllocPool[ALLOC_POOL_TOTAL].maxBlockSize)
            statAllocPool[ALLOC_POOL_TOTAL].maxBlockSize = block->allocByApp;
    }
    {   // Update pool statistics
        const uint32 poolIndex = block->pool;
        statAllocPool[poolIndex].allocByApp += block->allocByApp;
        statAllocPool[poolIndex].allocTotal += block->allocTotal;
        statAllocPool[poolIndex].blockCount += 1;

        if (block->allocByApp > statAllocPool[poolIndex].maxBlockSize)
            statAllocPool[poolIndex].maxBlockSize = block->allocByApp;
    }

    {   // Update tag statistics
        uint32 tags = statGeneral.activeTags;
        for (size_t index = 0;tags != 0;++index, tags >>= 1)
        {
            if (tags & 0x01)
            {
                statTag[index].allocByApp += block->allocByApp;
                statTag[index].blockCount += 1;
            }
        }
    }
}

void MemoryManager::UpdateStatAfterDealloc(MemoryBlock* block)
{
    {   // Update total statistics
        statAllocPool[ALLOC_POOL_TOTAL].allocByApp -= block->allocByApp;
        statAllocPool[ALLOC_POOL_TOTAL].allocTotal -= block->allocTotal;
        statAllocPool[ALLOC_POOL_TOTAL].blockCount -= 1;
    }
    {   // Update pool statistics
        const uint32 poolIndex = block->pool;
        statAllocPool[poolIndex].allocByApp -= block->allocByApp;
        statAllocPool[poolIndex].allocTotal -= block->allocTotal;
        statAllocPool[poolIndex].blockCount -= 1;
    }
    {   // Update tag statistics
        uint32 tags = block->tags;
        for (size_t index = 0;tags != 0;++index, tags >>= 1)
        {
            if (tags & 0x01)
            {
                statTag[index].allocByApp -= block->allocByApp;
                statTag[index].blockCount -= 1;
            }
        }
    }
}

void MemoryManager::InsertBacktrace(Backtrace& backtrace)
{
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
    if (nullptr == bktraceMap)
    {
        static uint8 bufferForMap[sizeof(BacktraceMap)];
        static uint8 bufferForStorage[sizeof(BacktraceStorage)];

        bktraceMap = new (bufferForMap) BacktraceMap;
        bktraceStorage = new (bufferForStorage) BacktraceStorage;
    }

    auto i = bktraceMap->find(backtrace.hash);
    if (i == bktraceMap->end())
    {
        const size_t index = bktraceStorage->size();

        Backtrace* p = new(InternalAllocate(sizeof(Backtrace))) Backtrace(backtrace);
        bktraceStorage->emplace_back(p);
        //bktraceStorage->emplace_back(backtrace);
        bktraceMap->emplace(backtrace.hash, index);
    }
    else
    {
        const size_t index = i->second;
        Backtrace* p = (*bktraceStorage)[index];
        if (p == nullptr)
        {
            Backtrace* p = new(InternalAllocate(sizeof(Backtrace))) Backtrace(backtrace);
            (*bktraceStorage)[index] = p;
        }
    }
#endif
}

void MemoryManager::RemoveBacktrace(uint32 hash)
{
#if defined(DAVA_MEMORY_MANAGER_COLLECT_BACKTRACES)
    // do nothing
    auto i = bktraceMap->find(hash);
    const size_t index = i->second;

    Backtrace* p = (*bktraceStorage)[index];
    (*bktraceStorage)[index] = nullptr;

    InternalDeallocate(p);
#endif
}

#if defined(__DAVAENGINE_ANDROID__)
namespace
{

struct BacktraceState
{
    void** current;
    void** end;
};

_Unwind_Reason_Code UnwindCallback(struct _Unwind_Context* context, void* arg)
{
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc != 0)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

}   // unnamed namespace
#endif

DAVA_NOINLINE void MemoryManager::CollectBacktrace(Backtrace* backtrace, size_t nskip)
{
    const size_t EXTRA_FRAMES = 5;
    void* frames[BACKTRACE_DEPTH + EXTRA_FRAMES] = {nullptr};
    Memset(backtrace, 0, sizeof(Backtrace));
    
#if defined(__DAVAENGINE_WIN32__)
    CaptureStackBackTrace(0, COUNT_OF(frames), frames, nullptr);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    ::backtrace(frames, COUNT_OF(frames));
#elif defined(__DAVAENGINE_ANDROID__)
    BacktraceState state = {frames, frames + COUNT_OF(frames)};
    _Unwind_Backtrace(&UnwindCallback, &state);
#endif
    for (size_t idst = 0, isrc = nskip + 1;idst < BACKTRACE_DEPTH;++idst, ++isrc)
    {
        backtrace->frames[idst] = frames[isrc];
    }
    backtrace->hash = HashValue_N(reinterpret_cast<const char*>(backtrace->frames), sizeof(backtrace->frames));
    backtrace->nref = 1;
    backtrace->symbolsCollected = false;
}

void MemoryManager::ObtainAllBacktraceSymbols()
{
    for (auto& x : *bktraceStorage)
    {
        //if (!x.symbolsCollected)
        //{
        //    ObtainBacktraceSymbols(&x);
        //    x.symbolsCollected = true;
        //}
    }
}

void MemoryManager::ObtainBacktraceSymbols(const Backtrace* backtrace)
{
    if (nullptr == symbolMap)
    {
        static uint8 bufferFoMap[sizeof(SymbolMap)];
        static uint8 bufferForStorage[sizeof(SymbolStorage)];

        symbolMap = new (bufferFoMap) SymbolMap;
        symbolStorage = new (bufferForStorage) SymbolStorage;
    }

    const size_t NAME_BUFFER_SIZE = 4 * 1024;
    static char8 nameBuffer[NAME_BUFFER_SIZE];
    
#if defined(__DAVAENGINE_WIN32__)
    HANDLE hprocess = GetCurrentProcess();

    static bool symInited = false;
    if (!symInited)
    {
        SymInitialize(hprocess, nullptr, TRUE);
        symInited = true;
    }

    SYMBOL_INFO* symInfo = reinterpret_cast<SYMBOL_INFO*>(nameBuffer);
    symInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    symInfo->MaxNameLen = NAME_BUFFER_SIZE - sizeof(SYMBOL_INFO);

    for (size_t i = 0;i < COUNT_OF(backtrace->frames);++i)
    {
        if (backtrace->frames[i] != nullptr && symbolMap->find(backtrace->frames[i]) == symbolMap->cend())
        {
            if (SymFromAddr(hprocess, reinterpret_cast<DWORD64>(backtrace->frames[i]), 0, symInfo))
            {
                const size_t index = symbolStorage->size();
                symbolStorage->emplace_back(InternalString(symInfo->Name));
                symbolMap->emplace(backtrace->frames[i], index);
            }
        }
    }
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    for (size_t i = 0;i < COUNT_OF(backtrace->frames);++i)
    {
        if (backtrace->frames[i] != nullptr && symbolMap->find(backtrace->frames[i]) == symbolMap->cend())
        {
            /*
             https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/dladdr.3.html#//apple_ref/doc/man/3/dladdr
             If an image containing addr cannot be found, dladdr() returns 0.  On success, a non-zero value is returned.
             If the image containing addr is found, but no nearest symbol was found, the dli_sname and dli_saddr fields are set to NULL.
            */
            Dl_info dlinfo;
            if (dladdr(backtrace->frames[i], &dlinfo) != 0 && dlinfo.dli_sname != nullptr)
            {
                int status = 0;
                size_t n = COUNT_OF(nameBuffer);
                abi::__cxa_demangle(dlinfo.dli_sname, nameBuffer, &n, &status);
                const size_t index = symbolStorage->size();
                symbolStorage->emplace_back(0 == status ? InternalString(nameBuffer) : InternalString(dlinfo.dli_sname));
                symbolMap->emplace(backtrace->frames[i], index);
            }
        }
    }
#endif
}

MMStatConfig* MemoryManager::GetStatConfig() const
{
    LockType lock(allocMutex);

    const size_t size = sizeof(MMStatConfig)
                      + sizeof(MMItemName) * registeredAllocPoolCount
                      + sizeof(MMItemName) * registeredTagCount;

    MMStatConfig* config = static_cast<MMStatConfig*>(MallocHook::Malloc(size));
    if (config != nullptr)
    {
        config->size = static_cast<uint32>(size);
        config->allocPoolCount = static_cast<uint32>(registeredAllocPoolCount);
        config->tagCount = static_cast<uint32>(registeredTagCount);
        config->bktraceDepth = BACKTRACE_DEPTH;

        MMItemName* names = OffsetPointer<MMItemName>(config, sizeof(MMStatConfig));
        for (size_t i = 0;i < registeredAllocPoolCount;++i, ++names)
        {
            *names = allocPoolNames[i];
        }
        for (size_t i = 0;i < registeredTagCount;++i, ++names)
        {
            *names = tagNames[i];
        }
    }
    return config;
}

MMCurStat* MemoryManager::GetCurStat() const
{
    LockType lock(allocMutex);

    const size_t size = CalcCurStatSize();
    void* buffer = MallocHook::Malloc(size);
    return FillCurStat(buffer, size);
}

MMDump* MemoryManager::GetMemoryDump()
{
    return nullptr;
#if 0
    ObtainAllBacktraceSymbols();

    LockType lock(allocMutex);

#if defined(DAVA_MEMORY_MANAGER_NEW_DATASTRUCT)
    const size_t blockCount = statAllocPool[ALLOC_POOL_TOTAL].blockCount;
    const size_t symbolCount = symbolStorage->size();
    const size_t bktraceCount = bktraceStorage->size();
#else
    const size_t blockCount = statAllocPool[ALLOC_POOL_TOTAL].blockCount;
    const size_t symbolCount = symbols->size();
    const size_t bktraceCount = backtraces->size();
#endif

    const size_t statSize = CalcCurStatSize();
    const size_t bktraceSize = sizeof(MMBacktrace) + sizeof(uint64) * BACKTRACE_DEPTH;
    const size_t size = sizeof(MMDump)
                      + statSize
                      + sizeof(MMBlock) * blockCount
                      + sizeof(MMSymbol) * symbolCount
                      + bktraceSize * bktraceCount;

    MMDump* dump = static_cast<MMDump*>(MallocHook::Malloc(size));
    if (dump != nullptr)
    {
        dump->timestamp = 0;
        dump->collectTime = 0;
        dump->packTime = 0;
        dump->size = static_cast<uint32>(size);
        dump->blockCount = static_cast<uint32>(blockCount);
        dump->bktraceCount = static_cast<uint32>(bktraceCount);
        dump->symbolCount = static_cast<uint32>(symbolCount);
        dump->bktraceDepth = BACKTRACE_DEPTH;

        void* statBuf = OffsetPointer<void>(dump, sizeof(MMDump));
        FillCurStat(statBuf, statSize);

        MemoryBlock* curBlock = head;
        MMBlock* blocks = OffsetPointer<MMBlock>(statBuf, statSize);
        for (size_t i = 0, n = dump->blockCount;i < n;++i)
        {
            blocks[i].orderNo = curBlock->orderNo;
            blocks[i].allocByApp = curBlock->allocByApp;
            blocks[i].allocTotal = curBlock->allocTotal;
            blocks[i].bktraceHash = curBlock->bktraceHash;
            blocks[i].pool = curBlock->pool;
            blocks[i].tags = curBlock->tags;

            curBlock = curBlock->next;
        }

        MMSymbol* symbol = OffsetPointer<MMSymbol>(blocks, sizeof(MMBlock) * blockCount);
#if defined(DAVA_MEMORY_MANAGER_NEW_DATASTRUCT)
        for (auto& pair : *symbolMap)
        {
            void* addr = pair.first;
            auto& name = (*symbolStorage)[pair.second];

            symbol->addr = reinterpret_cast<uint64>(addr);
            strncpy(symbol->name, name.c_str(), MMSymbol::NAME_LENGTH);
            symbol->name[MMSymbol::NAME_LENGTH - 1] = '\0';

            symbol += 1;
        }
#else
        for (auto& pair : *symbols)
        {
            void* addr = pair.first;
            auto& name = pair.second;

            symbol->addr = reinterpret_cast<uint64>(addr);
            strncpy(symbol->name, name.c_str(), MMSymbol::NAME_LENGTH);
            symbol->name[MMSymbol::NAME_LENGTH - 1] = '\0';

            symbol += 1;
        }
#endif

        MMBacktrace* bktrace = reinterpret_cast<MMBacktrace*>(symbol);
#if defined(DAVA_MEMORY_MANAGER_NEW_DATASTRUCT)
        for (auto& curBktrace : *bktraceStorage)
        {
            bktrace->hash = curBktrace.hash;
            uint64* frames = OffsetPointer<uint64>(bktrace, sizeof(MMBacktrace));
            for (size_t i = 0;i < BACKTRACE_DEPTH;++i)
            {
                frames[i] = reinterpret_cast<uint64>(curBktrace.frames[i]);
            }
            bktrace = OffsetPointer<MMBacktrace>(bktrace, bktraceSize);
        }
#else
        for (auto& pair : *backtraces)
        {
            auto& o = pair.second;

            bktrace->hash = o.hash;
            uint64* frames = OffsetPointer<uint64>(bktrace, sizeof(MMBacktrace));
            for (size_t i = 0;i < BACKTRACE_DEPTH;++i)
            {
                frames[i] = reinterpret_cast<uint64>(o.frames[i]);
            }
            bktrace = OffsetPointer<MMBacktrace>(bktrace, bktraceSize);
        }
#endif
    }
    return dump;
#endif
}

void MemoryManager::FreeStatMemory(void* ptr) const
{
    MallocHook::Free(ptr);
}

MMCurStat* MemoryManager::FillCurStat(void* buffer, size_t size) const
{
    MMCurStat* curStat = static_cast<MMCurStat*>(buffer);
    curStat->timestamp = 0;
    curStat->size = static_cast<uint32>(size);
    curStat->statGeneral = statGeneral;

    AllocPoolStat* pools = OffsetPointer<AllocPoolStat>(curStat, sizeof(MMCurStat));
    for (size_t i = 0;i < registeredAllocPoolCount;++i)
    {
        pools[i] = statAllocPool[i];
    }

    TagAllocStat* tags = OffsetPointer<TagAllocStat>(pools, sizeof(AllocPoolStat) * registeredAllocPoolCount);
    for (size_t i = 0;i < registeredTagCount;++i)
    {
        tags[i] = statTag[i];
    }
    return curStat;
}

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
