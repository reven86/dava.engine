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

#if defined(__DAVAENGINE_WINDOWS__)
#include <dbghelp.h>
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#elif defined(__DAVAENGINE_ANDROID__)
#endif

#include "Base/Hash.h"
#include "Debug/DVAssert.h"

#include "MallocHook.h"
#include "MemoryManager.h"

namespace DAVA
{

struct MemoryManager::MemoryBlock
{
    MemoryBlock* prev;      // Pointer to previous block
    MemoryBlock* next;      // Pointer to next block
    void* realBlockStart;   // Pointer to real block start
    size_t backtraceHash;   // Unique hash number to identify block backtrace
    size_t allocByApp;      // Size requested by application
    size_t allocTotal;      // Total allocated size
    size_t orderNo;         // Block order number
    size_t pool;            // Allocation pool block belongs to
    size_t mark;            // Mark to distinguish tracked memory blocks
    size_t padding[3];      // Padding to make sure that struct size is integral multiple of 16 bytes
};

MMItemName MemoryManager::tagNames[MMConst::MAX_TAG_COUNT] = {
    { "application" }
};

MMItemName MemoryManager::allocPoolNames[MMConst::MAX_ALLOC_POOL_COUNT] = {
    { "application" },
    { "FMOD" }, 
    { "RENDERBATCH" }, 
    { "COMPONENT" },
    { "ENTITY" }
};

size_t MemoryManager::registeredTagCount = 1;
size_t MemoryManager::registeredAllocPoolCount = ePredefAllocPools::PREDEF_POOL_COUNT;

void MemoryManager::RegisterAllocPoolName(size_t index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MMConst::MAX_NAME_LENGTH);
    DVASSERT(FIRST_CUSTOM_ALLOC_POOL <= index && index < MMConst::MAX_ALLOC_POOL_COUNT);
    DVASSERT(allocPoolNames[index - 1].name[0] != '\0');     // Names should be registered sequentially with no gap

    strncpy(allocPoolNames[index].name, name, MMConst::MAX_NAME_LENGTH);
    allocPoolNames[index].name[MMConst::MAX_NAME_LENGTH - 1] = '\0';
    registeredAllocPoolCount += 1;
}

void MemoryManager::RegisterTagName(size_t index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MMConst::MAX_NAME_LENGTH);
    DVASSERT(MMConst::DEFAULT_TAG != index && index < MMConst::MAX_TAG_COUNT);
    DVASSERT(tagNames[index - 1].name[0] != '\0');       // Names should be registered sequentially with no gap

    strncpy(tagNames[index].name, name, MMConst::MAX_NAME_LENGTH);
    tagNames[index].name[MMConst::MAX_NAME_LENGTH - 1] = '\0';
    registeredTagCount += 1;
}

MemoryManager* MemoryManager::Instance()
{
    static_assert(sizeof(MemoryManager::MemoryBlock) % 16 == 0, "sizeof(MemoryManager::MemoryBlock) % 16 == 0");
    static MallocHook hook;
    static MemoryManager mm;
    return &mm;
}

void MemoryManager::InstallDumpCallback(DumpRequestCallback callback, void* arg)
{
    MemoryManager* mm = Instance();
    mm->dumpCallback = callback;
    mm->callbackArg = arg;
}

CC_NOINLINE void* MemoryManager::Allocate(size_t size, size_t poolIndex)
{
    assert(IsInternalAllocationPool(poolIndex) || poolIndex < MMConst::MAX_ALLOC_POOL_COUNT);

    // On zero-sized allocation request allocate 1 byte to return unique memory block
    if (0 == size)
    {
        size = 1;
    }

    size_t totalSize = sizeof(MemoryBlock) + size;
    if (totalSize & (BLOCK_ALIGN - 1))
    {
        totalSize += (BLOCK_ALIGN - (totalSize & (BLOCK_ALIGN - 1)));
    }

    MemoryBlock* block = static_cast<MemoryBlock*>(MallocHook::Malloc(totalSize));
    if (block != nullptr)
    {
        Memset(block, 0, sizeof(MemoryBlock));
        block->mark = BLOCK_MARK;
        block->pool = poolIndex;
        block->allocByApp = size;
        block->allocTotal = totalSize;
        block->realBlockStart = static_cast<void*>(block);
        if (!IsInternalAllocationPool(poolIndex))
        {
            Backtrace backtrace;
            {
                // Lock is required only here:
                //  - collecting backtrace
                //  - updating statistics
                //  - inserting block into internal list
                LockType lock(mutex);

                CollectBacktrace(&backtrace, 1);
                block->orderNo = nextBlockNo++;
                block->backtraceHash = backtrace.hash;

                InsertBlock(block);
                UpdateStatAfterAlloc(block, poolIndex);
            }
            {
                LockType backtraceLock(backtraceMutex);
                InsertBacktrace(backtrace);
            }
        }
        else
        {
            block->next = nullptr;
            block->prev = nullptr;
            block->orderNo = 0;

            // For internal allocation pool lock is required only for updating statistics
            LockType lock(mutex);
            statGeneral.allocInternal += block->allocByApp;
            statGeneral.internalBlockCount += 1;
           
        }
        return static_cast<void*>(block + 1);
    }
    return nullptr;
}

