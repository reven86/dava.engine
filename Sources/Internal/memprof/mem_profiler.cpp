#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#if defined(MEMPROF_WIN32)
#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX

#include <windows.h>
#include <dbghelp.h>

#elif defined(MEMPROF_MACOS) || defined(MEMPROF_IOS)
#include <execinfo.h>
#include <malloc/malloc.h>

#elif defined(MEMPROF_ANDROID)

#endif

#include <new>
#include <cassert>
#include <cstring>

#include "malloc_hook.h"
#include "mem_profiler.h"
#include "FileSystem/Logger.h"

struct mem_profiler::backtrace_t
{
#if defined(MEMPROF_MACOS)
    static const uint32_t MAX_FRAMES = 16;
#else
    static const uint32_t MAX_FRAMES = 18;
#endif

    uint32_t depth;
    uint32_t padding;
    void*    frames[MAX_FRAMES];
};

struct mem_profiler::mem_block_t
{
    mem_block_t* prev;
    mem_block_t* next;
    uint32_t     mark;
    mem_type_e   type;
    uint32_t     alloc_size;
    uint32_t     total_size;
    uint32_t     order_no;
    //size_t       padding;

    uint32_t     cur_tag;
    backtrace_t  backtrace;
};

//mem_profiler::mem_profiler()
//    //: head(nullptr)
//    //, next_order_no(0)
//    //, tag_depth(0)
//{
//    static_assert(sizeof(mem_profiler::mem_block_t) % 16 == 0, "sizeof(memmgr::mem_block_t) % 16 == 0");
//    // do not clear variables as object must be static
//    // and memory allocation can happen before constructor is called
//    //memset(tag_bookmarks, 0, sizeof(tag_bookmarks));
//    //memset(stat, 0, sizeof(stat));
//}

//mem_profiler::~mem_profiler()
//{
//    printf("size=%u\n", v.size());
//    printf("capacity=%u\n", v.capacity());
//}

mem_profiler* mem_profiler::instance()
{
    static_assert(sizeof(mem_profiler::mem_block_t) % 16 == 0, "sizeof(memmgr::mem_block_t) % 16 == 0");
    static malloc_hook hook;
    static mem_profiler o;
    return &o;
}

void* mem_profiler::allocate(size_t size, mem_type_e type)
{
    return instance()->internal_allocate(size, type);
}

void mem_profiler::deallocate(void* ptr)
{
    instance()->internal_deallocate(ptr);
}

uint32_t mem_profiler::block_size(void* ptr)
{
    return instance()->internal_block_size(ptr);
}

void mem_profiler::enter_scope(uint32_t tag)
{
    instance()->internal_enter_scope(tag);
}

void mem_profiler::leave_scope()
{
    instance()->internal_leave_scope();
}

void mem_profiler::get_memstat(net_mem_stat_t* dst)
{
    mem_profiler* m = instance();
    dst->next_order = m->next_order_no;
    dst->tag_depth = m->tag_depth;
    for (uint32_t i = 0;i < m->tag_depth;++i)
        dst->tags[i] = m->tag_bookmarks[i].tag;
    Memcpy(dst->stat, m->stat, sizeof(m->stat));
    dst->stat[static_cast<int>(mem_type_e::MEM_TYPE_STL)][0].alloc_size = m->ndeletions;
    dst->stat[static_cast<int>(mem_type_e::MEM_TYPE_STL)][0].total_size = m->deletionSize;
}

void mem_profiler::dump(FILE* file)
{
    instance()->internal_dump(file);
}

void* mem_profiler::internal_allocate(size_t size, mem_type_e type)
{
    size_t total_size = sizeof(mem_block_t) + size;
    if (total_size & (BLOCK_ALIGN - 1))
        total_size += (BLOCK_ALIGN - (total_size & (BLOCK_ALIGN - 1)));

    LockType lock(mutex);
    mem_block_t* new_block = static_cast<mem_block_t*>(malloc_hook::do_malloc(total_size));
    new_block->mark        = BLOCK_MARK;
    new_block->type        = type;
    new_block->alloc_size  = size;
    new_block->total_size  = total_size;  // FIX: real malloc size
    new_block->order_no    = next_order_no++;

    new_block->cur_tag = tag_bookmarks[tag_depth].tag;

    collect_backtrace(new_block, 0);
    push_block(new_block);
    update_stat_after_push(new_block, type, tag_depth);

    return static_cast<void*>(new_block + 1);
}

