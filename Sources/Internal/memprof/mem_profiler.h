#pragma once

#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

//#define TEST_VECTOR

#include "Thread/Spinlock.h"
#include "Thread/LockGuard.h"
#include "Platform/Mutex.h"

#include <cstdio>
#include <vector>
#include <unordered_map>

#include "mem_profiler_types.h"
#include "internal_allocator.h"

class mem_profiler final
{
    struct mem_block_t;
    struct backtrace_t;

    struct bookmark_t
    {
        uint32_t tag;
        uint32_t begin;
        uint32_t end;
    };

    static const uint32_t BLOCK_MARK = 0xBA0BAB;
    static const uint32_t BLOCK_DELETED = 0xACCA;
    static const size_t BLOCK_ALIGN = 16;

    static const uint32_t TAG_DEPTH = 3;
    static const size_t MEM_COUNT = static_cast<size_t>(mem_type_e::MEM_TYPE_COUNT);

public:
    mem_profiler() = default;
    ~mem_profiler() = default;

    static mem_profiler* instance();

    static void* allocate(size_t size, mem_type_e type);
    static void deallocate(void* ptr);

    static void enter_scope(uint32_t tag);
    static void leave_scope();

    static void dump(FILE* file);

private:
    void* internal_allocate(size_t size, mem_type_e type);
    void internal_deallocate(void* ptr);

    void internal_enter_scope(uint32_t tag);
    void internal_leave_scope();

    void push_block(mem_block_t* block);
    mem_block_t* is_profiled_block(void* ptr);
    mem_block_t* find_block(void* ptr);
    mem_block_t* find_block(uint32_t order);
    void pop_block(mem_block_t* block);

    void collect_backtrace(mem_block_t* block, size_t nskip);

    void update_stat_after_push(mem_block_t* block, mem_type_e type, uint32_t depth);
    void update_stat_after_pop(mem_block_t* block, mem_type_e type, uint32_t depth);

    void internal_dump(FILE* file);
    void internal_dump_memory_type(FILE* file, size_t index);
    void internal_dump_tag(const bookmark_t& bookmark);
    void internal_dump_backtrace(FILE* file, mem_block_t* block);

private:
    mem_block_t* head;
    uint32_t     next_order_no;
    uint32_t     tag_depth;
    uint32_t     ndeletions;
    bookmark_t   tag_bookmarks[TAG_DEPTH];
    mem_stat_t   stat[MEM_COUNT][TAG_DEPTH];
    
    typedef DAVA::Spinlock MutexType;
    //typedef DAVA::Mutex MutexType;
    typedef DAVA::LockGuard<MutexType> LockType;
    
    MutexType mutex;
    
    //typedef std::unordered_map<uintptr_t, char*, std::hash<uintptr_t>, std::equal_to<uintptr_t>, internal_allocator<std::pair<const uintptr_t, char*>>> sym_map_t;
    //typedef std::map<void*, char, std::less<void*>, internal_allocator<std::pair<void*, char*>>> sym_map_t;
    //sym_map_t* sym;
    
#ifdef TEST_VECTOR
    std::vector<int, internal_allocator<int>> v;
#endif
};

#define MEMPROF_INIT()      mem_profiler::instance()
#define MEMPROF_ENTER(tag)  mem_profiler::instance()->enter_scope(tag)
#define MEMPROF_LEAVE()     mem_profiler::instance()->leave_scope()
#define MEMPROF_DUMP(file)  mem_profiler::instance()->dump(file)

#else

#define MEMPROF_INIT()
#define MEMPROF_ENTER(tag)
#define MEMPROF_LEAVE()
#define MEMPROF_DUMP(file)

#endif