CC_NOINLINE void* MemoryManager::AlignedAllocate(size_t size, size_t align, size_t poolIndex)
{
    // TODO: check whether size is integral multiple of align
    assert(align > 0 && 0 == (align & (align - 1)));    // Check whether align is power of 2
    assert(IsInternalAllocationPool(poolIndex) || poolIndex < MMConst::MAX_ALLOC_POOL_COUNT);

    // On zero-sized allocation request allocate 1 byte to return unique memory block
    if (0 == size)
    {
        size = 1;
    }

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
        // Some pointer arithmetics
        uintptr_t aligned = uintptr_t(realPtr) + sizeof(MemoryBlock);
        aligned += align - (aligned & (align - 1));

        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(aligned - sizeof(MemoryBlock));
        Memset(block, 0, sizeof(MemoryBlock));
        block->mark = BLOCK_MARK;
        block->pool = poolIndex;
        block->allocByApp = size;
        block->allocTotal = totalSize;
        block->realBlockStart = realPtr;
        if (!IsInternalAllocationPool(poolIndex))
        {
            Backtrace backtrace;
            {
                // Lock is required only here:
                //  - collecting backtrace
                //  - updating statistics
                //  - inserting block into internal list
                LockType lock(mutex);

                CollectBacktrace(&backtrace, 1);
                block->orderNo = nextBlockNo++;
                block->backtraceHash = backtrace.hash;

                InsertBlock(block);
                UpdateStatAfterAlloc(block, poolIndex);
            }
            {
                LockType backtraceLock(backtraceMutex);
                InsertBacktrace(backtrace);
            }
        }
        else
        {
            block->next = nullptr;
            block->prev = nullptr;
            block->orderNo = 0;

            // For internal allocation pool lock is required only for updating statistics
            LockType lock(mutex);
            statGeneral.allocInternal += block->allocByApp;
            statGeneral.internalBlockCount += 1;
            
        }
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
    
    MemoryBlock* block = IsTrackedBlock(ptr);
    if (block != nullptr)
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
        return DAVA::MallocHook::Realloc(ptr, newSize);
    }
}
    
void MemoryManager::Deallocate(void* ptr)
{
    if (ptr != nullptr)
    {
        MemoryBlock* block = IsTrackedBlock(ptr);
        if (block != nullptr)
        {
            if (!IsInternalAllocationPool(block->pool))
            {
                {
                    // Lock is required only here:
                    //  - updating statistics
                    //  - inserting block into internal list
                    LockType lock(mutex);

                    RemoveBlock(block);
                    UpdateStatAfterDealloc(block, block->pool);
                }
                {
                    LockType backtraceLock(backtraceMutex);
                    RemoveBacktrace(block->backtraceHash);
                }
            }
            else
            {
                // For internal allocation pool lock is required only for updating statistics
                LockType lock(mutex);
                statGeneral.allocInternal -= block->allocByApp;
                statGeneral.internalBlockCount -= 1;
              
            }
            // Tracked memory block consists of header (of type struct MemoryBlock) and data block that returned to app.
            // Tracked memory blocks are distinguished by special mark in header.
            // In some cases, especially on iOS, memory allocations bypass memory manager, but memory freeing goes through
            // memory manager. So I need to erase header to properly free bypassed allocations as system can allocate memory at the same address
            void* freeMe = block->realBlockStart;
            Memset(block, 0xEC, sizeof(MemoryBlock));
            MallocHook::Free(freeMe);
        }
        else
        {
            {
                LockType lock(mutex);
                statGeneral.ghostBlockCount += 1;
                statGeneral.ghostSize += MallocHook::MallocSize(ptr);
            }
            MallocHook::Free(ptr);
        }
    }
}