void mem_profiler::internal_deallocate(void* ptr)
{
    bool needLog;
    needLog = false;
    if (ptr != nullptr)
    {
        LockType lock(mutex);
        mem_block_t* block = is_profiled_block(ptr);
        if (block != nullptr)
        {
            block->mark = BLOCK_DELETED;
            pop_block(block);
            update_stat_after_pop(block, block->type, tag_depth);
            malloc_hook::do_free(block);
        }
        else
        {
            ndeletions += 1;
#if defined(MEMPROF_IOS) || defined(MEMPROF_MACOS)
            deletionSize += malloc_size(ptr);
            needLog = false;
#endif
            malloc_hook::do_free(ptr);
        }
        
    }
   /* if(needLog)
    {
        mem_block_t block;
        collect_backtrace(&block, 1);
        char ** bt = backtrace_symbols(block.backtrace.frames, block.backtrace.depth);
        for(size_t i=0;i<block.backtrace.depth;i++)
        {
            DAVA::Logger::Error("%s",bt[i]);
        }
        needLog = false;
    }*/
}

uint32_t mem_profiler::internal_block_size(void* ptr)
{
    if (ptr != nullptr)
    {
        LockType lock(mutex);
        mem_block_t* block = is_profiled_block(ptr);
        if (block != nullptr)
            return block->alloc_size;
    }
    return 0;
}

void mem_profiler::internal_enter_scope(uint32_t tag)
{
    assert(tag != 0);
    assert(tag_depth < TAG_DEPTH);

#ifdef TEST_VECTOR
    v.resize(v.size() + 999);
#endif

    LockType lock(mutex);
    
    tag_depth += 1;
    tag_bookmarks[tag_depth].tag   = tag;
    tag_bookmarks[tag_depth].begin = next_order_no;
    tag_bookmarks[tag_depth].end   = -1;
}

void mem_profiler::internal_leave_scope()
{
    assert(tag_depth > 0);

    LockType lock(mutex);
    tag_bookmarks[tag_depth].end = next_order_no;
    //internal_dump_tag(tag_bookmarks[tag_depth]);
    tag_bookmarks[tag_depth] = bookmark_t();
    for (size_t i = static_cast<size_t>(mem_type_e::MEM_TYPE_INTERNAL);i < static_cast<size_t>(mem_type_e::MEM_TYPE_COUNT);++i)
    {
        stat[i][tag_depth].alloc_size = 0;
        stat[i][tag_depth].total_size = 0;
        stat[i][tag_depth].nblocks    = 0;
    }
    tag_depth -= 1;
}

void mem_profiler::push_block(mem_block_t* block)
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

mem_profiler::mem_block_t* mem_profiler::is_profiled_block(void* ptr)
{
    mem_block_t* block = static_cast<mem_block_t*>(ptr) - 1;
    return BLOCK_MARK == block->mark ? block : nullptr;
}

mem_profiler::mem_block_t* mem_profiler::find_block(void* ptr)
{
    mem_block_t* block_to_find = static_cast<mem_block_t*>(ptr)-1;
    mem_block_t* cur_block = head;
    while (cur_block != nullptr)
    {
        if (cur_block == block_to_find)
            return cur_block;
        cur_block = cur_block->next;
    }
    return nullptr;
}

mem_profiler::mem_block_t* mem_profiler::find_block(uint32_t order)
{
    mem_block_t* cur_block = head;
    while (cur_block != nullptr)
    {
        if (cur_block->order_no == order)
            return cur_block;
        cur_block = cur_block->next;
    }
    return nullptr;
}

void mem_profiler::pop_block(mem_block_t* block)
{
    if (block->prev != nullptr)
        block->prev->next = block->next;
    if (block->next != nullptr)
        block->next->prev = block->prev;
    if (block == head)
        head = head->next;
}

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

void mem_profiler::update_stat_after_push(mem_block_t* block, mem_type_e type, uint32_t depth)
{
    const size_t mem_index = static_cast<size_t>(type);
    for (uint32_t i = 0;i <= depth;++i)
    {
        stat[mem_index][i].alloc_size += block->alloc_size;
        stat[mem_index][i].total_size += block->total_size;
        stat[mem_index][i].nblocks    += 1;

        if (stat[mem_index][i].alloc_size > stat[mem_index][i].peak_alloc_size)
            stat[mem_index][i].peak_alloc_size = stat[mem_index][i].alloc_size;
        if (stat[mem_index][i].total_size > stat[mem_index][i].peak_total_size)
            stat[mem_index][i].peak_total_size = stat[mem_index][i].total_size;
        if (block->alloc_size > stat[mem_index][i].max_block_size)
            stat[mem_index][i].max_block_size = block->alloc_size;
    }
}

void mem_profiler::update_stat_after_pop(mem_block_t* block, mem_type_e type, uint32_t depth)
{
    const size_t mem_index = static_cast<size_t>(type);
    for (uint32_t i = 0;i <= depth;++i)
    {
        assert(stat[mem_index][i].nblocks >= 1);
        assert(stat[mem_index][i].alloc_size >= block->alloc_size);
        assert(stat[mem_index][i].total_size >= block->total_size);

        stat[mem_index][i].alloc_size -= block->alloc_size;
        stat[mem_index][i].total_size -= block->total_size;
        stat[mem_index][i].nblocks    -= 1;
    }
}

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

#endif
