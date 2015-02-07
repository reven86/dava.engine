#pragma once

#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#include "mem_profiler_types.h"

void* internal_alloc(size_t size, mem_type_e type);
void internal_free(void* ptr);

#endif
