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

const uint32_t MEMPROF_TAG_DEPTH = 3;
const uint32_t MEMPROF_MEM_COUNT = static_cast<uint32_t>(mem_type_e::MEM_TYPE_COUNT);

struct mem_stat_t
{
    uint32_t alloc_size;
    uint32_t total_size;
    uint32_t peak_alloc_size;
    uint32_t peak_total_size;
    uint32_t max_block_size;
    uint32_t nblocks;
};

struct net_mem_stat_t
{
    uint32_t timestamp;
    uint32_t next_order;
    uint32_t tag_depth;
    uint8_t tags[MEMPROF_TAG_DEPTH];
    uint8_t padding[1];
    mem_stat_t stat[MEMPROF_MEM_COUNT][MEMPROF_TAG_DEPTH];
};

static_assert(sizeof(net_mem_stat_t) % 8 == 0, "sizeof(net_mem_stat_t) % 8 == 0");
