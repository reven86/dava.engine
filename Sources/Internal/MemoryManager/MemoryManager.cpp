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
#include <windows.h>
#include <dbghelp.h>
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#elif defined(__DAVAENGINE_ANDROID__)
#endif

#include "Debug/DVAssert.h"

#include "MallocHook.h"
#include "MemoryManager.h"

namespace DAVA
{

struct MemoryManager::Backtrace
{
    void* frames[16];
};

struct MemoryManager::MemoryBlock
{
    Backtrace backtrace;
    MemoryBlock* prev;      // Pointer to previous block
    MemoryBlock* next;      // Pointer to next block
    void* realBlockStart;   // Pointer to real block start
    size_t backtraceHash;   // Unique hash number to identify block backtrace
    size_t allocByApp;      // Size requested by application
    size_t allocTotal;      // Total allocated size
    size_t orderNo;         // Block order number
    size_t pool;            // Allocation pool block belongs to
    size_t mark;            // Mark to distinguish tracked memory blocks
    size_t padding[3];
};

MMItemName MemoryManager::tagNames[MAX_TAG_COUNT] = {
    {"application"}
};

MMItemName MemoryManager::allocPoolNames[MAX_ALLOC_POOL_COUNT] = {
    {"application"},
    {"FMOD"}
};

size_t MemoryManager::registeredTagCount = 1;
size_t MemoryManager::registeredAllocPoolCount = ePredefAllocPools::PREDEF_POOL_COUNT;
    
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

void MemoryManager::InstallTagCallback(TagCallback callback, void* arg)
{
    MemoryManager* mm = Instance();
    mm->tagCallback = callback;
    mm->callbackArg = arg;
}

DAVA_NOINLINE void* MemoryManager::Alloc(size_t size, uint32 poolIndex)
{
    assert(IsInternalAllocationPool(poolIndex) || poolIndex < MAX_ALLOC_POOL_COUNT);

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
            // Lock is required only here:
            //  - collecting backtrace
            //  - updating statistics
            //  - inserting block into internal list
            LockType lock(mutex);

            block->orderNo = nextBlockNo++;
            block->backtraceHash = 0;
            CollectBacktrace(block, 1);

            InsertBlock(block);
            UpdateStatAfterAlloc(block, poolIndex);
        }
        else
        {
            block->next = nullptr;
            block->prev = nullptr;
            block->orderNo = 0;

            // For internal allocation pool lock is required only for updating statistics
            // TODO: update stat for internal block
        }
        return static_cast<void*>(block + 1);
    }
    return nullptr;
}

DAVA_NOINLINE void* MemoryManager::AlignedAlloc(size_t size, size_t align, uint32 poolIndex)
{
    // On zero-sized allocation request allocate 1 byte to return unique memory block
    if (0 == size)
    {
        size = 1;
    }

    // TODO: check whether size is integral multiple of align
    assert(align > 0 && 0 == (align & (align - 1)));    // Check whether align is power of 2
    assert(IsInternalAllocationPool(poolIndex) || poolIndex < MAX_ALLOC_POOL_COUNT);

    if (align < BLOCK_ALIGN)
    {
        align = BLOCK_ALIGN;
    }

    size_t totalSize = sizeof(MemoryBlock) + size + (align - 1);
    if (totalSize & (BLOCK_ALIGN - 1))
    {
        totalSize += (BLOCK_ALIGN - (totalSize & (BLOCK_ALIGN - 1)));
    }

    LockType lock(mutex);
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
            // Lock is required only here:
            //  - collecting backtrace
            //  - updating statistics
            //  - inserting block into internal list
            LockType lock(mutex);

            block->orderNo = nextBlockNo++;
            block->backtraceHash = 0;
            CollectBacktrace(block, 1);

            InsertBlock(block);
            UpdateStatAfterAlloc(block, poolIndex);
        }
        else
        {
            block->next = nullptr;
            block->prev = nullptr;
            block->orderNo = 0;

            // For internal allocation pool lock is required only for updating statistics
            // TODO: update stat for internal block
        }
        return reinterpret_cast<void*>(aligned);
    }
    return nullptr;
}

void MemoryManager::Dealloc(void* ptr)
{
    if (ptr != nullptr)
    {
        MemoryBlock* block = IsTrackedBlock(ptr);
        if (block != nullptr)
        {
            block->mark = BLOCK_DELETED;
            if (!IsInternalAllocationPool(block->pool))
            {
                // Lock is required only here:
                //  - updating statistics
                //  - inserting block into internal list
                LockType lock(mutex);

                RemoveBlock(block);
                UpdateStatAfterDealloc(block, block->pool);
            }
            else
            {
                // For internal allocation pool lock is required only for updating statistics
                // TODO: update stat for internal block
            }
            MallocHook::Free(block->realBlockStart);
        }
        else
        {
            statGeneral.ghostBlockCount += 1;
            statGeneral.ghostSize += MallocHook::MallocSize(ptr);
            MallocHook::Free(ptr);
        }
    }
}

