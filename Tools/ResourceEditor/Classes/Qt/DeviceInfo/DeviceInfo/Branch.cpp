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

#include "Branch.h"

using namespace DAVA;

Branch::Branch(const char* aName)
    : name(aName)
{}

Branch::~Branch()
{
    for (auto child : children)
    {
        delete child;
    }
}

Branch* Branch::FindInChildren(const char* name_) const
{
    for (auto child : children)
    {
        if (child->name == name_)
        {
            return child;
        }
    }
    return nullptr;
}

int Branch::ChildIndex(Branch* child) const
{
    for (int i = 0, n = static_cast<size_t>(children.size());i < n;++i)
    {
        if (children[i] == child)
        {
            return i;
        }
    }
    return -1;
}

void Branch::AppendChild(Branch* child)
{
    DVASSERT(child != nullptr);
    DVASSERT(ChildIndex(child) < 0);

    children.push_back(child);
    child->parent = this;
    child->level = level + 1;
}

void Branch::UpdateStat(DAVA::uint32 allocSize, DAVA::uint32 blockCount)
{
    allocByApp += allocSize;
    nblocks += blockCount;

    Branch* p = parent;
    while (p != nullptr)
    {
        p->allocByApp += allocSize;
        p->nblocks += blockCount;
        p = p->parent;
    }
}

Vector<MMBlock> Branch::GetMemoryBlocks() const
{
    Vector<MMBlock> result;
    if (nblocks > 0)
    {
        result.reserve(nblocks);
        CollectBlocks(this, result);

        std::sort(result.begin(), result.end(), [](const MMBlock& l, const MMBlock& r) -> bool {
            return l.orderNo < r.orderNo;
        });
    }
    return result;
}

void Branch::CollectBlocks(const Branch* branch, Vector<MMBlock>& target)
{
    for (auto& block : branch->mblocks)
    {
        target.emplace_back(block);
    }
    for (auto child : branch->children)
    {
        CollectBlocks(child, target);
    }
}
