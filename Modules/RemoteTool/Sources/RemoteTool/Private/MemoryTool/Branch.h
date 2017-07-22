#pragma once

#include <Base/BaseTypes.h>
#include <MemoryManager/MemoryManagerTypes.h>

struct Branch final
{
    Branch(const DAVA::String* name = nullptr);
    ~Branch();

    Branch* FindInChildren(const DAVA::String* name) const;
    int ChildIndex(Branch* child) const;

    void AppendChild(Branch* child);
    void UpdateStat(DAVA::uint32 allocSize, DAVA::uint32 blockCount, DAVA::uint32 pools, DAVA::uint32 tags);

    // Get memory blocks from all children
    DAVA::Vector<DAVA::MMBlock*> GetMemoryBlocks() const;

    template <typename F>
    void SortChildren(F fn);

    int level = 0; // Level in tree hierarchy - for convenience in Qt models
    const DAVA::String* name = nullptr; // Function name
    Branch* parent = nullptr;
    DAVA::Vector<Branch*> children;

    DAVA::uint32 allocByApp = 0; // Total allocated size in branch including children
    DAVA::uint32 nblocks = 0; // Total block count in branch including children
    DAVA::Vector<DAVA::MMBlock*> mblocks; // Memory blocks belonging to leaf branch

private:
    static void CollectBlocks(const Branch* branch, DAVA::Vector<DAVA::MMBlock*>& target);
};

//////////////////////////////////////////////////////////////////////////
inline Branch::Branch(const DAVA::String* name_)
    : name(name_)
{
}

template <typename F>
inline void Branch::SortChildren(F fn)
{
    std::sort(children.begin(), children.end(), fn);
    for (Branch* child : children)
    {
        child->SortChildren(fn);
    }
}
