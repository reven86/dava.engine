#pragma once

#include <Base/BaseTypes.h>
#include <Base/TemplateHelpers.h>
#include <MemoryManager/MemoryManagerTypes.h>

class MemoryStatItem final
{
public:
    MemoryStatItem(DAVA::uint64 timestamp);
    MemoryStatItem(const DAVA::MMCurStat* stat, size_t poolCount, size_t tagCount);
    MemoryStatItem(const MemoryStatItem&) = default;
    MemoryStatItem& operator=(const MemoryStatItem&) = default;
    MemoryStatItem(MemoryStatItem&& other);
    MemoryStatItem& operator=(MemoryStatItem&& other);
    ~MemoryStatItem() = default;

    DAVA::uint64 Timestamp() const;
    const DAVA::GeneralAllocStat& GeneralStat() const;
    const DAVA::Vector<DAVA::AllocPoolStat>& PoolStat() const;
    const DAVA::Vector<DAVA::TagAllocStat>& TagStat() const;

    // Total memory consumption
    const DAVA::AllocPoolStat& TotalStat() const;

private:
    void Init(const DAVA::MMCurStat* rawStat, size_t poolCount, size_t tagCount);

private:
    DAVA::uint64 timestamp;
    DAVA::GeneralAllocStat statGeneral;
    DAVA::Vector<DAVA::AllocPoolStat> statPools;
    DAVA::Vector<DAVA::TagAllocStat> statTags;
};

//////////////////////////////////////////////////////////////////////////
inline MemoryStatItem::MemoryStatItem(DAVA::uint64 timestamp_)
    : timestamp(timestamp_)
{
}

inline MemoryStatItem::MemoryStatItem(const DAVA::MMCurStat* stat, size_t poolCount, size_t tagCount)
{
    Init(stat, poolCount, tagCount);
}

inline MemoryStatItem::MemoryStatItem(MemoryStatItem&& other)
    : timestamp(other.timestamp)
    , statGeneral(other.statGeneral)
    , statPools(std::move(other.statPools))
    , statTags(std::move(other.statTags))
{
}

inline MemoryStatItem& MemoryStatItem::operator=(MemoryStatItem&& other)
{
    if (this != &other)
    {
        timestamp = other.timestamp;
        statGeneral = other.statGeneral;
        statPools = std::move(other.statPools);
        statTags = std::move(other.statTags);
    }
    return *this;
}

inline DAVA::uint64 MemoryStatItem::Timestamp() const
{
    return timestamp;
}

inline const DAVA::GeneralAllocStat& MemoryStatItem::GeneralStat() const
{
    return statGeneral;
}

inline const DAVA::Vector<DAVA::AllocPoolStat>& MemoryStatItem::PoolStat() const
{
    return statPools;
}

inline const DAVA::Vector<DAVA::TagAllocStat>& MemoryStatItem::TagStat() const
{
    return statTags;
}

inline const DAVA::AllocPoolStat& MemoryStatItem::TotalStat() const
{
    return statPools[0];
}

inline void MemoryStatItem::Init(const DAVA::MMCurStat* stat, size_t poolCount, size_t tagCount)
{
    using namespace DAVA;

    statPools.reserve(poolCount);
    statTags.reserve(tagCount);
    timestamp = stat->timestamp;
    statGeneral = stat->statGeneral;

    const AllocPoolStat* pools = OffsetPointer<const AllocPoolStat>(stat, sizeof(MMCurStat));
    for (size_t i = 0; i < poolCount; ++i)
    {
        statPools.emplace_back(pools[i]);
    }
    const TagAllocStat* tags = OffsetPointer<const TagAllocStat>(pools, sizeof(AllocPoolStat) * poolCount);
    for (size_t i = 0; i < tagCount; ++i)
    {
        statTags.emplace_back(tags[i]);
    }
}
