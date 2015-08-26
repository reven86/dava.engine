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

#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/MemoryTool/MemoryStatItem.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

namespace DAVA
{
class File;
}

struct Branch;
struct BranchDiff;

class ProfilingSession
{
public:
    ProfilingSession();
    ProfilingSession(const ProfilingSession&) = delete;
    ProfilingSession& operator = (const ProfilingSession&) = delete;
    ~ProfilingSession();

    bool StartNew(const DAVA::MMStatConfig* config, const DAVA::Net::PeerDescription& deviceInfo, const DAVA::FilePath& destDir);
    bool LoadFromFile(const DAVA::FilePath& srcFile);

    // Returns true if ProfilingSession has been restored from saved file, i.e. works in file mode
    bool IsFileMode() const;
    bool IsValid() const;

    // Add statistics items for memory consumption trends
    void AppendStatItems(const DAVA::MMCurStat* statBuf, size_t itemCount);
    // Add memory snapshot retrieved from profiled device
    void AppendSnapshot(const DAVA::FilePath& filename);
    // Flush file buffers to storage
    void Flush();

    DAVA::FilePath GenerateSnapshotFilename() const;

    // Get index of the closest stat item or -1 if not found
    size_t ClosestStatItem(DAVA::uint64 timestamp) const;
    
    // Load memory snapshot
    bool LoadSnapshot(size_t index);
    
    // Number of registered allocation pools
    size_t AllocPoolCount() const;
    // Number of registered memory block tags
    size_t TagCount() const;
    // Number of collected statistics items for trends
    size_t StatCount() const;
    // Number of collected memory snapshotss
    size_t SnapshotCount() const;
    // Profiled device description
    const DAVA::Net::PeerDescription& DeviceInfo() const;
    // Symbol and backtrace table for
    const BacktraceSymbolTable& SymbolTable() const;

    const DAVA::String& AllocPoolName(size_t index) const;
    const DAVA::String& AllocPoolNameByMask(DAVA::uint32 mask) const;
    const DAVA::String& TagName(size_t index) const;
    // Get stat item by index, items are sorted by timestamp
    const MemoryStatItem& Stat(size_t index) const;
    const MemoryStatItem& LastStat() const;
    // Get memory snapshot by index, snapshots are sorted by timestamp
    const MemorySnapshot& Snapshot(size_t index) const;
    const MemorySnapshot& LastSnapshot() const;

    const DAVA::FilePath& MemoryLogFile() const;

private:
    bool CreateLogFile(const DAVA::MMStatConfig* config);
    bool SaveLogHeader(const DAVA::MMStatConfig* config);
    void UpdateFileHeader(bool finalize);

    bool LoadLogFile();
    bool LoadLogHeader(size_t* itemCount, size_t* itemSize);
    bool LoadStatItems(size_t count, size_t itemSize);

    void ApplyConfig(const DAVA::MMStatConfig* config);
    void FlushAndReset(bool eraseFiles);

    void LookForShapshots();
    void LoadShapshotDescriptor(const DAVA::FilePath& path);

    void LoadSymbols(const DAVA::MMSymbol* symbols, size_t count);

private:
    bool isValid = false;
    bool isFileMode = false;
    size_t allocPoolCount = 0;
    size_t tagCount = 0;
    mutable int snapshotSeqNo = 1;
    DAVA::Net::PeerDescription deviceInfo;
    DAVA::Vector<DAVA::String> allocPoolNames;
    DAVA::Vector<std::pair<DAVA::uint32, size_t>> poolMaskMapping;
    DAVA::Vector<DAVA::String> tagNames;
    DAVA::Vector<MemoryStatItem> stat;
    DAVA::Vector<MemorySnapshot> snapshots;

    DAVA::FilePath storageDir;
    DAVA::FilePath logFileName;
    DAVA::RefPtr<DAVA::File> logFile;
    size_t statItemFlushed = 0;
    size_t statItemFlushThreshold = 5000;

    BacktraceSymbolTable symbolTable;
};

//////////////////////////////////////////////////////////////////////////
inline bool ProfilingSession::IsFileMode() const
{
    return isFileMode;
}

inline bool ProfilingSession::IsValid() const
{
    return isValid;
}

inline size_t ProfilingSession::AllocPoolCount() const
{
    return allocPoolCount;
}

inline size_t ProfilingSession::TagCount() const
{
    return tagCount;
}

inline size_t ProfilingSession::StatCount() const
{
    return stat.size();
}

inline size_t ProfilingSession::SnapshotCount() const
{
    return snapshots.size();
}

inline const DAVA::Net::PeerDescription& ProfilingSession::DeviceInfo() const
{
    return deviceInfo;
}

inline const BacktraceSymbolTable& ProfilingSession::SymbolTable() const
{
    return symbolTable;
}

inline const DAVA::String& ProfilingSession::AllocPoolName(size_t index) const
{
    DVASSERT(0 <= index && index < allocPoolCount);
    return allocPoolNames[index];
}

inline const DAVA::String& ProfilingSession::AllocPoolNameByMask(DAVA::uint32 mask) const
{
    DVASSERT(DAVA::IsPowerOf2(mask));
    auto iter = std::find_if(poolMaskMapping.cbegin(), poolMaskMapping.cend(), [mask](const std::pair<DAVA::uint32, size_t>& p) -> bool {
        return p.first == mask;
    });
    DVASSERT(iter != poolMaskMapping.cend());
    size_t index = iter->second;
    return AllocPoolName(index);
}

inline const DAVA::String& ProfilingSession::TagName(size_t index) const
{
    DVASSERT(0 <= index && index < tagCount);
    return tagNames[index];
}

inline const MemoryStatItem& ProfilingSession::Stat(size_t index) const
{
    DVASSERT(0 <= index && index < stat.size());
    return stat[index];
}

inline const MemoryStatItem& ProfilingSession::LastStat() const
{
    DVASSERT(!stat.empty());
    return stat.back();
}

inline const MemorySnapshot& ProfilingSession::Snapshot(size_t index) const
{
    DVASSERT(0 <= index && index < snapshots.size());
    return snapshots[index];
}

inline const MemorySnapshot& ProfilingSession::LastSnapshot() const
{
    DVASSERT(!snapshots.empty());
    return snapshots.back();
}

inline const DAVA::FilePath& ProfilingSession::MemoryLogFile() const
{
    return logFileName;
}

#endif  // __PROFILINGSESSION_H__
