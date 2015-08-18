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

#ifndef __MEMORYSNAPSHOT_H__
#define __MEMORYSNAPSHOT_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "MemoryManager/MemoryManagerTypes.h"

struct Branch;
class BacktraceSymbolTable;
class ProfilingSession;

class MemorySnapshot final
{
public:
    MemorySnapshot(const ProfilingSession* profilingSession, const DAVA::FilePath& filename, const DAVA::MMSnapshot* msnapshot);
    MemorySnapshot(MemorySnapshot&& other);
    MemorySnapshot& operator = (MemorySnapshot&& other);
    ~MemorySnapshot() = default;

    const DAVA::FilePath& FileName() const;
    DAVA::uint64 Timestamp() const;
    size_t BlockCount() const;
    size_t SymbolCount() const;
    size_t BktraceCount() const;
    size_t TotalSize() const;

    bool IsLoaded() const;
    bool Load(BacktraceSymbolTable* symbolTable);
    void Unload();

    const BacktraceSymbolTable* SymbolTable() const;
    const ProfilingSession* Session() const;
    const DAVA::Vector<DAVA::MMBlock>& MemoryBlocks() const { return mblocks; }
    
    // Create call tree branch starting from given names
    Branch* CreateBranch(const DAVA::Vector<const DAVA::String*>& startNames) const;

private:
    void Init(const DAVA::MMSnapshot* msnapshot);
    bool LoadFile();
    void BuildBlockMap();
    
    Branch* BuildPath(Branch* parent, int startFrame, const DAVA::Vector<const DAVA::String*>& bktraceNames) const;
    int FindNamesInBacktrace(const DAVA::Vector<const DAVA::String*>& namesToFind, const DAVA::Vector<const DAVA::String*>& bktraceNames) const;

private:
    const ProfilingSession* profilingSession = nullptr;
    DAVA::FilePath fileName;
    DAVA::uint64 timestamp = 0;
    size_t blockCount = 0;
    size_t symbolCount = 0;
    size_t bktraceCount = 0;
    size_t totalSize = 0;

    BacktraceSymbolTable* symbolTable = nullptr;
    DAVA::Vector<DAVA::MMBlock> mblocks;                                // All memory blocks contained in snapshot
    DAVA::Map<DAVA::uint32, DAVA::Vector<DAVA::MMBlock*>> blockMap;     // Map of memory blocks allocated at backtrace identified by its hash
};

//////////////////////////////////////////////////////////////////////////
inline MemorySnapshot::MemorySnapshot(const ProfilingSession* profilingSession_, const DAVA::FilePath& filename, const DAVA::MMSnapshot* msnapshot)
    : profilingSession(profilingSession_)
    , fileName(filename)
{
    Init(msnapshot);
}

inline const DAVA::FilePath& MemorySnapshot::FileName() const
{
    return fileName;
}

inline DAVA::uint64 MemorySnapshot::Timestamp() const
{
    return timestamp;
}

inline size_t MemorySnapshot::BlockCount() const
{
    return blockCount;
}

inline size_t MemorySnapshot::SymbolCount() const
{
    return symbolCount;
}

inline size_t MemorySnapshot::BktraceCount() const
{
    return bktraceCount;
}

inline size_t MemorySnapshot::TotalSize() const
{
    return totalSize;
}

inline bool MemorySnapshot::IsLoaded() const
{
    return symbolTable != nullptr;
}

inline const BacktraceSymbolTable* MemorySnapshot::SymbolTable() const
{
    return symbolTable;
}

inline const ProfilingSession* MemorySnapshot::Session() const
{
    return profilingSession;
}

inline void MemorySnapshot::Init(const DAVA::MMSnapshot* msnapshot)
{
    timestamp = msnapshot->timestamp;
    blockCount = msnapshot->blockCount;
    symbolCount = msnapshot->symbolCount;
    bktraceCount = msnapshot->bktraceCount;
    totalSize = msnapshot->size;
}

#endif  // __MEMORYSNAPSHOT_H__
