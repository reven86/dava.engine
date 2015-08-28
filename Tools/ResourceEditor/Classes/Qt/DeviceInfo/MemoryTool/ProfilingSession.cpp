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

#include "Utils/StringFormat.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "FileSystem/File.h"
#include "Platform/DateTime.h"
#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"

using namespace DAVA;

namespace
{

const uint32 FILE_SIGNATURE = 0x41764144;

struct FileHeader
{
    uint32 signature;
    uint32 statCount;
    uint32 finished;
    uint32 devInfoSize;
    uint32 statConfigSize;
    uint32 statItemSize;
    uint32 padding[2];
};
static_assert(sizeof(FileHeader) == 32, "sizeof(FileHeader) != 32");

}   // unnamed namespace

ProfilingSession::ProfilingSession()
{}

ProfilingSession::~ProfilingSession()
{
    FlushAndReset(false);
}

bool ProfilingSession::StartNew(const DAVA::MMStatConfig* config, const DAVA::Net::PeerDescription& deviceInfo_, const DAVA::FilePath& destDir)
{
    DVASSERT(config != nullptr && !destDir.IsEmpty());

    FlushAndReset(false);

    isFileMode = false;
    deviceInfo = deviceInfo_;
    storageDir = destDir;
    logFileName = storageDir;
    logFileName += "/";
    logFileName += "memory.mlog";

    ApplyConfig(config);

    isValid = CreateLogFile(config);
    if (!isValid)
    {
        FlushAndReset(true);    // Erase files if created
    }
    return isValid;
}

bool ProfilingSession::LoadFromFile(const DAVA::FilePath& srcFile)
{
    DVASSERT(!srcFile.IsEmpty());

    FlushAndReset(false);

    isFileMode = true;
    storageDir = srcFile.GetDirectory();
    logFileName = srcFile;

    isValid = LoadLogFile();
    if (isValid)
    {
        LookForShapshots();
    }
    else
    {
        FlushAndReset(false);
    }
    return isValid;
}

void ProfilingSession::AppendStatItems(const DAVA::MMCurStat* statBuf, size_t itemCount)
{
    DVASSERT(statBuf != nullptr && !isFileMode);

    const size_t itemSize = statBuf->size;
    stat.reserve(stat.size() + itemCount);
    const DAVA::MMCurStat* statPtr = statBuf;
    for (size_t i = 0;i < itemCount;++i)
    {
        stat.emplace_back(statBuf, allocPoolCount, tagCount);
        statPtr = OffsetPointer<const DAVA::MMCurStat>(statPtr, itemSize);
    }

    if (logFile.Valid())
    {
        logFile->Write(statBuf, itemSize * itemCount);
        if (stat.size() - statItemFlushed >= statItemFlushThreshold)
        {
            UpdateFileHeader(false);
            statItemFlushed = stat.size();
        }
    }
}

void ProfilingSession::AppendSnapshot(const DAVA::FilePath& filename)
{
    LoadShapshotDescriptor(filename);
}

void ProfilingSession::Flush()
{
    if (logFile.Valid())
    {
        logFile->Flush();
    }
}

FilePath ProfilingSession::GenerateSnapshotFilename() const
{
    return storageDir + Format("%02d.snapshot", snapshotSeqNo++);
}

bool ProfilingSession::LoadSnapshot(size_t index)
{
    DVASSERT(0 <= index && index < snapshots.size());

    MemorySnapshot& snapshot = snapshots[index];
    if (!snapshot.IsLoaded())
    {
        return snapshot.Load(&symbolTable);
    }
    return true;
}

size_t ProfilingSession::ClosestStatItem(DAVA::uint64 timestamp) const
{
    MemoryStatItem dummy(timestamp);
    auto less = [](const MemoryStatItem& l, const MemoryStatItem& r) -> bool { return l.Timestamp() < r.Timestamp(); };
    
    auto iter = std::lower_bound(stat.begin(), stat.end(), dummy, less);
    if (iter != stat.end())
    {
        return std::distance(stat.begin(), iter);
    }
    return size_t(-1);
}

