#include "RemoteTool/Private/MemoryTool/Branch.h"

#include <Debug/DVAssert.h>

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
