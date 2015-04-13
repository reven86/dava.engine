#include <QDebug>

#include "MemoryDumpSession.h"

using namespace DAVA;

MemoryDumpSession::MemoryDumpSession()
{}

MemoryDumpSession::~MemoryDumpSession()
{
    for (auto p : dumpData)
    {
        delete p;
    }
}

Branch* MemoryDumpSession::CreateBranch(const char* fromName, size_t dumpIndex) const
{
    Q_ASSERT(fromName != nullptr);
    Q_ASSERT(dumpIndex < dumpData.size());
    Branch* root = new Branch(nullptr);
    return CreateBranchFromDump(root, fromName, dumpIndex);
}

Branch* MemoryDumpSession::CreateBranch(const DAVA::Vector<const char*>& names, size_t dumpIndex) const
{
    Branch* root = new Branch(nullptr);
    return CreateBranchFromDump(root, names, dumpIndex);
}

Branch* MemoryDumpSession::CreateBranchDiff(const char* fromName) const
{
    Q_ASSERT(fromName != nullptr);
    Q_ASSERT(dumpData.size() >= 2);
    if (dumpData.size() == 2)
    {
        Branch* root = new Branch(nullptr);
        return CreateBranchDiffFromDump(root, fromName);
    }
    return nullptr;
}

Branch* MemoryDumpSession::CreateBranchDiff(const DAVA::Vector<const char*>& names) const
{
    if (dumpData.size() == 2)
    {
        Branch* root = new Branch(nullptr);
        return CreateBranchDiffFromDump(root, names);
    }
    return nullptr;
}

DAVA::Vector<const DAVA::MMBlock*> MemoryDumpSession::GetBlocks(Branch* branch, size_t dumpIndex) const
{
    DAVA::Vector<const DAVA::MMBlock*> result;
    result.reserve(branch->blocks[dumpIndex].size());
    GetBlocks(branch, dumpIndex, result);
    return result;
}

void MemoryDumpSession::GetBlocks(Branch* branch, size_t dumpIndex, DAVA::Vector<const DAVA::MMBlock*>& dst) const
{
    for (Branch* child : branch->childBranches)
    {
        GetBlocks(child, dumpIndex, dst);
    }
    if (!branch->blocks[dumpIndex].empty())
    {
        size_t ndst = dst.size();
        size_t nblocks = branch->blocks[dumpIndex].size();
        dst.reserve(ndst + nblocks);
        for (auto x : branch->blocks[dumpIndex])
        {
            dst.push_back(x);
        }
    }
}

Branch* MemoryDumpSession::FindPath(Branch* branch, const DAVA::Vector<const char*>& frames) const
{
    for (Branch* child : branch->childBranches)
    {
        size_t startFrame = FindNameInBacktrace(child->name, frames);
        if (startFrame != size_t(-1))
        {
            while (startFrame > 0)
            {
                startFrame -= 1;
                const char8* curName = frames[startFrame];
                Branch* p = child->FindInChildren(curName);
                if (p == nullptr)
                {
                    child = nullptr;
                    break;
                }
                child = p;
            }

            if (child != nullptr)
                return child;
        }
    }
    return nullptr;
}

Branch* MemoryDumpSession::CreateIntersection() const
{
    DAVA::Vector<DAVA::MMBlock>& v1 = dumpData[0]->memoryBlocks;
    DAVA::Vector<DAVA::MMBlock>& v2 = dumpData[1]->memoryBlocks;
    size_t n = std::min(v1.size(), v2.size());
    Vector<const MMBlock*> v;
    v.reserve(n);

    class out_iter : public std::iterator < std::output_iterator_tag, void, void, void, void >
    {
    public:
        out_iter(Vector<const MMBlock*>& vref) : v(vref) {}
        out_iter& operator = (const MMBlock& b) {
            v.push_back(&b);
            return *this;
        }
        out_iter& operator*() { return *this; }
        out_iter& operator++() {return *this; }
        out_iter& operator++(int) { return *this; }
    private:
        Vector<const MMBlock*>& v;
    };
    std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                          out_iter(v),
                          [](const MMBlock& l, const MMBlock& r) -> bool {
        return l.orderNo < r.orderNo;
    });
    return nullptr;
}