bool ProfilingSession::CreateLogFile(const DAVA::MMStatConfig* config)
{
    int dirCreateResult = FileSystem::DIRECTORY_CREATED;
    if (!storageDir.IsEmpty())
    {
        dirCreateResult = FileSystem::Instance()->CreateDirectory(storageDir, true);
    }
    if (dirCreateResult != FileSystem::DIRECTORY_CANT_CREATE)
    {
        {
            // DAVA::File doesn't allow creating new file for read/write
            // So first create file and then open for reading and writing
            // TODO: maybe this feature should be added
            RefPtr<File> temp(File::Create(logFileName, File::CREATE | File::WRITE));
        }
        logFile.Set(File::Create(logFileName, File::OPEN | File::READ | File::WRITE));
        DVASSERT(logFile.Valid());
        if (logFile.Valid())
        {
            return SaveLogHeader(config);
        }
    }
    return false;
}

bool ProfilingSession::SaveLogHeader(const DAVA::MMStatConfig* config)
{
    FileHeader header{};
    header.signature = FILE_SIGNATURE;
    header.statCount = 0;
    header.finished = 0;
    header.devInfoSize = static_cast<uint32>(deviceInfo.SerializedSize());
    header.statConfigSize = config->size;
    header.statItemSize = sizeof(MMCurStat)
                        + sizeof(AllocPoolStat) * config->allocPoolCount
                        + sizeof(TagAllocStat) * config->tagCount;

    Vector<uint8> serializedDeviceInfo(header.devInfoSize, 0);
    deviceInfo.Serialize(&*serializedDeviceInfo.begin(), header.devInfoSize);

    // Save file header
    uint32 nwritten = logFile->Write(&header);
    // Save device information
    nwritten += logFile->Write(serializedDeviceInfo.data(), header.devInfoSize);
    // Save stat config
    nwritten += logFile->Write(config, config->size);
    logFile->Flush();

    uint32 nexpected = sizeof(FileHeader) + header.devInfoSize + config->size;
    return nexpected == nwritten;
}

void ProfilingSession::UpdateFileHeader(bool finalize)
{
    logFile->Seek(0, File::SEEK_FROM_START);

    FileHeader header{};
    logFile->Read(&header);
    header.statCount = static_cast<uint32>(stat.size());
    header.finished = finalize ? 1 : 0;

    logFile->Seek(0, File::SEEK_FROM_START);

    logFile->Write(&header);
    logFile->Seek(0, File::SEEK_FROM_END);

    logFile->Flush();
}

bool ProfilingSession::LoadLogFile()
{
    logFile.Set(File::Create(logFileName, File::OPEN | File::READ));
    if (logFile.Valid())
    {
        size_t itemCount = 0;
        size_t itemSize = 0;
        if (LoadLogHeader(&itemCount, &itemSize))
        {
            return LoadStatItems(itemCount, itemSize);
        }
    }
    return false;
}

bool ProfilingSession::LoadLogHeader(size_t* itemCount, size_t* itemSize)
{
    FileHeader header;
    size_t nread = logFile->Read(&header);
    if (sizeof(FileHeader) == nread)
    {
        if (FILE_SIGNATURE == header.signature && header.devInfoSize > 0 &&
                              header.statConfigSize > sizeof(MMStatConfig) && header.statItemSize > sizeof(MMCurStat))
        {
            Vector<uint8> tempBuf;

            // Load device info
            tempBuf.resize(header.devInfoSize);
            nread = logFile->Read(&*tempBuf.begin(), header.devInfoSize);
            if (nread == header.devInfoSize)
            {
                deviceInfo.Deserialize(tempBuf.data(), header.devInfoSize);

                // Load stat config
                tempBuf.resize(header.statConfigSize);
                nread = logFile->Read(&*tempBuf.begin(), header.statConfigSize);
                if (nread == header.statConfigSize)
                {
                    *itemCount = header.statCount;
                    *itemSize = header.statItemSize;

                    const MMStatConfig* statConfig = reinterpret_cast<const MMStatConfig*>(tempBuf.data());
                    ApplyConfig(statConfig);
                    return true;
                }
            }
        }
    }
    return false;
}

