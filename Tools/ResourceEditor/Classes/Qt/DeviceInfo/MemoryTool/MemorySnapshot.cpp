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

#include "Base/RefPtr.h"
#include "Debug/DVAssert.h"
#include "FileSystem/File.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

using namespace DAVA;

MemorySnapshot::MemorySnapshot(MemorySnapshot&& other)
    : profilingSession(std::move(other.profilingSession))
    , fileName(std::move(other.fileName))
    , timestamp(std::move(other.timestamp))
    , blockCount(std::move(other.blockCount))
    , symbolCount(std::move(other.symbolCount))
    , bktraceCount(std::move(other.bktraceCount))
    , totalSize(std::move(other.totalSize))
    , symbolTable(std::move(other.symbolTable))
    , mblocks(std::move(other.mblocks))
    , blockMap(std::move(other.blockMap))
{}

MemorySnapshot& MemorySnapshot::operator = (MemorySnapshot&& other)
{
    if (this != &other)
    {
        profilingSession = std::move(other.profilingSession);
        fileName = std::move(other.fileName);
        timestamp = std::move(other.timestamp);
        blockCount = std::move(other.blockCount);
        symbolCount = std::move(other.symbolCount);
        bktraceCount = std::move(other.bktraceCount);
        totalSize = std::move(other.totalSize);
        symbolTable = std::move(other.symbolTable);
        mblocks = std::move(other.mblocks);
        blockMap = std::move(other.blockMap);
    }
    return *this;
}

bool MemorySnapshot::Load(BacktraceSymbolTable* symbolTable_)
{
    DVASSERT(symbolTable_ != nullptr);
    symbolTable = symbolTable_;

    if (LoadFile())
    {
        BuildBlockMap();
        return true;
    }
    return false;
}

void MemorySnapshot::Unload()
{
    symbolTable = nullptr;
    blockMap.clear();
    mblocks.clear();
    mblocks.shrink_to_fit();
}

Branch* MemorySnapshot::CreateBranch(const Vector<const String*>& startNames) const
{
    DVASSERT(IsLoaded());
    
    Branch* root = new Branch;
    for (auto& pair : blockMap)
    {
        const Vector<const String*>* bktraceNames = symbolTable->GetBacktraceSymbols(pair.first);
        const Vector<MMBlock*>& blocks = pair.second;
        DVASSERT(bktraceNames != nullptr);
        
        if (!blocks.empty() && !bktraceNames->empty())
        {
            int startFrame = FindNamesInBacktrace(startNames, *bktraceNames);
            if (startFrame >= 0)
            {
                Branch* leaf = BuildPath(root, startFrame, *bktraceNames);
                
                // Append memory blocks to leaf
                uint32 allocByApp = 0;
                uint32 pools = 0;
                uint32 tags = 0;
                uint32 nblocks = static_cast<uint32>(blocks.size());
                leaf->mblocks.reserve(leaf->mblocks.size() + nblocks);
                for (auto& x : blocks)
                {
                    allocByApp += x->allocByApp;
                    pools |= x->pool;
                    tags |= x->tags;
                    leaf->mblocks.emplace_back(x);
                }
                leaf->UpdateStat(allocByApp, nblocks, pools, tags);
            }
        }
    }
    // Sort children by symbol name
    root->SortChildren([](const Branch* l, const Branch* r) -> bool { return *l->name < *r->name; });
    return root;
}

Branch* MemorySnapshot::BuildPath(Branch* parent, int startFrame, const Vector<const String*>& bktraceNames) const
{
    do {
        const String* curName = bktraceNames[startFrame];
        Branch* branch = parent->FindInChildren(curName);
        if (nullptr == branch)
        {
            branch = new Branch(curName);
            parent->AppendChild(branch);
        }
        parent = branch;
    } while (startFrame --> 0);
    return parent;
}

int MemorySnapshot::FindNamesInBacktrace(const Vector<const String*>& namesToFind, const Vector<const String*>& bktraceNames) const
{
    int index = static_cast<int>(bktraceNames.size() - 1);
    do {
        auto iterFind = std::find(namesToFind.begin(), namesToFind.end(), bktraceNames[index]);
        if (iterFind != namesToFind.end())
        {
            return index;
        }
    } while (index --> 0);
    return -1;
}

bool MemorySnapshot::LoadFile()
{
    RefPtr<File> file(File::Create(fileName, File::OPEN | File::READ));
    if (file.Valid())
    {
        // Load and check file header
        MMSnapshot msnapshot;
        size_t nread = file->Read(&msnapshot);
        if (sizeof(MMSnapshot) == nread || file->GetSize() == msnapshot.size)
        {
            // Move pointer to data
            file->Seek(msnapshot.dataOffset, File::SEEK_FROM_START);

            const uint32 bktraceSize = sizeof(MMBacktrace) + msnapshot.bktraceDepth * sizeof(uint64);
            Vector<MMBlock> blocks(msnapshot.blockCount, MMBlock());
            Vector<MMSymbol> symbols(msnapshot.symbolCount, MMSymbol());
            Vector<uint8> bktrace(bktraceSize * msnapshot.bktraceCount, 0);

            // Read memory blocks
            nread = file->Read(&*blocks.begin(), msnapshot.blockCount * sizeof(MMBlock));
            if (msnapshot.blockCount * sizeof(MMBlock) == nread)
            {
                // Read symbols
                nread = file->Read(&*symbols.begin(), msnapshot.symbolCount * sizeof(MMSymbol));
                if (msnapshot.symbolCount * sizeof(MMSymbol) == nread)
                {
                    // Read backtraces
                    nread = file->Read(&*bktrace.begin(), bktraceSize * msnapshot.bktraceCount);
                    if (bktraceSize * msnapshot.bktraceCount == nread)
                    {
                        const uint8* curOffset = bktrace.data();
                        for (size_t i = 0, n = msnapshot.bktraceCount;i < n;++i)
                        {
                            const MMBacktrace* curBktrace = reinterpret_cast<const MMBacktrace*>(curOffset);
                            const uint64* frames = OffsetPointer<uint64>(curBktrace, sizeof(MMBacktrace));
                            symbolTable->AddBacktrace(curBktrace->hash, frames, msnapshot.bktraceDepth);
                            curOffset += bktraceSize;
                        }

                        for (MMBlock& x : blocks)
                        {
                            x.pool = 1 << x.pool;
                        }
                        std::sort(blocks.begin(), blocks.end(), [](const MMBlock& l, const MMBlock& r) -> bool { return l.orderNo < r.orderNo; });
                        mblocks.swap(blocks);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void MemorySnapshot::BuildBlockMap()
{
    Map<uint32, Vector<MMBlock*>> map;
    for (MMBlock& curBlock : mblocks)
    {
        auto iter = map.find(curBlock.bktraceHash);
        if (iter == map.end())
        {
            iter = map.emplace(curBlock.bktraceHash, Vector<MMBlock*>()).first;
        }
        iter->second.emplace_back(&curBlock);
    }
    blockMap.swap(map);
}
