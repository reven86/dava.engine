#include <type_traits>

#include "RemoteTool/Private/MemoryTool/BlockLink.h"
#include "RemoteTool/Private/MemoryTool/MemorySnapshot.h"

#include <Debug/DVAssert.h>
#include <MemoryManager/MemoryManagerTypes.h>

using namespace DAVA;

namespace
{
// Helper template functions to return pointer from reference or pointer
template <typename T>
T* ToPointer(T& ref)
{
    return &ref;
}

template <typename T>
T* ToPointer(T* ptr)
{
    return ptr;
}

template <typename T>
BlockLink CreateBlockLinkT(const Vector<T>& blocks, const MemorySnapshot* snapshot)
{
    // Memory blocks should be sorted by orderNo in ascending order
    DVASSERT(snapshot != nullptr);
    DVASSERT(std::is_sorted(blocks.cbegin(), blocks.cend(), [](const T& l, const T& r) -> bool { return ToPointer(l)->orderNo < ToPointer(r)->orderNo; }));

    BlockLink link;
    link.linkCount = 1;
    link.allocSize[0] = 0;
    link.allocSize[1] = 0;
    link.blockCount[0] = static_cast<uint32>(blocks.size());
    link.blockCount[1] = 0;
    link.sourceSnapshots[0] = snapshot;
    link.sourceSnapshots[1] = nullptr;
    link.items.reserve(link.blockCount[0]);

    using pointer = typename std::conditional<std::is_pointer<T>::value, const T, const T*>::type;
    for (const auto& i : blocks)
    {
        pointer curBlock = ToPointer(i);

        link.items.emplace_back(curBlock, nullptr);
        link.allocSize[0] += curBlock->allocByApp;
    }
    return link;
}

template <typename T>
BlockLink CreateBlockLinkT(const Vector<T>& blocks1, const MemorySnapshot* snapshot1, const Vector<T>& blocks2, const MemorySnapshot* snapshot2)
{
    // Memory blocks should be sorted by orderNo in ascending order
    DVASSERT(snapshot1 != nullptr && snapshot2 != nullptr);
    DVASSERT(std::is_sorted(blocks1.cbegin(), blocks1.cend(), [](const T& l, const T& r) -> bool { return ToPointer(l)->orderNo < ToPointer(r)->orderNo; }));
    DVASSERT(std::is_sorted(blocks2.cbegin(), blocks2.cend(), [](const T& l, const T& r) -> bool { return ToPointer(l)->orderNo < ToPointer(r)->orderNo; }));

    BlockLink link;
    link.linkCount = 2;
    link.allocSize[0] = 0;
    link.allocSize[1] = 0;
    link.blockCount[0] = static_cast<uint32>(blocks1.size());
    link.blockCount[1] = static_cast<uint32>(blocks2.size());
    link.sourceSnapshots[0] = snapshot1;
    link.sourceSnapshots[1] = snapshot2;
    link.items.reserve(std::max(link.blockCount[0], link.blockCount[1]));

    auto begin1 = blocks1.begin();
    auto end1 = blocks1.end();
    auto begin2 = blocks2.begin();
    auto end2 = blocks2.end();
    while (begin1 != end1 && begin2 != end2)
    {
        const MMBlock* curBlock1 = ToPointer(*begin1);
        const MMBlock* curBlock2 = ToPointer(*begin2);
        if (curBlock1->orderNo == curBlock2->orderNo)
        {
            link.items.emplace_back(curBlock1, curBlock2);
            link.allocSize[0] += curBlock1->allocByApp;
            link.allocSize[1] += curBlock2->allocByApp;
            ++begin1;
            ++begin2;
        }
        else if (curBlock1->orderNo < curBlock2->orderNo)
        {
            link.items.emplace_back(curBlock1, nullptr);
            link.allocSize[0] += curBlock1->allocByApp;
            ++begin1;
        }
        else
        {
            link.items.emplace_back(nullptr, curBlock2);
            link.allocSize[1] += curBlock2->allocByApp;
            ++begin2;
        }
    }
    DVASSERT((begin1 == end1 && begin2 == end2) || (begin1 < end1 && begin2 == end2) || (begin1 == end1 && begin2 < end2));

    while (begin1 < end1)
    {
        const MMBlock* curBlock1 = ToPointer(*begin1);
        link.items.emplace_back(curBlock1, nullptr);
        link.allocSize[0] += curBlock1->allocByApp;
        ++begin1;
    }
    while (begin2 < end2)
    {
        const MMBlock* curBlock2 = ToPointer(*begin2);
        link.items.emplace_back(nullptr, curBlock2);
        link.allocSize[1] += curBlock2->allocByApp;
        ++begin2;
    }
    return link;
}

} // unnamed namespace

BlockLink::BlockLink()
{
    allocSize[0] = 0;
    allocSize[1] = 0;
    blockCount[0] = 0;
    blockCount[1] = 0;
    sourceSnapshots[0] = nullptr;
    sourceSnapshots[1] = nullptr;
}

BlockLink::BlockLink(BlockLink&& other)
    : items(std::move(other.items))
    , linkCount(other.linkCount)
{
    for (uint32 i = 0; i < 2; ++i)
    {
        allocSize[i] = other.allocSize[i];
        blockCount[i] = other.blockCount[i];
        sourceSnapshots[i] = other.sourceSnapshots[i];
    }
    other.linkCount = 0;
}

BlockLink& BlockLink::operator=(BlockLink&& other)
{
    if (this != &other)
    {
        items = std::move(other.items);
        linkCount = other.linkCount;

        for (uint32 i = 0; i < 2; ++i)
        {
            allocSize[i] = other.allocSize[i];
            blockCount[i] = other.blockCount[i];
            sourceSnapshots[i] = other.sourceSnapshots[i];
        }
        other.linkCount = 0;
    }
    return *this;
}

BlockLink BlockLink::CreateBlockLink(const MemorySnapshot* snapshot)
{
    DVASSERT(snapshot != nullptr);

    const Vector<MMBlock>& blocks = snapshot->MemoryBlocks();
    return CreateBlockLinkT(blocks, snapshot);
}

BlockLink BlockLink::CreateBlockLink(const DAVA::Vector<DAVA::MMBlock*>& blocks, const MemorySnapshot* snapshot)
{
    return CreateBlockLinkT(blocks, snapshot);
}

BlockLink BlockLink::CreateBlockLink(const MemorySnapshot* snapshot1, const MemorySnapshot* snapshot2)
{
    DVASSERT(snapshot1 != nullptr && snapshot2 != nullptr);

    const Vector<MMBlock>& blocks1 = snapshot1->MemoryBlocks();
    const Vector<MMBlock>& blocks2 = snapshot2->MemoryBlocks();
    return CreateBlockLinkT(blocks1, snapshot1, blocks2, snapshot2);
}

BlockLink BlockLink::CreateBlockLink(const DAVA::Vector<DAVA::MMBlock*>& blocks1, const MemorySnapshot* snapshot1,
                                     const DAVA::Vector<DAVA::MMBlock*>& blocks2, const MemorySnapshot* snapshot2)
{
    return CreateBlockLinkT(blocks1, snapshot1, blocks2, snapshot2);
}
