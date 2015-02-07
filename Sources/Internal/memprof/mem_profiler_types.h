#pragma once

#include "memprof_config.h"

#include <cstdint>

enum class mem_type_e : uint32_t
{
    MEM_TYPE_INTERNAL = 0,
    MEM_TYPE_NEW,
    MEM_TYPE_STL,
    MEM_TYPE_CLASS,
    MEM_TYPE_OTHER,
    MEM_TYPE_COUNT
};

struct mem_stat_t
{
    uint32_t alloc_size;
    uint32_t total_size;
    uint32_t peak_alloc_size;
    uint32_t peak_total_size;
    uint32_t max_block_size;
    uint32_t nblocks;
};

#if defined(MEMPROF_ENABLE)

#include "mem_profiler.h"
#include "profiled_allocator.h"

template<typename T>
using std_allocator = profiled_allocator<T>;

#define MEMPROF_CLASS()                                                     \
    void* operator new(std::size_t size)                                    \
    {                                                                       \
        return mem_profiler::allocate(size, mem_type_e::MEM_TYPE_CLASS);    \
    }                                                                       \
                                                                            \
    void operator delete(void* ptr)                                         \
    {                                                                       \
        mem_profiler::deallocate(ptr);                                      \
    }

#else

#define MEMPROF_CLASS()

template<typename T>
using std_allocator = std::allocator<T>;

#endif
