#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#include "mem_profiler.h"

/*
* http://en.cppreference.com/w/cpp/memory/new/operator_new
* The single-object version is called by the standard library implementations of all other versions,
* so replacing that one function is sufficient to handle all deallocations.	(since C++11)
*/

#if defined(MEMPROF_WIN32)
#define MEMPROF_NOEXCEPT
#else
#define MEMPROF_NOEXCEPT    noexcept
#endif

void* operator new(size_t size)
{
    //return mem_profiler::allocate(size, mem_type_e::MEM_TYPE_OTHER);
    return mem_profiler::allocate(size, mem_type_e::MEM_TYPE_NEW);
}

void operator delete(void* ptr) MEMPROF_NOEXCEPT
{
    mem_profiler::deallocate(ptr);
}

void* operator new [](size_t size)
{
    return mem_profiler::allocate(size, mem_type_e::MEM_TYPE_NEW);
}

void operator delete[](void* ptr) MEMPROF_NOEXCEPT
{
    mem_profiler::deallocate(ptr);
}

#endif
