#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#include "mem_profiler.h"
#include "mem_profiler_types.h"

void* internal_alloc(size_t size, mem_type_e type)
{
    return mem_profiler::allocate(size, type);
}

void internal_free(void* ptr)
{
    mem_profiler::deallocate(ptr);
}

#endif
