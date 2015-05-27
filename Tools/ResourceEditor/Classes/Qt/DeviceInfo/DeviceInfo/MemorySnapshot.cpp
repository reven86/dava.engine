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

#include "Branch.h"
#include "BacktraceSymbolTable.h"
#include "MemorySnapshot.h"

using namespace DAVA;

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

Branch* MemorySnapshot::CreateBranch(const DAVA::Vector<const char*>& startNames) const
{
    DVASSERT(IsLoaded());
    
    Branch* root = new Branch(nullptr);
    for (auto& pair : blockMap)
    {
        auto& frames = symbolTable->GetFrames(pair.first);
        auto& blocks = pair.second;
        
        if (!blocks.empty() && !frames.empty())
        {
            int startFrame = FindNamesInBacktrace(startNames, frames);
            if (startFrame >= 0)
            {
                Branch* leaf = BuildPath(root, startFrame, frames);
                
                // Append memory blocks to leaf
                uint32 allocByApp = 0;
                leaf->mblocks.reserve(leaf->mblocks.size() + blocks.size());
                for (auto& x : blocks)
                {
                    allocByApp += x.allocByApp;
                    leaf->mblocks.emplace_back(x);
                }
                leaf->UpdateStat(allocByApp, static_cast<uint32>(blocks.size()));
            }
        }
    }
    // Sort children by symbol name
    root->SortChildren([](const Branch* l, const Branch* r) -> bool {
        return strcmp(l->name, r->name) < 0;
    });
    return root;
}

Branch* MemorySnapshot::BuildPath(Branch* parent, int startFrame, const DAVA::Vector<const char*>& frames) const
{
    do {
        const char* curName = frames[startFrame];
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

int MemorySnapshot::FindNamesInBacktrace(const DAVA::Vector<const char*>& names, const DAVA::Vector<const char*>& frames) const
{
    int index = static_cast<int>(frames.size() - 1);
    do {
        auto iterFind = std::find(names.begin(), names.end(), frames[index]);
        if (iterFind != names.end())
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
        MMDump rawDump;
        size_t nread = file->Read(&rawDump);
        if (sizeof(MMDump) == nread || file->GetSize() == rawDump.size)
        {
            // Load and skip statistics
            MMCurStat curStat;
            nread = file->Read(&curStat, sizeof(curStat));
            if (sizeof(MMCurStat) == nread)
            {
                file->Seek(curStat.size - sizeof(MMCurStat), File::SEEK_FROM_CURRENT);

                const uint32 bktraceSize = sizeof(MMBacktrace) + rawDump.bktraceDepth * sizeof(uint64);
                Vector<MMBlock> blocks(rawDump.blockCount, MMBlock());
                Vector<MMSymbol> symbols(rawDump.symbolCount, MMSymbol());
                Vector<uint8> bktrace(bktraceSize * rawDump.bktraceCount, 0);

                // Read memory blocks
                nread = file->Read(&*blocks.begin(), rawDump.blockCount * sizeof(MMBlock));
                if (rawDump.blockCount * sizeof(MMBlock) == nread)
                {
                    // Read symbols
                    nread = file->Read(&*symbols.begin(), rawDump.symbolCount * sizeof(MMSymbol));
                    if (rawDump.symbolCount * sizeof(MMSymbol) == nread)
                    {
                        // Read backtraces
                        nread = file->Read(&*bktrace.begin(), bktraceSize * rawDump.bktraceCount);
                        if (bktraceSize * rawDump.bktraceCount == nread)
                        {
                            for (auto& sym : symbols)
                            {
                                if (sym.name[0] == '\0')
                                {
                                    Snprintf(sym.name, COUNT_OF(sym.name), "%08llX", sym.addr);
                                }
                                symbolTable->AddSymbol(sym.addr, sym.name);
                            }

                            const uint8* curOffset = bktrace.data();
                            for (size_t i = 0, n = rawDump.bktraceCount;i < n;++i)
                            {
                                const MMBacktrace* curBktrace = reinterpret_cast<const MMBacktrace*>(curOffset);
                                const uint64* frames = OffsetPointer<uint64>(curBktrace, sizeof(MMBacktrace));
                                symbolTable->AddBacktrace(curBktrace->hash, frames, rawDump.bktraceDepth);
                                curOffset += bktraceSize;
                            }

                            mblocks.swap(blocks);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void MemorySnapshot::BuildBlockMap()
{
    DAVA::Map<DAVA::uint32, DAVA::Vector<DAVA::MMBlock>> map;
    for (MMBlock& curBlock : mblocks)
    {
        auto iterAt = map.find(curBlock.bktraceHash);
        if (iterAt == map.end())
        {
            iterAt = map.emplace(curBlock.bktraceHash, DAVA::Vector<DAVA::MMBlock>()).first;
        }
        iterAt->second.emplace_back(curBlock);
    }
    blockMap.swap(map);
}