Branch* MemoryDumpSession::CreateBranchFromDump(Branch* root, const char* fromName, size_t dumpIndex) const
{
    DumpData* dump = dumpData[dumpIndex];
    for (auto& pair : dump->blockMap)
    {
        auto& frames = bktraceTable.GetFrames(pair.first);
        auto& blocks = pair.second;
        Q_ASSERT(!frames.empty());

        if (!blocks.empty())
        {
            size_t startFrame = FindNameInBacktrace(fromName, frames);
            if (startFrame != size_t(-1))
            {
                Branch* leaf = BuildPath(root, startFrame, frames, dumpIndex);

                size_t allocByApp = 0;
                size_t allocTotal = 0;
                size_t blockCount = blocks.size();
                size_t n = leaf->blocks[dumpIndex].size();
                leaf->blocks[dumpIndex].reserve(n + blockCount);
                for (auto x : blocks)
                {
                    allocByApp += x->allocByApp;
                    allocTotal += x->allocTotal;
                    leaf->blocks[dumpIndex].push_back(x);
                }
                leaf->UpdateStat(dumpIndex, allocByApp, allocTotal, blockCount);
            }
        }
    }
    return root;
}

Branch* MemoryDumpSession::CreateBranchFromDump(Branch* root, const DAVA::Vector<const char*>& names, size_t dumpIndex) const
{
    DumpData* dump = dumpData[dumpIndex];
    for (auto& pair : dump->blockMap)
    {
        auto& frames = bktraceTable.GetFrames(pair.first);
        auto& blocks = pair.second;
        Q_ASSERT(!frames.empty());

        if (!blocks.empty())
        {
            size_t startFrame = FindNamesInBacktrace(names, frames);
            if (startFrame != size_t(-1))
            {
                Branch* leaf = BuildPath(root, startFrame, frames, dumpIndex);

                size_t allocByApp = 0;
                size_t allocTotal = 0;
                size_t blockCount = blocks.size();
                size_t n = leaf->blocks[dumpIndex].size();
                leaf->blocks[dumpIndex].reserve(n + blockCount);
                for (auto x : blocks)
                {
                    allocByApp += x->allocByApp;
                    allocTotal += x->allocTotal;
                    leaf->blocks[dumpIndex].push_back(x);
                }
                leaf->UpdateStat(dumpIndex, allocByApp, allocTotal, blockCount);
            }
        }
    }
    return root;
}

Branch* MemoryDumpSession::CreateBranchDiffFromDump(Branch* root, const char* fromName) const
{
    CreateBranchFromDump(root, fromName, 0);
    CreateBranchFromDump(root, fromName, 1);
    return root;
}

Branch* MemoryDumpSession::CreateBranchDiffFromDump(Branch* root, const DAVA::Vector<const char*>& names) const
{
    CreateBranchFromDump(root, names, 0);
    CreateBranchFromDump(root, names, 1);
    return root;
}

Branch* MemoryDumpSession::BuildPath(Branch* parent, size_t startFrame, const DAVA::Vector<const DAVA::char8*>& frames, size_t dumpIndex) const
{
    do {
        const char8* curName = frames[startFrame];
        Branch* branch = parent->FindInChildren(curName);
        if (nullptr == branch)
        {
            branch = new Branch(curName);
            branch->diff[dumpIndex] = 1;
            parent->AppendChild(branch);
        }
        else
            branch->diff[dumpIndex] = 1;
        parent = branch;
    } while (startFrame --> 0);
    return parent;
}

size_t MemoryDumpSession::FindNameInBacktrace(const char* name, const DAVA::Vector<const DAVA::char8*>& frames) const
{
    size_t index = frames.size() - 1;
    do {
        if (frames[index] == name)
        {
            return index;
        }
    } while (index --> 0);
    return size_t(-1);
}

