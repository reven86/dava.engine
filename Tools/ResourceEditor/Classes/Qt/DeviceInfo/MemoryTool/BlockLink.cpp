/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Debug/DVAssert.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"

using namespace DAVA;

BlockLink::BlockLink(BlockLink&& other)
    : items(std::move(other.items))
    , linkCount(other.linkCount)
{
    for (uint32 i = 0;i < linkCount;++i)
    {
        allocSize[i] = other.allocSize[i];
        blockCount[i] = other.blockCount[i];
    }
    other.linkCount = 0;
}

BlockLink& BlockLink::operator = (BlockLink&& other)
{
    if (this != &other)
    {
        items = std::move(other.items);
        linkCount = other.linkCount;

        for (uint32 i = 0;i < linkCount;++i)
        {
            allocSize[i] = other.allocSize[i];
            blockCount[i] = other.blockCount[i];
        }
        other.linkCount = 0;
    }
    return *this;
}

BlockLink BlockLink::CreateBlockLink(const MemorySnapshot* snapshot)
{
    DVASSERT(snapshot != nullptr);

    const Vector<MMBlock>& blocks = snapshot->MemoryBlocks();

    BlockLink link;
    link.linkCount = 1;
    link.allocSize[0] = 0;
    link.allocSize[1] = 0;
    link.blockCount[0] = static_cast<uint32>(blocks.size());
    link.blockCount[1] = 0;
    link.items.reserve(link.blockCount[0]);

    for (const MMBlock& curBlock : blocks)
    {
        const MMBlock* prev = nullptr;
        if (link.items.size() > 0)
        {
            prev = BlockLink::AnyBlock(link.items.back());
            DVASSERT(prev->orderNo > curBlock.orderNo);
        }
        link.items.emplace_back(&curBlock, nullptr);
        link.allocSize[0] += curBlock.allocByApp;
    }
    return link;
}

BlockLink BlockLink::CreateBlockLink(const MemorySnapshot* snapshot1, const MemorySnapshot* snapshot2)
{
    DVASSERT(snapshot1 != nullptr && snapshot2 != nullptr);

    // Memory blocks are sorted by orderNo in descending order
    const Vector<MMBlock>& blocks1 = snapshot1->MemoryBlocks();
    const Vector<MMBlock>& blocks2 = snapshot2->MemoryBlocks();

    BlockLink link;
    link.linkCount = 2;
    link.allocSize[0] = 0;
    link.allocSize[1] = 0;
    link.blockCount[0] = static_cast<uint32>(blocks1.size());
    link.blockCount[1] = static_cast<uint32>(blocks2.size());
    link.items.reserve(std::max(link.blockCount[0], link.blockCount[1]));

    auto begin1 = blocks1.begin();
    auto end1 = blocks1.end();
    auto begin2 = blocks2.begin();
    auto end2 = blocks2.end();
    while (begin1 != end1 && begin2 != end2)
    {
        const MMBlock& curBlock1 = *begin1;
        const MMBlock& curBlock2 = *begin2;
        if (curBlock1.orderNo == curBlock2.orderNo)
        {
            link.items.emplace_back(&curBlock1, &curBlock2);
            link.allocSize[0] += curBlock1.allocByApp;
            link.allocSize[1] += curBlock2.allocByApp;
            ++begin1;
            ++begin2;
        }
        else if (curBlock1.orderNo > curBlock2.orderNo)
        {
            link.items.emplace_back(&curBlock1, nullptr);
            link.allocSize[0] += curBlock1.allocByApp;
            ++begin1;
        }
        else
        {
            link.items.emplace_back(nullptr, &curBlock2);
            link.allocSize[1] += curBlock2.allocByApp;
            ++begin2;
        }
    }
    DVASSERT((begin1 == end1 && begin2 == end2) || (begin1 < end1 && begin2 == end2) || (begin1 == end1 && begin2 < end2));

    while (begin1 < end1)
    {
        const MMBlock& curBlock1 = *begin1;
        link.items.emplace_back(&curBlock1, nullptr);
        link.allocSize[0] += curBlock1.allocByApp;
        ++begin1;
    }
    while (begin2 < end2)
    {
        const MMBlock& curBlock2 = *begin2;
        link.items.emplace_back(nullptr, &curBlock2);
        link.allocSize[1] += curBlock2.allocByApp;
        ++begin2;
    }
    return link;
}
