#pragma once

namespace DAVA
{

const size_t MAX_TAG_STACK_DEPTH = 8;

struct MemoryStatHeader
{
    uint32 nextBlockNo;
    uint32 stackDepth;
    uint8 tagStack[MAX_TAG_STACK_DEPTH];
};

static_assert(sizeof(MemoryStatHeader) % 16 == 0, "sizeof(MemoryStatHeader) % 16 == 0");

struct MemoryStat
{
    uint32 userAllocatedMemory;
    uint32 totalAllocatedMemory;
    uint32 peakUserMemoryUsage;
    uint32 peakTotalMemoryUsage;
    uint32 maxUserBlockSize;
    uint32 blockCount;
};

static_assert(sizeof(MemoryStat) % 8 == 0, "sizeof(MemoryStat) % 8 == 0");

// list of supported tags
enum class TagNames : int32
{
    TAG_1 = 0,
    TAG_COUNT
};

// tag to name
inline const char8* TagToString(TagNames tag)
{
    static const char8* table[TagNames::TAG_COUNT] = {
        ""
    };
    return table[tag];
}

}   // namespace DAVA
