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
#ifndef __MEMORYDUMP_H__
#define __MEMORYDUMP_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

struct Branch;
class DumpBrief;
class BacktraceSymbolTable;

class MemoryDump final
{
public:
    MemoryDump(const DumpBrief& brief, const BacktraceSymbolTable& symTable, DAVA::Vector<DAVA::MMBlock>&& mblocks);
    ~MemoryDump();

    const DumpBrief& Brief() const { return dumpBrief; }
    const BacktraceSymbolTable& SymbolTable() const { return symbolTable; }

    // Get list of all memory blocks in dump
    DAVA::Vector<const DAVA::MMBlock*> GetMemoryBlocks() const;

    // Create call tree branch starting from given names
    Branch* CreateBranch(const DAVA::Vector<const char*>& startNames) const;

private:
    Branch* BuildPath(Branch* parent, int startFrame, const DAVA::Vector<const char*>& frames) const;
    int FindNamesInBacktrace(const DAVA::Vector<const char*>& names, const DAVA::Vector<const char*>& frames) const;
    void BuildBlockMap();

private:
    const DumpBrief& dumpBrief;
    const BacktraceSymbolTable& symbolTable;
    DAVA::Vector<DAVA::MMBlock> memoryBlocks;
    DAVA::Map<DAVA::uint32, DAVA::Vector<const DAVA::MMBlock*>> blockMap;   // key - backtrace hash,
                                                                            // value - vector of memory blocks allocated at backtrace
};

#endif  // __MEMORYDUMP_H__
