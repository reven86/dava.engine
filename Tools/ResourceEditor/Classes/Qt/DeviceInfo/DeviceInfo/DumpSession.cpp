#include "DumpSession.h"

#include "Debug/DVAssert.h"

using namespace DAVA;

DumpSession::Branch::Branch(DAVA::uint64 frameAddr)
    : parent(nullptr)
    , addr(frameAddr)
    , level(0)
    , allocSize(0)
    , nblocks(0)
{}

DumpSession::Branch::~Branch()
{
    for (auto x : children)
        delete x;
}

void DumpSession::Branch::AddChild(Branch* child)
{
    DVASSERT(child != nullptr);
    DVASSERT(FindInChildren(child).first == nullptr);
    DVASSERT(FindInChildren(child->Addr()).first == nullptr);

    children.push_back(child);
    child->parent = this;
    child->level = level + 1;
}

std::pair<DumpSession::Branch*, int> DumpSession::Branch::FindInChildren(Branch* child)
{
    std::pair<Branch*, int> result(nullptr, -1);
    auto i = std::find(children.begin(), children.end(), child);
    if (i != children.end())
    {
        result.first = *i;
        result.second = (int)std::distance(children.begin(), i);
    }
    return result;
}

std::pair<DumpSession::Branch*, int> DumpSession::Branch::FindInChildren(DAVA::uint64 frameAddr)
{
    std::pair<Branch*, int> result(nullptr, -1);
    auto i = std::find_if(children.begin(), children.end(), [frameAddr](Branch* o) -> bool {
        return o->Addr() == frameAddr;
    });
    if (i != children.end())
    {
        result.first = *i;
        result.second = (int)std::distance(children.begin(), i);
    }
    return result;
}

void DumpSession::Branch::RemoveChild(Branch* child)
{
    auto i = std::find(children.begin(), children.end(), child);
    if (i != children.end())
    {
        *i = nullptr;
    }
}

void DumpSession::Branch::RemoveNull()
{
    children.erase(std::remove(children.begin(), children.end(), nullptr), children.end());
}

DumpSession::Branch* DumpSession::Branch::FindRecursive(DAVA::uint64 frameAddr)
{
    if (frameAddr == addr)
        return this;
    for (auto x : children)
    {
        Branch* y = x->FindRecursive(frameAddr);
        if (y != nullptr)
            return y;
    }
    return nullptr;
}

void DumpSession::Branch::UpdateLevels()
{
    for (auto x : children)
    {
        x->level = level + 1;
        x->UpdateLevels();
    }
}

//////////////////////////////////////////////////////////////////////////
DumpSession::Branch* DumpSession::CreateTreeBack() const
{
    Branch* root = new Branch(0);
    symbolTable.IterateOverBacktraces([this, root](uint32 hash, const DAVA::Vector<uint64>& frames) -> void {
        AddFramesBack(root, hash, frames);
    });

    auto fn_sort = [this](const DumpSession::Branch* l, const DumpSession::Branch* r) -> bool {
        const String& sl = symbolTable.GetSymbol(l->Addr());
        const String& sr = symbolTable.GetSymbol(r->Addr());
        return sl < sr;
    };
    root->SortChildren(fn_sort);

    return root;
}

DumpSession::Branch* DumpSession::CreateTreeFwd() const
{
    Branch* root = new Branch(0);
    symbolTable.IterateOverBacktraces([this, root](uint32 hash, const DAVA::Vector<uint64>& frames) -> void {
        AddFramesFwd(root, hash, frames);
    });

    auto fn_sort = [this](const DumpSession::Branch* l, const DumpSession::Branch* r) -> bool {
        const String& sl = symbolTable.GetSymbol(l->Addr());
        const String& sr = symbolTable.GetSymbol(r->Addr());
        return sl < sr;
    };
    root->SortChildren(fn_sort);

    return root;
}

DumpSession::Branch* DumpSession::CreateTreeFwdEx() const
{
    Branch* root = new Branch(0);
    symbolTable.IterateOverBacktraces([this, root](uint32 hash, const DAVA::Vector<uint64>& frames) -> void {
        AddFramesFwd(root, hash, frames);
    });

    int n = root->ChildrenCount();
    for (int cur = 0;cur < n;++cur)
    {
        Branch* curBranch = root->Child(cur);
        uint64 curAddr = curBranch->Addr();
        for (int i = 0;i < n;++i)
        {
            if (i == cur)
                continue;
            Branch* ch = root->Child(i);
            if (ch != nullptr)
            {
                Branch* br = ch->FindRecursive(curAddr);
                if (br != nullptr)
                {
                    MergeBranches(br, curBranch);
                    root->RemoveChild(curBranch);
                    break;
                }
            }
        }
    }
    root->RemoveNull();
    root->UpdateLevels();

    auto fn_sort = [this](const DumpSession::Branch* l, const DumpSession::Branch* r) -> bool {
        const String& sl = symbolTable.GetSymbol(l->Addr());
        const String& sr = symbolTable.GetSymbol(r->Addr());
        return sl < sr;
    };
    root->SortChildren(fn_sort);

    return root;
}

