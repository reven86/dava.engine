#pragma once

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "SymbolTable.h"

class DumpSession
{
public:
    class Branch
    {
    public:
        Branch(DAVA::uint64 frameAddr);
        ~Branch();
        void AddChild(Branch* child);
        std::pair<Branch*, int> FindInChildren(Branch* child);
        std::pair<Branch*, int> FindInChildren(DAVA::uint64 frameAddr);
        Branch* Child(int index) const { return children[index]; }

        Branch* FindRecursive(DAVA::uint64 frameAddr);
        void RemoveChild(Branch* child);
        void RemoveNull();

        DAVA::uint64 Addr() const { return addr; }
        Branch* Parent() const { return parent; }
        int ChildrenCount() const { return static_cast<int>(children.size()); }
        bool HasChildren() const { return !children.empty(); }
        int Level() const { return level - 1; }
        void UpdateLevels();
        void AddValues(DAVA::uint32 alloc, DAVA::uint32 count) {
            allocSize += alloc;
            nblocks += count;
        }

        DAVA::uint32 AllocSize() const { return allocSize; }
        DAVA::uint32 NBlocks() const { return nblocks; }

        template<typename F>
        void SortChildren(F fn)
        {
            std::sort(children.begin(), children.end(), fn);
            for (auto x : children)
                x->SortChildren(fn);
        }

    private:
        DAVA::uint64 addr;
        int level;
        Branch* parent;
        DAVA::Vector<Branch*> children;

        DAVA::uint32 allocSize;
        DAVA::uint32 nblocks;
    };

public:

    Branch* CreateTreeBack() const;
    Branch* CreateTreeFwd() const;
    Branch* CreateTreeFwdEx() const;
    bool LoadDump(const char* filename);
    const SymbolTable& Symbols() const { return symbolTable; }

private:
    void AddFramesBack(Branch* root, DAVA::uint32 hash, const DAVA::Vector<DAVA::uint64>& frames) const;
    void AddFramesFwd(Branch* root, DAVA::uint32 hash, const DAVA::Vector<DAVA::uint64>& frames) const;

    void MergeBranches(Branch* into, Branch* src) const;

    void BuildBlockMap();

protected:
    SymbolTable symbolTable;
    DAVA::Vector<DAVA::MMBlock> blocks;

    DAVA::Map<DAVA::uint32, DAVA::Vector<DAVA::MMBlock*>> blockMap;
};
