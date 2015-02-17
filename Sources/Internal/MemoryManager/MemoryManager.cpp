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
    
char8 MemoryManager::tagNames[MAX_TAG_COUNT][MAX_NAME_LENGTH] = {
    "application"
};

char8 MemoryManager::allocPoolNames[MAX_ALLOC_POOL_COUNT][MAX_NAME_LENGTH] = {
    "internal",
    "application"
};
    
void MemoryManager::RegisterAllocPoolName(size_t index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MAX_NAME_LENGTH);
    DVASSERT(FIRST_CUSTOM_ALLOC_POOL <= index && index < MAX_ALLOC_POOL_COUNT);
    DVASSERT(allocPoolNames[index - 1][0] != '\0');     // Names should be registered sequentially with no gap

    strncpy(allocPoolNames[index], name, MAX_NAME_LENGTH);
    allocPoolNames[index][MAX_NAME_LENGTH - 1] = '\0';
}

void MemoryManager::RegisterTagName(size_t index, const char8* name)
{
    DVASSERT(name != nullptr && 0 < strlen(name) && strlen(name) < MAX_NAME_LENGTH);
    DVASSERT(DEFAULT_TAG != index && index < MAX_TAG_COUNT);
    DVASSERT(tagNames[index - 1][0] != '\0');       // Names should be registered sequentially with no gap

    strncpy(tagNames[index], name, MAX_NAME_LENGTH);
    tagNames[index][MAX_NAME_LENGTH - 1] = '\0';
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
    assert(tagDepth < MAX_TAG_DEPTH - 1);

    LockType lock(mutex);
    tagDepth += 1;
    tagStack[tagDepth] = tag;
    tagBegin[tagDepth] = nextBlockNo;
}