void MemoryManager::EnterTagScope(uint32 tag)
{
    assert(tag != MMConst::DEFAULT_TAG);
    assert(tags.depth < MMConst::MAX_TAG_DEPTH - 1);
    if (tags.stack[tags.depth] == tag)
        return;
    LockType lock(mutex);
    tags.depth += 1;
    tags.stack[tags.depth] = tag;
    tags.begin[tags.depth] = nextBlockNo;
}

void MemoryManager::LeaveTagScope(uint32 tagToLeave)
{
    assert(tags.depth > 0);
    if (tagToLeave != 0 && tagToLeave != tags.stack[tags.depth])
        return;
    DVASSERT_MSG(tagToLeave == 0 || tagToLeave == tags.stack[tags.depth], "Leaving Tags don't match up");

    uint32 tag = 0;
    uint32 tagBegin = 0;
    uint32 tagEnd = 0;
    {
        LockType lock(mutex);
        tag = tags.stack[tags.depth];
      
        tagBegin = tags.begin[tags.depth];
        tagEnd = nextBlockNo;
        for (size_t i = 0;i < MMConst::MAX_ALLOC_POOL_COUNT;++i)
        {
            statAllocPool[tags.depth][i] = AllocPoolStat();
        }
        tags.depth -= 1;
    }
    if (dumpCallback != nullptr)
        dumpCallback(callbackArg, MMConst::DUMP_REQUEST_TAG, tag, tagBegin, tagEnd);
}

