#ifndef __MEMORYDUMPSESSION_H__
#define __MEMORYDUMPSESSION_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "BacktraceSymbolTable.h"

struct Branch
{
    Branch(const char* aName) : name(aName), parent(nullptr), level(0)
    {
        for (size_t i = 0;i < 2;++i)
        {
            diff[i] = 0;
            allocByApp[i] = 0;
            allocTotal[i] = 0;
            blockCount[i] = 0;
        }
    }
    ~Branch()
    {
        for (auto child : childBranches)
        {
            delete child;
        }
    }
    void AppendChild(Branch* child)
    {
        childBranches.push_back(child);
        child->parent = this;
        child->level = level + 1;
    }

    Branch* FindInChildren(const char* aName) const
    {
        for (auto child : childBranches)
        {
            if (child->name == aName)
            {
                return child;
            }
        }
        return nullptr;
    }
    int ChildIndex(Branch* ch) const
    {
        for (int i = 0, n = int(childBranches.size());i < n;++i)
        {
            if (childBranches[i] == ch)
                return i;
        }
        return -1;
    }
    void UpdateStat(size_t index, size_t aApp, size_t aTotal, size_t nblocks)
    {
        allocByApp[index] += aApp;
        allocTotal[index] += aTotal;
        blockCount[index] += nblocks;
        Branch* p = parent;
        while (p != nullptr)
        {
            p->allocByApp[index] += aApp;
            p->allocTotal[index] += aTotal;
            p->blockCount[index] += nblocks;
            p = p->parent;
        }
    }

    const char* name;
    Branch* parent;
    DAVA::Vector<Branch*> childBranches;
    int level;
    int diff[2];

    size_t allocByApp[2];
    size_t allocTotal[2];
    size_t blockCount[2];

    DAVA::Vector<const DAVA::MMBlock*> blocks[2];
};

class MemoryDumpSession
{
    struct DumpData
    {
        DumpData(DAVA::Vector<DAVA::MMBlock>&& blocks);

        void BuildBlockMap();

        DAVA::Vector<DAVA::MMBlock> memoryBlocks;
        DAVA::Map<DAVA::uint32, DAVA::Vector<const DAVA::MMBlock*>> blockMap;
    };

public:
    MemoryDumpSession();
    ~MemoryDumpSession();

    size_t DumpCount() const { return dumpData.size(); }
    const BacktraceSymbolTable& SymbolTable() const { return bktraceTable; }

    Branch* CreateBranch(const char* fromName, size_t dumpIndex) const;
    Branch* CreateBranch(const DAVA::Vector<const char*>& names, size_t dumpIndex) const;
    Branch* CreateBranchDiff(const char* fromName) const;
    Branch* CreateBranchDiff(const DAVA::Vector<const char*>& names) const;

    Branch* CreateIntersection() const;

    Branch* FindPath(Branch* branch, const DAVA::Vector<const char*>& frames) const;

    DAVA::Vector<const DAVA::MMBlock*> GetBlocks(Branch* branch, size_t dumpIndex) const;
    DAVA::Vector<const DAVA::MMBlock*> GetBlocks(size_t dumpIndex) const {
        DAVA::Vector<const DAVA::MMBlock*> r;
        r.reserve(dumpData[dumpIndex]->memoryBlocks.size());
        for (auto& x : dumpData[dumpIndex]->memoryBlocks)
            r.push_back(&x);
        return r;
    }

    bool LoadDump(const char* filename);

private:
    void GetBlocks(Branch* branch, size_t dumpIndex, DAVA::Vector<const DAVA::MMBlock*>& dst) const;

    Branch* CreateBranchFromDump(Branch* root, const char* fromName, size_t dumpIndex) const;
    Branch* CreateBranchFromDump(Branch* root, const DAVA::Vector<const char*>& names, size_t dumpIndex) const;
    Branch* CreateBranchDiffFromDump(Branch* root, const char* fromName) const;
    Branch* CreateBranchDiffFromDump(Branch* root, const DAVA::Vector<const char*>& names) const;
    Branch* BuildPath(Branch* parent, size_t startFrame, const DAVA::Vector<const DAVA::char8*>& frames, size_t dumpIndex) const;
    size_t FindNameInBacktrace(const char* name, const DAVA::Vector<const DAVA::char8*>& frames) const;
    size_t FindNamesInBacktrace(const DAVA::Vector<const char*>& names, const DAVA::Vector<const DAVA::char8*>& frames) const;

private:
    BacktraceSymbolTable bktraceTable;
    DAVA::Vector<DumpData*> dumpData;

    //DAVA::Vector<const DAVA::MMBlock*> memBlocks[2];
};

#endif  // __MEMORYDUMPSESSION_H__
