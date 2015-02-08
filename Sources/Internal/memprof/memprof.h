#pragma once

#include "profiled_allocator.h"
#include "mem_profiler_types.h"

#if defined(MEMPROF_ENABLE)

#include "internal_alloc.h"
#include "profiled_allocator.h"

template<typename T>
using std_allocator = profiled_allocator<T>;

#define MEMPROF_CLASS()                                                     \
    void* operator new(std::size_t size)                                    \
    {                                                                       \
        return internal_alloc(size, mem_type_e::MEM_TYPE_CLASS);            \
    }                                                                       \
                                                                            \
    void operator delete(void* ptr)                                         \
    {                                                                       \
       internal_free(ptr);                                                  \
    }

#else

#define MEMPROF_CLASS()

template<typename T>
using std_allocator = std::allocator<T>;

#endif
