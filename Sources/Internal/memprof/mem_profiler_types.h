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