void MemoryManager::LeaveScope()
{
    assert(tagDepth > 0);

    LockType lock(mutex);
    // TODO: perform action on tag leave
    for (size_t i = 0;i < MAX_ALLOC_POOL_COUNT;++i)
    {
        statAllocPool[tagDepth][i] = AllocPoolStat();
        // TODO: clear additional counters on tag leave
    }
    tagDepth -= 1;
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
    for (size_t i = 0;i <= tagDepth;++i)
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
    for (size_t i = 0;i <= tagDepth;++i)
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

GeneralInfo* MemoryManager::GetGeneralInfo()
{
    GeneralInfo temp;
    temp.tagCount = CalcNamesCount(tagNames[0], tagNames[0] + MAX_TAG_COUNT * MAX_NAME_LENGTH);
    temp.allocPoolCount = CalcNamesCount(allocPoolNames[0], allocPoolNames[0] + MAX_ALLOC_POOL_COUNT * MAX_NAME_LENGTH);
    temp.counterCount = 0;
    temp.poolCounterCount = 0;
    
    const size_t ntotal = temp.tagCount + temp.allocPoolCount + temp.counterCount + temp.poolCounterCount;
    
    uint8* buf = new uint8[sizeof(GeneralInfo) + (ntotal - 1) * MAX_NAME_LENGTH];
    GeneralInfo* result = new (buf) GeneralInfo;
    result->tagCount = temp.tagCount;
    result->allocPoolCount = temp.allocPoolCount;
    result->counterCount = temp.counterCount;
    result->poolCounterCount = temp.poolCounterCount;
    
    size_t curIndex = 0;
    CopyNames(result->names[curIndex], tagNames[0], temp.tagCount);
    curIndex += temp.tagCount;
    CopyNames(result->names[curIndex], allocPoolNames[0], temp.allocPoolCount);
    curIndex += temp.allocPoolCount;
    return result;
}

void MemoryManager::FreeGeneralInfo(const GeneralInfo* ptr)
{
    delete [] reinterpret_cast<const uint8*>(ptr);
}

CurrentAllocStat* MemoryManager::GetCurrentAllocStat()
{
    CurrentAllocStat temp;

    return nullptr;
}

void MemoryManager::FreeCurrentAllocStat(const CurrentAllocStat* ptr)
{
    delete[] reinterpret_cast<const uint8*>(ptr);
}

size_t MemoryManager::CalcNamesCount(const char8* begin, const char* end)
{
    size_t n = 0;
    while (begin < end)
    {
        if (begin[0] == '\0')
            break;
        n += 1;
        begin += MAX_NAME_LENGTH;
    }
    return n;
}

void MemoryManager::CopyNames(char8* dst, const char8* src, size_t n)
{
    const size_t LENGTH = MAX_NAME_LENGTH > GeneralInfo::NAME_LENGTH ? GeneralInfo::NAME_LENGTH : MAX_NAME_LENGTH;
    for (size_t i = 0;i < n;++i)
    {
        strncpy(dst, src, LENGTH);
        dst[LENGTH - 1] = '\0';
        
        dst += GeneralInfo::NAME_LENGTH;
        src += MAX_NAME_LENGTH;
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

#if 0   // disable dumps
void mem_profiler::internal_dump(FILE* file)
{
    //LockType lock(mutex);
    
    for (size_t i = static_cast<size_t>(mem_type_e::MEM_TYPE_INTERNAL);i < static_cast<size_t>(mem_type_e::MEM_TYPE_COUNT);++i)
        internal_dump_memory_type(file, i);
    fprintf(file, "External deletions: %u\n", ndeletions);
#ifdef TEST_VECTOR
    fprintf(file, "v size: %u\n", v.size());
    fprintf(file, "v capacity: %u\n", v.capacity());
#endif
}

void mem_profiler::internal_dump_memory_type(FILE* file, size_t mem_index)
{
    static const char* mem_descr[] = {
        "INTERNAL",
        "NEW", 
        "STL",
        "CLASS",
        "OTHER"
    };
    fprintf(file, "stat: mem_type=%s, tag_depth=%u\n", mem_descr[mem_index], tag_depth);
    for (uint32_t i = 0;i <= tag_depth;++i)
    {
        fprintf(file, "  tag            : %u\n", tag_bookmarks[i].tag);
        fprintf(file, "  alloc_size     : %u  ", stat[mem_index][i].alloc_size);
        if (stat[mem_index][i].alloc_size > 0)
            fprintf(file, "!!!!!!!");
        fprintf(file, "\n");
        fprintf(file, "  total_size     : %u\n", stat[mem_index][i].total_size);
        fprintf(file, "  peak_alloc_size: %u\n", stat[mem_index][i].peak_alloc_size);
        fprintf(file, "  peak_total_size: %u\n", stat[mem_index][i].peak_total_size);
        fprintf(file, "  max_block_size : %u\n", stat[mem_index][i].max_block_size);
        fprintf(file, "  nblocks        : %u\n", stat[mem_index][i].nblocks);
        if (stat[mem_index][i].nblocks > 0)
        {
            mem_block_t* cur = head;
            while (cur != nullptr)
            {
                if (static_cast<size_t>(cur->type) == mem_index)
                {
                    fprintf(file, "    ptr=%p, alloc_size=%u, total_size=%u, order=%u, tag=%u", cur + 1, cur->alloc_size, cur->total_size, cur->order_no, cur->cur_tag);
                    if (cur->mark == BLOCK_DELETED)
                        fprintf(file, "        DELETED");
                    fprintf(file, "\n");
                    internal_dump_backtrace(file, cur);
                }
                cur = cur->next;
            }
        }
        fprintf(file, "  ========================\n");
    }
}

void mem_profiler::internal_dump_backtrace(FILE* file, mem_block_t* block)
{
#if defined(MEMPROF_WIN32)
    HANDLE hprocess = GetCurrentProcess();
    SymInitialize(hprocess, nullptr, TRUE);

    const size_t NAME_LENGTH = 128;
    uint8_t symbol_buf[sizeof(SYMBOL_INFO) + NAME_LENGTH];
    SYMBOL_INFO* symbol = new (symbol_buf) SYMBOL_INFO();
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = NAME_LENGTH;

    for (size_t i = 0;i < block->backtrace.depth;++i)
    {
        SymFromAddr(hprocess, reinterpret_cast<DWORD64>(block->backtrace.frames[i]), 0, symbol);
        fprintf(file, "        addr=%p, func=%s\n", block->backtrace.frames[i], symbol->Name);
    }

    SymCleanup(hprocess);
#elif defined(MEMPROF_MACOS)
    if (block->backtrace.depth > 0)
    {
        /*if (!sym)
        {
            void* buf = internal_allocate(sizeof(sym_map_t), mem_type_e::MEM_TYPE_INTERNAL);
            sym = new (buf) sym_map_t;
        }
        
        int32_t div = (int32_t)block->backtrace.depth - 1;
        for (;div >= 0;--div)
        {
            uintptr_t key = reinterpret_cast<uintptr_t>(block->backtrace.frames[div]);
            auto i = sym->find(key);
            if (i == sym->end())
                break;
        }
        
        if (div >= 0)
        {
            char** symbols = backtrace_symbols(block->backtrace.frames, static_cast<int>(div));
            for (int32_t i = 0;i < div;++i)
            {
                size_tstrlen(symbols[i]);
            }
            free(symbols);
        }*/
        
        char** symbols = backtrace_symbols(block->backtrace.frames, static_cast<int>(block->backtrace.depth));
        for (size_t i = 0;i < block->backtrace.depth;++i)
        {
            fprintf(file, "        addr=%p, func=%s\n", block->backtrace.frames[i], symbols[i]);
        }
        free(symbols);
    }
#elif defined(MEMPROF_ANDROID)
    for (size_t i = 0;i < block->backtrace.depth;++i)
    {
        fprintf(file, "        addr=%p, func=%s\n", block->backtrace.frames[i], "");    //symbols[i]);
    }
#endif
}

void mem_profiler::internal_dump_tag(const bookmark_t& bookmark)
{
    /*
    printf("Leave tag: %u\n", bookmark.tag);
    for (size_t i = bookmark.begin;i < bookmark.end;++i)
    {
        mem_block_t* block = find_block(i);
        if (block == nullptr) continue;
        printf("  ptr=%p, alloc_size=%u, total_size=%u, order=%u\n", block + 1, block->alloc_size, block->total_size, block->order_no);
        dump_backtrace(block);
    }
    */
}

#endif  // disable dumps

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