bool ProfilingSession::LoadStatItems(size_t count, size_t itemSize)
{
    const size_t BUF_CAPACITY = 1000;   // Some reasonable buffer size
    Vector<uint8> buf(BUF_CAPACITY * itemSize, 0);

    bool result = true;
    size_t nloaded = 0;
    stat.reserve(count);
    while (nloaded < count && result)
    {
        size_t toread = Min(BUF_CAPACITY * itemSize, (count - nloaded) * itemSize);
        size_t nread = logFile->Read(&*buf.begin(), toread);
        result = toread == nread && 0 == nread % itemSize;
        if (result)
        {
            const MMCurStat* rawStat = reinterpret_cast<const MMCurStat*>(buf.data());
            size_t nitems = nread / itemSize;
            for (size_t i = 0;i < nitems;++i)
            {
                stat.emplace_back(rawStat, allocPoolCount, tagCount);
                rawStat = OffsetPointer<const MMCurStat>(rawStat, itemSize);
            }
            nloaded += nitems;
        }
    }
    return result;
}

void ProfilingSession::ApplyConfig(const DAVA::MMStatConfig* config)
{
    allocPoolCount = config->allocPoolCount;
    tagCount = config->tagCount;

    allocPoolNames.reserve(allocPoolCount);
    poolMaskMapping.reserve(allocPoolCount);
    tagNames.reserve(tagCount);

    const MMItemName* names = OffsetPointer<const MMItemName>(config, sizeof(MMItemName));
    for (size_t i = 0;i < allocPoolCount;++i)
    {
        poolMaskMapping.emplace_back(1 << i, i);
        allocPoolNames.push_back(names->name);
        names += 1;
    }
    for (size_t i = 0;i < tagCount;++i)
    {
        tagNames.push_back(names->name);
        names += 1;
    }
}

void ProfilingSession::FlushAndReset(bool eraseFiles)
{
    if (!isFileMode && logFile.Valid())
    {
        UpdateFileHeader(true);
    }
    logFile.Set(nullptr);

    isValid = false;
    isFileMode = false;
    allocPoolCount = 0;
    tagCount = 0;
    snapshotSeqNo = 1;

    allocPoolNames.clear();
    tagNames.clear();
    stat.clear();
    snapshots.clear();

    if (eraseFiles)
    {
        FileSystem::Instance()->DeleteFile(logFileName);
    }
}

void ProfilingSession::LookForShapshots()
{
    RefPtr<FileList> fileList(new FileList(storageDir));
    for (int32 i = 0, n = fileList->GetCount();i < n;++i)
    {
        if (!fileList->IsDirectory(i))
        {
            const FilePath& path = fileList->GetPathname(i);
            String ext = path.GetExtension();
            if (ext == ".snapshot")
            {
                LoadShapshotDescriptor(path);
            }
        }
    }
    // Sort snapshots by timestamp
    std::sort(snapshots.begin(), snapshots.end(), [](const MemorySnapshot& l, const MemorySnapshot& r) -> bool { return l.Timestamp() < r.Timestamp(); });
}

void ProfilingSession::LoadShapshotDescriptor(const DAVA::FilePath& path)
{
    RefPtr<File> file(File::Create(path, File::OPEN | File::READ));
    if (file.Valid())
    {
        MMSnapshot msnapshot;
        size_t nread = file->Read(&msnapshot);
        if (sizeof(MMSnapshot) == nread && file->GetSize() == msnapshot.size)
        {
            if (msnapshot.blockCount > 0 && msnapshot.bktraceDepth > 0)
            {
                snapshots.emplace_back(this, path, &msnapshot);

                if (msnapshot.symbolCount > 0)
                {
                    uint32 symOffset = msnapshot.dataOffset + msnapshot.blockCount * sizeof(MMBlock);
                    file->Seek(symOffset, File::SEEK_FROM_START);

                    Vector<MMSymbol> symbols(msnapshot.symbolCount, MMSymbol());
                    nread = file->Read(&*symbols.begin(), msnapshot.symbolCount * sizeof(MMSymbol));
                    if (msnapshot.symbolCount * sizeof(MMSymbol) == nread)
                    {
                        LoadSymbols(&*symbols.begin(), msnapshot.symbolCount);
                    }
                }
            }
        }
    }
}

void ProfilingSession::LoadSymbols(const MMSymbol* symbols, size_t count)
{
    for (size_t i = 0;i < count;++i)
    {
        DVASSERT(strlen(symbols[i].name) > 0);
        symbolTable.AddSymbol(symbols[i].addr, symbols[i].name);
    }
}
