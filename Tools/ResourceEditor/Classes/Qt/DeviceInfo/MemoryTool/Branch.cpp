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

#include "Qt/DeviceInfo/MemoryTool/Branch.h"

using namespace DAVA;

Branch::~Branch()
{
    for (auto child : children)
    {
        delete child;
    }
}

Branch* Branch::FindInChildren(const String* name_) const
{
    auto iter = std::find_if(children.cbegin(), children.cend(), [name_](const Branch* o) -> bool {
        return o->name == name_;
    });
    return iter != children.cend() ? *iter : nullptr;
}

int Branch::ChildIndex(Branch* child) const
{
    auto iter = std::find_if(children.cbegin(), children.cend(), [child](const Branch* o) -> bool {
        return o == child;
    });
    return iter != children.cend() ? static_cast<int>(std::distance(children.cbegin(), iter)) : -1;
}

void Branch::AppendChild(Branch* child)
{
    DVASSERT(child != nullptr);
    DVASSERT(ChildIndex(child) < 0);

    children.push_back(child);
    child->parent = this;
    child->level = level + 1;
}

void Branch::UpdateStat(uint32 allocSize, uint32 blockCount, uint32 pools, uint32 tags)
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

Vector<MMBlock*> Branch::GetMemoryBlocks() const
{
    Vector<MMBlock*> result;
    if (nblocks > 0)
    {
        result.reserve(nblocks);
        CollectBlocks(this, result);

        std::sort(result.begin(), result.end(), [](const MMBlock* l, const MMBlock* r) -> bool {
            return l->orderNo < r->orderNo;
        });
    }
    return result;
}

void Branch::CollectBlocks(const Branch* branch, Vector<MMBlock*>& target)
{
    target.insert(target.end(), branch->mblocks.cbegin(), branch->mblocks.cend());
    for (auto child : branch->children)
    {
        CollectBlocks(child, target);
    }
}