void DumpSession::AddFramesBack(Branch* root, DAVA::uint32 hash, const DAVA::Vector<DAVA::uint64>& frames) const
{
    uint32 alloc = 0;
    uint32 count = 0;
    auto it = blockMap.find(hash);
    for (auto x : it->second)
    {
        alloc += x->allocByApp;
        count += 1;
    }
    root->AddValues(alloc, count);
    for (auto frame : frames)
    {
        auto x = root->FindInChildren(frame);
        Branch* next = x.first;
        if (next == nullptr)
        {
            next = new Branch(frame);
            root->AddChild(next);
        }
        next->AddValues(alloc, count);
        root = next;
    }
}

void DumpSession::AddFramesFwd(Branch* root, DAVA::uint32 hash, const DAVA::Vector<DAVA::uint64>& frames) const
{
    uint32 alloc = 0;
    uint32 count = 0;
    auto it = blockMap.find(hash);
    for (auto x : it->second)
    {
        alloc += x->allocByApp;
        count += 1;
    }
    root->AddValues(alloc, count);

    int n = (int)frames.size();
    for (int i = n - 1;i >= 0;--i)
    {
        uint64 frame = frames[i];
        auto x = root->FindInChildren(frame);
        Branch* next = x.first;
        if (next == nullptr)
        {
            next = new Branch(frame);
            root->AddChild(next);
        }
        next->AddValues(alloc, count);
        root = next;
    }
}

void DumpSession::MergeBranches(Branch* into, Branch* src) const
{
    into->AddValues(src->AllocSize(), src->NBlocks());

    int n = src->ChildrenCount();
    for (int i = 0;i < n;++i)
    {
        Branch* ch = src->Child(i);
        uint64 a = ch->Addr();
        Branch* p = into->FindInChildren(a).first;
        if (p != nullptr)
        {
            MergeBranches(p, ch);
        }
        else
        {
            into->AddChild(ch);
        }
    }
}

bool DumpSession::LoadDump(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (nullptr == file)
    {
        return false;
    }

    MMDump dumpHdr;
    size_t nread = fread(&dumpHdr, sizeof(dumpHdr), 1, file);
    DVASSERT(1 == nread);

    MMCurStat curStat;
    nread = fread(&curStat, sizeof(MMCurStat), 1, file);
    DVASSERT(1 == nread);

    fseek(file, curStat.size - sizeof(MMCurStat), SEEK_CUR);

    Vector<MMBlock> memoryBlocks;
    memoryBlocks.resize(dumpHdr.blockCount);
    nread = fread(&*memoryBlocks.begin(), sizeof(MMBlock), dumpHdr.blockCount, file);
    DVASSERT(nread == dumpHdr.blockCount);

    Vector<MMSymbol> symbols;
    symbols.resize(dumpHdr.symbolCount);
    nread = fread(&*symbols.begin(), sizeof(MMSymbol), dumpHdr.symbolCount, file);
    DVASSERT(nread == dumpHdr.symbolCount);

    const size_t bktraceSize = sizeof(MMBacktrace) + dumpHdr.bktraceDepth * sizeof(uint64);
    Vector<uint8> bktrace;
    bktrace.resize(bktraceSize * dumpHdr.bktraceCount);
    nread = fread(&*bktrace.begin(), 1, bktraceSize * dumpHdr.bktraceCount, file);
    DVASSERT(nread == bktraceSize * dumpHdr.bktraceCount);

    for (auto& sym : symbols)
    {
        if (sym.name[0] == '\0')// || strcmp(sym.name, "<redacted>") == 0)
            Snprintf(sym.name, COUNT_OF(sym.name), "%08llX", sym.addr);
        symbolTable.AddSymbol(sym.addr, sym.name);
    }

    const uint8* curOffset = bktrace.data();
    for (size_t i = 0, n = dumpHdr.bktraceCount;i < n;++i)
    {
        const MMBacktrace* curBktrace = reinterpret_cast<const MMBacktrace*>(curOffset);
        const uint64* frames = OffsetPointer<uint64>(curBktrace, sizeof(MMBacktrace));
        symbolTable.AddBacktrace(curBktrace->hash, frames, dumpHdr.bktraceDepth);
        curOffset += bktraceSize;
    }

    std::sort(memoryBlocks.begin(), memoryBlocks.end(), [](const MMBlock& l, const MMBlock& r) -> bool {
        return l.orderNo < r.orderNo;
    });

    blocks.swap(memoryBlocks);
    BuildBlockMap();
    return true;
}

void DumpSession::BuildBlockMap()
{
    for (MMBlock& curBlock : blocks)
    {
        auto iterAt = blockMap.find(curBlock.bktraceHash);
        if (iterAt == blockMap.end())
        {
            //Q_ASSERT(!bktraceTable.GetFrames(curBlock.backtraceHash).empty());
            iterAt = blockMap.emplace(curBlock.bktraceHash, DAVA::Vector<DAVA::MMBlock*>()).first;
        }
        iterAt->second.push_back(&curBlock);
    }
}
