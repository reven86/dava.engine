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

#include "MemoryDump.h"
#include "Branch.h"
#include "BacktraceSymbolTable.h"

using namespace DAVA;

MemoryDump::MemoryDump(const DumpBrief& brief, const BacktraceSymbolTable& symTable, DAVA::Vector<DAVA::MMBlock>&& mblocks)
    : dumpBrief(brief)
    , symbolTable(symTable)
    , memoryBlocks(std::move(mblocks))
{
    BuildBlockMap();
}

MemoryDump::~MemoryDump()
{}

Vector<const MMBlock*> MemoryDump::GetMemoryBlocks() const
{
    Vector<const MMBlock*> result;
    result.reserve(memoryBlocks.size());
    for (auto& block : memoryBlocks)
    {
        result.push_back(&block);
    }
    return result;
}

Branch* MemoryDump::CreateBranch(const DAVA::Vector<const char*>& startNames) const
{
    Branch* root = new Branch(nullptr);
    for (auto& pair : blockMap)
    {
        auto& frames = symbolTable.GetFrames(pair.first);
        auto& blocks = pair.second;

        if (!blocks.empty() && !frames.empty())
        {
            int startFrame = FindNamesInBacktrace(startNames, frames);
            if (startFrame >= 0)
            {
                Branch* leaf = BuildPath(root, startFrame, frames);

                // Append memory blocks to leaf
                uint32 allocByApp = 0;
                size_t blockCount = blocks.size();
                leaf->mblocks.reserve(leaf->mblocks.size() + blocks.size());
                for (auto x : blocks)
                {
                    allocByApp += x->allocByApp;
                    leaf->mblocks.push_back(x);
                }
                leaf->UpdateStat(allocByApp, static_cast<uint32>(blocks.size()));
            }
        }
    }
    root->SortChildren([](const Branch* l, const Branch* r) -> bool {
        return strcmp(l->name, r->name) < 0;
    });
    return root;
}

Branch* MemoryDump::BuildPath(Branch* parent, int startFrame, const Vector<const char*>& frames) const
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

int MemoryDump::FindNamesInBacktrace(const Vector<const char*>& names, const Vector<const char*>& frames) const
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

void MemoryDump::BuildBlockMap()
{
    for (MMBlock& curBlock : memoryBlocks)
    {
        auto iterAt = blockMap.find(curBlock.bktraceHash);
        if (iterAt == blockMap.end())
        {
            iterAt = blockMap.emplace(curBlock.bktraceHash, DAVA::Vector<const DAVA::MMBlock*>()).first;
        }
        iterAt->second.push_back(&curBlock);
    }
}
