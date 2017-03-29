#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
struct MMBlock;
}

class MemorySnapshot;

struct BlockLink
{
    using Item = std::pair<const DAVA::MMBlock*, const DAVA::MMBlock*>;

    static const DAVA::MMBlock* AnyBlock(const Item& item)
    {
        return item.first != nullptr ? item.first : item.second;
    }
    static const DAVA::MMBlock* Block(const Item& item, int index)
    {
        return 0 == index ? item.first : item.second;
    }

    static BlockLink CreateBlockLink(const MemorySnapshot* snapshot);
    static BlockLink CreateBlockLink(const MemorySnapshot* snapshot1, const MemorySnapshot* snapshot2);
    static BlockLink CreateBlockLink(const DAVA::Vector<DAVA::MMBlock*>& blocks, const MemorySnapshot* snapshot);
    static BlockLink CreateBlockLink(const DAVA::Vector<DAVA::MMBlock*>& blocks1, const MemorySnapshot* snapshot1,
                                     const DAVA::Vector<DAVA::MMBlock*>& blocks2, const MemorySnapshot* snapshot2);

    BlockLink();
    BlockLink(BlockLink&& other);
    BlockLink& operator=(BlockLink&& other);

    DAVA::Vector<Item> items;
    DAVA::uint32 linkCount = 0;
    DAVA::uint32 allocSize[2];
    DAVA::uint32 blockCount[2];
    const MemorySnapshot* sourceSnapshots[2];
};
