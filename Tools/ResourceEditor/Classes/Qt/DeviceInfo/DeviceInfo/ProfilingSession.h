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

#ifndef __PROFILINGSESSION_H__
#define __PROFILINGSESSION_H__

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FilePath.h"
#include "Network/PeerDesription.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "BacktraceSymbolTable.h"

namespace DAVA
{
class File;
namespace Net
{
class PeerDescription;
}
}

struct Branch;
struct BranchDiff;
class MemoryDump;
class ProfilingSession;

class StatItem final
{
public:
    StatItem(DAVA::uint64 aTimestamp) : timestamp(aTimestamp) {}
    StatItem(const DAVA::MMCurStat* rawStat, size_t poolCount, size_t tagCount)
    {
        Init(rawStat, poolCount, tagCount);
    }
    // TODO: implement move semantic

    DAVA::uint64 Timestamp() const { return timestamp; }
    const DAVA::GeneralAllocStat& GeneralStat() const { return statGeneral; }
    const DAVA::Vector<DAVA::AllocPoolStat>& PoolStat() const { return statPools; }
    const DAVA::Vector<DAVA::TagAllocStat>& TagStat() const { return statTags; }

    const DAVA::AllocPoolStat& TotalStat() const { return statPools[0]; }

private:
    void Init(const DAVA::MMCurStat* rawStat, size_t poolCount, size_t tagCount);

private:
    DAVA::uint64 timestamp;
    DAVA::GeneralAllocStat statGeneral;
    DAVA::Vector<DAVA::AllocPoolStat> statPools;
    DAVA::Vector<DAVA::TagAllocStat> statTags;
};

class DumpBrief final
{
    friend class ProfilingSession;

public:
    DumpBrief(const DAVA::FilePath& filename, const DAVA::MMDump* rawDump)
        : dumpFileName(filename)
    {
        Init(rawDump);
    }
    ~DumpBrief()
    {
        ReleaseDump();
    }

    const DAVA::FilePath& FileName() const { return dumpFileName; }
    DAVA::uint64 Timestamp() const { return timestamp; }
    DAVA::uint32 CollectTime() const { return collectTime; }
    DAVA::uint32 PackTime() const { return packTime; }
    DAVA::uint32 BlockCount() const { return blockCount; }
    DAVA::uint32 SymbolCount() const { return symbolCount; }
    DAVA::uint32 BktraceCount() const { return bktraceCount; }
    DAVA::uint32 TotalSize() const { return totalSize; }

    MemoryDump* Dump() const { return memoryDump; }
    void ReleaseDump();

private:
    void Init(const DAVA::MMDump* rawDump);

private:
    DAVA::FilePath dumpFileName;
    DAVA::uint64 timestamp;
    DAVA::uint32 collectTime;
    DAVA::uint32 packTime;
    DAVA::uint32 blockCount;
    DAVA::uint32 symbolCount;
    DAVA::uint32 bktraceCount;
    DAVA::uint32 totalSize;

    MemoryDump* memoryDump = nullptr;
};

class ProfilingSession
{
public:
    ProfilingSession(const DAVA::MMStatConfig* config, const DAVA::Net::PeerDescription& devInfo);
    ProfilingSession(const DAVA::FilePath& filename);
    ~ProfilingSession();
    // TODO: implement move semantic

    bool IsFileLog() const { return isFileLog; }

    void AddStatItem(const DAVA::MMCurStat* rawStat);
    void AddDump(const DAVA::MMDump* rawDump);
    void Flush();

    size_t AllocPoolCount() const { return allocPoolCount; }
    size_t TagCount() const { return tagCount; }
    size_t StatCount() const { return stat.size(); }
    size_t DumpCount() const { return dump.size(); }
    const DAVA::Net::PeerDescription& DeviceInfo() const { return deviceInfo; }
    const BacktraceSymbolTable& SymbolTable() const { return symbolTable; }

    const DAVA::String& AllocPoolName(size_t index) const;
    const DAVA::String& TagName(size_t index) const;
    const StatItem& Stat(size_t index) const;
    const StatItem& LastStat() const;
    const DumpBrief& Dump(size_t index) const;
    const DumpBrief& LastDump() const;

    size_t ClosestStatItem(DAVA::uint64 timestamp) const;

    bool LoadDump(size_t index);

private:
    void Init(const DAVA::MMStatConfig* config);
    void InitFileSystem();
    void InitFileSystemWhenLoaded(const DAVA::FilePath& filename);

    void SaveDumpAsText(const DAVA::MMDump* rawDump, const char* filename);

private:
    void SaveLogHeader(const DAVA::MMStatConfig* config);
    void UpdateFileHeader(bool finalize);

    void LoadLogFile();
    void LoadStatItems(size_t count, DAVA::uint32 itemSize);
    void LookForDumps();
    void LoadDumpBrief(const DAVA::FilePath& path);
    bool LoadFullDump(const DumpBrief& brief, DAVA::Vector<DAVA::MMBlock>& mblocks);

private:
    bool isFileLog;
    size_t allocPoolCount = 0;
    size_t tagCount = 0;
    int dumpNo = 1;
    DAVA::Net::PeerDescription deviceInfo;
    DAVA::Vector<DAVA::String> allocPoolNames;
    DAVA::Vector<DAVA::String> tagNames;
    DAVA::Vector<StatItem> stat;
    DAVA::Vector<DumpBrief> dump;

    DAVA::FilePath storageDir;
    DAVA::FilePath statFileName;
    DAVA::RefPtr<DAVA::File> statFile;

    BacktraceSymbolTable symbolTable;
};

//////////////////////////////////////////////////////////////////////////
inline const DAVA::String& ProfilingSession::AllocPoolName(size_t index) const
{
    DVASSERT(0 <= index && index < allocPoolCount);
    return allocPoolNames[index];
}

inline const DAVA::String& ProfilingSession::TagName(size_t index) const
{
    DVASSERT(0 <= index && index < tagCount);
    return tagNames[index];
}

inline const StatItem& ProfilingSession::Stat(size_t index) const
{
    DVASSERT(0 <= index && index < stat.size());
    return stat[index];
}

inline const StatItem& ProfilingSession::LastStat() const
{
    DVASSERT(!stat.empty());
    return stat.back();
}

inline const DumpBrief& ProfilingSession::Dump(size_t index) const
{
    DVASSERT(0 <= index && index < dump.size());
    return dump[index];
}

inline const DumpBrief& ProfilingSession::LastDump() const
{
    DVASSERT(!dump.empty());
    return dump.back();
}

#endif  // __PROFILINGSESSION_H__