void MemoryManager::Checkpoint(uint32 checkpoint)
{
    if (dumpCallback != nullptr)
        dumpCallback(callbackArg, MMConst::DUMP_REQUEST_CHECKPOINT, checkpoint, 0, nextBlockNo);
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

MemoryManager::MemoryBlock* MemoryManager::IsTrackedBlock(void* ptr)
{
    MemoryBlock* block = static_cast<MemoryBlock*>(ptr) - 1;
    return BLOCK_MARK == block->mark ? block : nullptr;
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

void MemoryManager::UpdateStatAfterAlloc(MemoryBlock* block, size_t poolIndex)
{
    for (size_t i = 0;i <= tags.depth;++i)
    {
        // Calculate fixed statistics for allocation pool
        statAllocPool[i][poolIndex].allocByApp += block->allocByApp;
        statAllocPool[i][poolIndex].allocTotal += block->allocTotal;
        statAllocPool[i][poolIndex].blockCount += 1;

        if (block->allocByApp > statAllocPool[i][poolIndex].maxBlockSize)
            statAllocPool[i][poolIndex].maxBlockSize = static_cast<uint32>(block->allocByApp);

    }

    statGeneral.realSize += MallocHook::MallocSize(block->realBlockStart);
}

void MemoryManager::UpdateStatAfterDealloc(MemoryBlock* block, size_t poolIndex)
{
    for (size_t i = 0;i <= tags.depth;++i)
    {
        if (block->orderNo >= tags.begin[i])
        {
            assert(statAllocPool[i][poolIndex].blockCount >= 1);
            assert(statAllocPool[i][poolIndex].allocByApp >= block->allocByApp);
            assert(statAllocPool[i][poolIndex].allocTotal >= block->allocTotal);

            // Calculate fixed statistics for allocation pool
            statAllocPool[i][poolIndex].allocByApp -= block->allocByApp;
            statAllocPool[i][poolIndex].allocTotal -= block->allocTotal;
            statAllocPool[i][poolIndex].blockCount -= 1;
        }
    }

    statGeneral.realSize -= MallocHook::MallocSize(block->realBlockStart);
}

void MemoryManager::InsertBacktrace(Backtrace& backtrace)
{
    if (nullptr == backtraces)
    {
        backtraces = new (&backtraceStorage)BacktraceMap;
    }

    backtrace.hash = HashValue_N(reinterpret_cast<const char*>(backtrace.frames), sizeof(backtrace.frames));
    backtrace.nref = 1;

    auto i = backtraces->find(backtrace.hash);
    if (i != backtraces->end())
    {
        // Backtrace already present, so increment refrence count
        i->second.nref += 1;
    }
    else
    {
        // New backtrace, add to list
        backtraces->insert(std::make_pair(backtrace.hash, backtrace));
    }
}

void MemoryManager::RemoveBacktrace(size_t hash)
{
    assert(backtraces != nullptr);

    auto i = backtraces->find(hash);
    if (i != backtraces->end())
    {
        Backtrace& backtrace = i->second;
        backtrace.nref -= 1;
        if (0 == backtrace.nref)
        {
            backtraces->erase(hash);
        }
    }
}

size_t MemoryManager::CalcStatConfigSize() const
{
    return sizeof(MMStatConfig) + sizeof(MMItemName) * (registeredTagCount + registeredAllocPoolCount - 1);
}

void MemoryManager::GetStatConfig(MMStatConfig* config) const
{
    DVASSERT(config != nullptr);
    
    config->maxTagCount = MMConst::MAX_TAG_COUNT;
    config->maxAllocPoolCount = MMConst::MAX_ALLOC_POOL_COUNT;
    config->tagCount = static_cast<uint32>(registeredTagCount);
    config->allocPoolCount = static_cast<uint32>(registeredAllocPoolCount);

    size_t k = 0;
    for (size_t i = 0;i < registeredTagCount;++i, ++k)
        config->names[k] = tagNames[i];
    for (size_t i = 0;i < registeredAllocPoolCount;++i, ++k)
        config->names[k] = allocPoolNames[i];
}

size_t MemoryManager::CalcStatSize() const
{
    return sizeof(MMStat) + sizeof(AllocPoolStat) * ((tags.depth + 1) * registeredAllocPoolCount - 1 + registeredAllocPoolCount);
}

void MemoryManager::GetStat(MMStat* stat) const
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

template<typename T>
inline T* Offset(void* ptr, size_t byteOffset)
{
    return reinterpret_cast<T*>(static_cast<uint8*>(ptr) + byteOffset);
}

size_t MemoryManager::GetDump(size_t userSize, void** buf, uint32 blockRangeBegin, uint32 blockRangeEnd)
{
    // TODO: carefully think of locking
    DVASSERT(userSize % 16 == 0);

    ObtainAllBacktraceSymbols();
    MemoryBlock* firstBlock = nullptr;
    MemoryBlock* lastBlock = nullptr;
    size_t nblocks = GetBlockRange(blockRangeBegin, blockRangeEnd, &firstBlock, &lastBlock);
    size_t nsymbols = symbols->size();
    size_t nbacktraces = backtraces->size();

    size_t bufSize = userSize
        + sizeof(MMDump)
        + sizeof(MMBlock) * nblocks
        + sizeof(MMBacktrace) * nbacktraces
        + sizeof(MMSymbol) * nsymbols;

    *buf = MallocHook::Malloc(bufSize);
    MMDump* dump = Offset<MMDump>(*buf, userSize);
    MMBlock* blocks = Offset<MMBlock>(dump, sizeof(MMDump));
    MMBacktrace* bt = Offset<MMBacktrace>(blocks, sizeof(MMBlock) * nblocks);
    MMSymbol* sym = Offset<MMSymbol>(bt, sizeof(MMBacktrace) * nbacktraces);

    dump->timestampBegin = 0;
    dump->timestampEnd = 0;
    dump->blockCount = static_cast<uint32>(nblocks);
    dump->backtraceCount = static_cast<uint32>(nbacktraces);
    dump->symbolCount = static_cast<uint32>(nsymbols);
    dump->blockBegin = static_cast<uint32>(firstBlock->orderNo);
    dump->blockEnd = static_cast<uint32>(lastBlock->orderNo);
    dump->type = 0;
    dump->tag = 0;

    size_t iBlock = 0;
    size_t nblocksToCheck = 0;
    while (firstBlock != nullptr)
    {
        blocks[iBlock].addr = reinterpret_cast<uint64>(firstBlock + 1);
        blocks[iBlock].allocByApp = static_cast<uint32>(firstBlock->allocByApp);
        blocks[iBlock].allocTotal = static_cast<uint32>(firstBlock->allocTotal);
        blocks[iBlock].pool = static_cast<uint32>(firstBlock->pool);
        blocks[iBlock].orderNo = static_cast<uint32>(firstBlock->orderNo);
        blocks[iBlock].backtraceHash = static_cast<uint32>(firstBlock->backtraceHash);
        iBlock += 1;
        nblocksToCheck += 1;
        if (firstBlock == lastBlock)
            break;
        firstBlock = firstBlock->next;
    }

    size_t iBt = 0;
    for (auto i = backtraces->cbegin(), e = backtraces->cend();i != e;++i)
    {
        const Backtrace& o = i->second;
        bt[iBt].hash = static_cast<uint32>(o.hash);
        for (size_t i = 0;i < MMConst::BACKTRACE_DEPTH;++i)
            bt[iBt].frames[i] = reinterpret_cast<uint64>(o.frames[i]);
        iBt += 1;
    }

    size_t iSym = 0;
    for (auto i = symbols->cbegin(), e = symbols->cend();i != e;++i)
    {
        void* addr = (*i).first;
        const InternalString& s = (*i).second;

        sym[iSym].addr = reinterpret_cast<uint64>(addr);
        strncpy(sym[iSym].name, s.c_str(), MMSymbol::NAME_LENGTH);
        sym[iSym].name[MMSymbol::NAME_LENGTH - 1] = '\0';
        iSym += 1;
    }
    return bufSize;
}

void MemoryManager::FreeDump(void* ptr)
{
    MallocHook::Free(ptr);
}

size_t MemoryManager::GetBlockRange(uint32 rangeBegin, uint32 rangeEnd, MemoryBlock** begin, MemoryBlock** end)
{
    size_t nblocks = 0;
    MemoryBlock* cur = head;
    while (cur != nullptr && cur->orderNo >= rangeEnd)
        cur = cur->next;
    *begin = cur;
    MemoryBlock* prev = cur;
    while (cur != nullptr && cur->orderNo > rangeBegin)
    {
        nblocks += 1;
        prev = cur;
        cur = cur->next;
    }
    *end = cur != nullptr ? cur : prev;
    return nblocks;
}

CC_NOINLINE void MemoryManager::CollectBacktrace(Backtrace* backtrace, size_t nskip)
{
    Memset(backtrace, 0, sizeof(Backtrace));
#if defined(__DAVAENGINE_WINDOWS__)
    backtrace->depth = CaptureStackBackTrace(nskip + 1, COUNT_OF(backtrace->frames), backtrace->frames, nullptr);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    backtrace->depth = ::backtrace(backtrace->frames, COUNT_OF(backtrace->frames));
#elif defined(__DAVAENGINE_ANDROID__)
    backtrace->depth = 0;
#endif
    backtrace->hash = HashValue_N(reinterpret_cast<const char*>(backtrace->frames), sizeof(backtrace->frames));
    backtrace->nref = 1;
    backtrace->symbolsCollected = false;
}

void MemoryManager::ObtainBacktraceSymbols(const Backtrace* backtrace)
{
    if (nullptr == symbols)
    {
        symbols = new (&symbolStorage) SymbolMap;
    }
    
#if defined(__DAVAENGINE_WINDOWS__)
    HANDLE hprocess = GetCurrentProcess();
    if (!symInited)
    {
        SymInitialize(hprocess, nullptr, TRUE);
        symInited = true;
    }

    const size_t NAME_LENGTH = 256;
    uint8 buf[sizeof(SYMBOL_INFO) + NAME_LENGTH];
    SYMBOL_INFO* symInfo = reinterpret_cast<SYMBOL_INFO*>(buf);
    symInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    symInfo->MaxNameLen = NAME_LENGTH;

    for (size_t i = 0;i < COUNT_OF(backtrace->frames) && backtrace->frames[i] != nullptr;++i)
    {
        if (symbols->find(backtrace->frames[i]) == symbols->cend())
        {
            if (SymFromAddr(hprocess, reinterpret_cast<DWORD64>(backtrace->frames[i]), 0, symInfo))
                symbols->emplace(std::make_pair(backtrace->frames[i], symInfo->Name));
        }
    }
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    for (size_t i = 0;i < COUNT_OF(backtrace->frames) && backtrace->frames[i] != nullptr;++i)
    {
        if (symbols->find(backtrace->frames[i]) == symbols->cend())
        {
            /*
             https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/dladdr.3.html#//apple_ref/doc/man/3/dladdr
             If an image containing addr cannot be found, dladdr() returns 0.  On success, a non-zero value is returned.
             If the image containing addr is found, but no nearest symbol was found, the dli_sname and dli_saddr fields are set to NULL.
            */
            Dl_info dlinfo;
            if (dladdr(backtrace->frames[i], &dlinfo) != 0 && dlinfo.dli_sname != nullptr)
            {
                char buf[1024 * 4];
                int status = 0;
                size_t n = COUNT_OF(buf);
                abi::__cxa_demangle(dlinfo.dli_sname, buf, &n, &status);
                if (0 == status)
                    symbols->emplace(std::make_pair(backtrace->frames[i], buf));
                else
                    symbols->emplace(std::make_pair(backtrace->frames[i], dlinfo.dli_sname));
            }
        }
    }
#endif
}

void MemoryManager::ObtainAllBacktraceSymbols()
{
    for (auto& x : *backtraces)
    {
        if (!x.second.symbolsCollected)
        {
            ObtainBacktraceSymbols(&x.second);
            x.second.symbolsCollected = true;
        }
    }
}

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