size_t MemoryDumpSession::FindNamesInBacktrace(const DAVA::Vector<const char*>& names, const DAVA::Vector<const DAVA::char8*>& frames) const
{
    size_t n = frames.size() - 1;
    do {
        auto iterFind = std::find(names.begin(), names.end(), frames[n]);
        if (iterFind != names.end())
        {
            return n;
        }
    } while (n-- > 0);
    return size_t(-1);
}

bool MemoryDumpSession::LoadDump(const char* filename)
{
    Q_ASSERT(dumpData.size() < 2);

    FILE* file = fopen(filename, "rb");
    if (nullptr == file)
    {
        return false;
    }

    MMDump dumpHdr;
    size_t nread = fread(&dumpHdr, sizeof(dumpHdr), 1, file);
    Q_ASSERT(1 == nread);

    MMCurStat curStat;
    nread = fread(&curStat, sizeof(MMCurStat), 1, file);
    Q_ASSERT(1 == nread);

    fseek(file, curStat.size - sizeof(MMCurStat), SEEK_CUR);

    Vector<MMBlock> memoryBlocks;
    memoryBlocks.resize(dumpHdr.blockCount);
    nread = fread(&*memoryBlocks.begin(), sizeof(MMBlock), dumpHdr.blockCount, file);
    Q_ASSERT(nread == dumpHdr.blockCount);

    Vector<MMSymbol> symbols;
    symbols.resize(dumpHdr.symbolCount);
    nread = fread(&*symbols.begin(), sizeof(MMSymbol), dumpHdr.symbolCount, file);
    Q_ASSERT(nread == dumpHdr.symbolCount);

    const size_t bktraceSize = sizeof(MMBacktrace) +  dumpHdr.bktraceDepth * sizeof(uint64);
    Vector<uint8> bktrace;
    bktrace.resize(bktraceSize * dumpHdr.bktraceCount);
    nread = fread(&*bktrace.begin(), 1, bktraceSize * dumpHdr.bktraceCount, file);
    Q_ASSERT(nread == bktraceSize * dumpHdr.bktraceCount);

    for (auto& sym : symbols)
    {
        if (sym.name[0] == '\0')// || strcmp(sym.name, "<redacted>") == 0)
            Snprintf(sym.name, COUNT_OF(sym.name), "%08llX", sym.addr);
        bktraceTable.AddSymbol(sym.addr, sym.name);
    }

    const uint8* curOffset = bktrace.data();
    for (size_t i = 0, n = dumpHdr.bktraceCount;i < n;++i)
    {
        const MMBacktrace* curBktrace = reinterpret_cast<const MMBacktrace*>(curOffset);
        const uint64* frames = OffsetPointer<uint64>(curBktrace, sizeof(MMBacktrace));
        bktraceTable.AddBacktrace(curBktrace->hash, frames, dumpHdr.bktraceDepth);
        curOffset += bktraceSize;
    }

    std::sort(memoryBlocks.begin(), memoryBlocks.end(), [](const MMBlock& l, const MMBlock& r) -> bool {
        return l.orderNo < r.orderNo;
    });

    DumpData* newDump = new DumpData(std::forward<DAVA::Vector<DAVA::MMBlock>>(memoryBlocks));
    dumpData.push_back(newDump);
    return true;
}

MemoryDumpSession::DumpData::DumpData(Vector<MMBlock>&& blocks)
    : memoryBlocks(std::move(blocks))
{
    BuildBlockMap();
}

void MemoryDumpSession::DumpData::BuildBlockMap()
{
    for (MMBlock& curBlock : memoryBlocks)
    {
        auto iterAt = blockMap.find(curBlock.bktraceHash);
        if (iterAt == blockMap.end())
        {
            //Q_ASSERT(!bktraceTable.GetFrames(curBlock.backtraceHash).empty());
            iterAt = blockMap.emplace(curBlock.bktraceHash, DAVA::Vector<const DAVA::MMBlock*>()).first;
        }
        iterAt->second.push_back(&curBlock);
    }
}