void* MemoryManager::Realloc(void* ptr, size_t newSize)
{
    assert(ptr != nullptr);     // This realloc must not be called with ptr == nullptr

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

    uint32 tag;
    uint32 tagBegin;
    uint32 tagEnd;
    {
        LockType lock(mutex);
        tag = tags.stack[tags.depth];
        tagBegin = tags.begin[tags.depth];
        tagEnd = nextBlockNo;
        for (size_t i = 0;i < MAX_ALLOC_POOL_COUNT;++i)
        {
            statAllocPool[tags.depth][i] = AllocPoolStat();
        }
        tags.depth -= 1;
    }
    if (tagCallback != nullptr)
        tagCallback(callbackArg, tag, tagBegin, tagEnd);
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
    }
}

void MemoryManager::UpdateStatAfterDealloc(MemoryBlock* block, uint32 poolIndex)
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
    return sizeof(MMStat) + sizeof(AllocPoolStat) * ((tags.depth + 1) * registeredAllocPoolCount - 1);
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

template<typename T>
inline T* Offset(void* ptr, size_t byteOffset)
{
    return reinterpret_cast<T*>(static_cast<uint8*>(ptr) + byteOffset);
}

size_t MemoryManager::GetDumpInternal(size_t userSize, void** buf, uint32 blockRangeBegin, uint32 blockRangeEnd)
{
    DVASSERT(userSize % 16 == 0);

    ObtainAllBacktraceSymbols();
    MemoryBlock* firstBlock = nullptr;
    MemoryBlock* lastBlock = nullptr;
    size_t nblocks = GetBlockRange(blockRangeBegin, blockRangeEnd, &firstBlock, &lastBlock);
    size_t nsymbols = symbols->size();

    size_t bufSize = userSize + sizeof(MMDump) + sizeof(MMBlock) * nblocks + sizeof(MMSymbol) * nsymbols;

    *buf = MallocHook::Malloc(bufSize);
    MMDump* dump = Offset<MMDump>(*buf, userSize);
    MMSymbol* sym = Offset<MMSymbol>(*buf, userSize + sizeof(MMDump) + sizeof(MMBlock) * nblocks);

    dump->timestampBegin = 0;
    dump->timestampEnd = 0;
    dump->blockCount = static_cast<uint32>(nblocks);
    dump->symbolCount = static_cast<uint32>(nsymbols);
    dump->blockBegin = firstBlock->orderNo;
    dump->blockEnd = lastBlock->orderNo;
    dump->type = 0;
    dump->tag = 0;

    size_t iBlock = 0;
    size_t nblocksToCheck = 0;
    while (firstBlock != nullptr)
    {
        dump->blocks[iBlock].addr = reinterpret_cast<uint64>(firstBlock + 1);
        dump->blocks[iBlock].allocByApp = firstBlock->allocByApp;
        dump->blocks[iBlock].allocTotal = firstBlock->allocTotal;
        dump->blocks[iBlock].pool = firstBlock->pool;
        dump->blocks[iBlock].orderNo = firstBlock->orderNo;
        for (size_t x = 0;x < 16;++x)
            dump->blocks[iBlock].backtrace.frames[x] = reinterpret_cast<uint64>(firstBlock->backtrace.frames[x]);
        iBlock += 1;
        nblocksToCheck += 1;
        if (firstBlock == lastBlock)
            break;
        firstBlock = firstBlock->next;
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

void MemoryManager::FreeDumpInternal(void* ptr)
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

DAVA_NOINLINE void MemoryManager::CollectBacktrace(MemoryBlock* block, size_t nskip)
{
    Memset(&block->backtrace, 0, sizeof(block->backtrace));
#if defined(__DAVAENGINE_WIN32__)
    USHORT n = CaptureStackBackTrace(nskip + 1, COUNT_OF(block->backtrace.frames), block->backtrace.frames, nullptr);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    int n = backtrace(block->backtrace.frames, COUNT_OF(block->backtrace.frames));
#elif defined(__DAVAENGINE_ANDROID__)
#endif
}

void MemoryManager::ObtainBacktraceSymbols(const Backtrace* backtrace)
{
    if (nullptr == symbols)
    {
        symbols = new SymbolMap;
    }

#if defined(__DAVAENGINE_WIN32__)
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
            Dl_info dlinfo;
            /*
             https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/dladdr.3.html#//apple_ref/doc/man/3/dladdr
             If an image containing addr cannot be found, dladdr() returns 0.  On success, a non-zero value is returned.
             If the image containing addr is found, but no nearest symbol was found, the dli_sname and dli_saddr fields are set to NULL.
            */
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
    MemoryBlock* cur = head;
    while (cur != nullptr)
    {
        ObtainBacktraceSymbols(&cur->backtrace);
        cur = cur->next;
    }
}

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
