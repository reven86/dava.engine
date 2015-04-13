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
#include "FileSystem/File.h"
#include "Platform/DateTime.h"
#include "Debug/DVAssert.h"

#include "BacktraceSymbolTable.h"

#include "ProfilingSession.h"

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

void StatItem::Init(const DAVA::MMCurStat* rawStat, size_t poolCount, size_t tagCount)
{
    DVASSERT(rawStat != nullptr && poolCount > 0);

    statPools.reserve(poolCount);
    statTags.reserve(tagCount);
    timestamp = rawStat->timestamp;
    statGeneral = rawStat->statGeneral;

    const AllocPoolStat* pools = OffsetPointer<AllocPoolStat>(rawStat, sizeof(MMCurStat));
    for (size_t i = 0;i < poolCount;++i)
    {
        statPools.push_back(*pools);
        pools += 1;
    }
    const TagAllocStat* tags = reinterpret_cast<const TagAllocStat*>(pools);
    for (size_t i = 0;i < tagCount;++i)
    {
        statTags.push_back(*tags);
        tags += 1;
    }
}

void DumpBrief::Init(const DAVA::MMDump* rawDump)
{
    DVASSERT(rawDump != nullptr);

    timestamp = rawDump->timestamp;
    collectTime = rawDump->collectTime * 10;
    packTime = rawDump->packTime * 10;
    blockCount = rawDump->blockCount;
    symbolCount = rawDump->symbolCount;
    bktraceCount = rawDump->bktraceCount;
    totalSize = rawDump->size;
}

ProfilingSession::ProfilingSession(const DAVA::MMStatConfig* config, const DAVA::Net::PeerDescription& devInfo)
    : isFileLog(false)
    , deviceInfo(devInfo)
{
    DVASSERT(config != nullptr);
    InitFileSystem();
    Init(config);
    SaveLogHeader(config);
}

ProfilingSession::ProfilingSession(const FilePath& filename)
    : isFileLog(true)
{
    InitFileSystemWhenLoaded(filename);
    LoadLogFile();
}

ProfilingSession::~ProfilingSession()
{
    if (!isFileLog)
    {
        UpdateFileHeader(true);
    }
}

void ProfilingSession::Init(const DAVA::MMStatConfig* config)
{
    allocPoolCount = config->allocPoolCount;
    tagCount = config->tagCount;

    allocPoolNames.reserve(allocPoolCount);
    tagNames.reserve(tagCount);

    const MMItemName* names = OffsetPointer<MMItemName>(config, sizeof(MMItemName));
    for (size_t i = 0;i < allocPoolCount;++i)
    {
        allocPoolNames.push_back(names->name);
        names += 1;
    }
    for (size_t i = 0;i < tagCount;++i)
    {
        tagNames.push_back(names->name);
        names += 1;
    }
}

void ProfilingSession::InitFileSystem()
{
    String level1 = Format("%s %s/",
                           deviceInfo.GetManufacturer().c_str(),
                           deviceInfo.GetModel().c_str());
    String level2 = Format("%s {%s}/",
                          deviceInfo.GetName().c_str(),
                          deviceInfo.GetUDID().c_str());

    DateTime now = DateTime::Now();
    String level3 = Format("%04d-%02d-%02d %02d%02d%02d/",
                          now.GetYear(), now.GetMonth(), now.GetDay(),
                          now.GetHour(), now.GetMinute(), now.GetSecond());

    storageDir = "~doc:/";
    storageDir += "memdumps/";
    storageDir += deviceInfo.GetPlatformString();
    storageDir += "/";
    storageDir += level1;
    storageDir += level2;
    storageDir += level3;

    auto result = FileSystem::Instance()->CreateDirectory(storageDir, true);
    DVASSERT(result != FileSystem::DIRECTORY_CANT_CREATE);
    if (result != FileSystem::DIRECTORY_CANT_CREATE)
    {
        statFileName = storageDir;
        statFileName += "statlog.bin";

        {
            // DAVA::File doesn't allow creating new file for read/write
            // So first create file and then open for reading and writing
            // TODO: maybe this feature should be added
            RefPtr<File> temp(File::Create(statFileName, File::CREATE | File::WRITE));
        }
        statFile.Set(File::Create(statFileName, File::OPEN | File::READ | File::WRITE));
        DVASSERT(statFile.Get() != nullptr);
    }
}

void ProfilingSession::InitFileSystemWhenLoaded(const FilePath& filename)
{
    storageDir = filename.GetDirectory();
    statFileName = filename;
}

void ProfilingSession::AddStatItem(const DAVA::MMCurStat* rawStat)
{
    stat.emplace_back(rawStat, allocPoolCount, tagCount);
    if (statFile.Valid())
    {
        statFile->Write(rawStat, rawStat->size);
    }
}

void ProfilingSession::AddDump(const DAVA::MMDump* rawDump)
{
    FilePath dumpFileName = storageDir;
    dumpFileName += Format("dump_%d.bin", dumpNo);

    RefPtr<File> dumpFile(File::Create(dumpFileName, File::CREATE | File::WRITE));
    if (dumpFile.Valid())
    {
        dump.emplace_back(dumpFileName, rawDump);

        dumpFile->Write(rawDump, rawDump->size);

        SaveDumpAsText(rawDump, (storageDir + Format("dump_%d.log", dumpNo)).GetAbsolutePathname().c_str());
    }
    dumpNo += 1;
}

void ProfilingSession::Flush()
{
    if (statFile.Valid())
    {
        statFile->Flush();
    }
}

size_t ProfilingSession::ClosestStatItem(DAVA::uint64 timestamp) const
{
    StatItem dummy(timestamp);
    auto iter = std::lower_bound(stat.begin(), stat.end(), dummy, [](const StatItem& l, const StatItem& r) -> bool {
        return l.Timestamp() < r.Timestamp();
    });
    if (iter != stat.end())
    {
        return std::distance(stat.begin(), iter);
    }
    return size_t(-1);
}

void ProfilingSession::SaveLogHeader(const DAVA::MMStatConfig* config)
{
    FileHeader header;
    Memset(&header, 0, sizeof(FileHeader));
    header.signature = FILE_SIGNATURE;
    header.statCount = 0;
    header.finished = 0;
    header.devInfoSize = static_cast<uint32>(deviceInfo.SerializedSize());
    header.statConfigSize = config->size;
    header.statItemSize = sizeof(MMCurStat)
                        + sizeof(AllocPoolStat) * config->allocPoolCount
                        + sizeof(TagAllocStat) * config->tagCount;

    Vector<uint8> rawDeviceInfo(header.devInfoSize, 0);
    deviceInfo.Serialize(&*rawDeviceInfo.begin(), header.devInfoSize);

    // Save file header
    uint32 nwritten = statFile->Write(&header);
    DVASSERT(sizeof(FileHeader) == nwritten);

    // Save device information
    nwritten = statFile->Write(rawDeviceInfo.data(), header.devInfoSize);
    DVASSERT(header.devInfoSize == nwritten);

    // Save stat config
    nwritten = statFile->Write(config, config->size);
    DVASSERT(config->size == nwritten);

    statFile->Flush();
}

void ProfilingSession::UpdateFileHeader(bool finalize)
{
    uint32 curPos = statFile->GetPos();
    statFile->Seek(0, File::SEEK_FROM_START);
    DVASSERT(0 == statFile->GetPos());

    FileHeader header;
    uint32 nread = statFile->Read(&header);
    DVASSERT(sizeof(FileHeader) == nread);
    header.statCount = static_cast<uint32>(stat.size());
    header.finished = finalize ? 1 : 0;

    statFile->Seek(0, File::SEEK_FROM_START);
    DVASSERT(0 == statFile->GetPos());

    uint32 nwritten = statFile->Write(&header);
    DVASSERT(sizeof(FileHeader) == nwritten);

    statFile->Seek(0, File::SEEK_FROM_END);
    DVASSERT(statFile->GetPos() == curPos);

    statFile->Flush();
}

void ProfilingSession::LoadLogFile()
{
    statFile.Set(File::Create(statFileName, File::OPEN | File::READ));
    DVASSERT(statFile.Valid());
    if (statFile.Valid())
    {
        // Load file header
        FileHeader header;
        uint32 nread = statFile->Read(&header);
        DVASSERT(sizeof(FileHeader) == nread);
        DVASSERT(FILE_SIGNATURE == header.signature);
        DVASSERT(header.finished != 0 && header.statCount > 0 && header.devInfoSize > 0);
        DVASSERT(header.statConfigSize > 0 && header.statItemSize > 0);

        // Load device info
        Vector<uint8> devInfoBuf(header.devInfoSize, 0);
        nread = statFile->Read(&*devInfoBuf.begin(), header.devInfoSize);
        DVASSERT(nread == header.devInfoSize);
        deviceInfo.Deserialize(devInfoBuf.data(), devInfoBuf.size());

        // Load stat config
        Vector<uint8> configBuf(header.statConfigSize, 0);
        nread = statFile->Read(&*configBuf.begin(), header.statConfigSize);
        DVASSERT(nread == header.statConfigSize);

        const MMStatConfig* statConfig = reinterpret_cast<const MMStatConfig*>(configBuf.data());
        DVASSERT(statConfig->size == header.statConfigSize);
        Init(statConfig);

        LoadStatItems(header.statCount, header.statItemSize);
    }
}

void ProfilingSession::LoadStatItems(size_t count, uint32 itemSize)
{
    const uint32 BUF_CAPACITY = 100;
    Vector<uint8> buf(BUF_CAPACITY * itemSize, 0);

    uint32 nitems = 1;
    size_t nloaded = 0;
    stat.reserve(count);
    while (nloaded < count && nitems > 0)
    {
        uint32 nread = statFile->Read(&*buf.begin(), itemSize * BUF_CAPACITY);
        DVASSERT(nread % itemSize == 0);
        nitems = nread / itemSize;

        const MMCurStat* rawStat = reinterpret_cast<const MMCurStat*>(buf.data());
        for (uint32 i = 0;i < nitems;++i)
        {
            stat.emplace_back(rawStat, allocPoolCount, tagCount);
            rawStat = OffsetPointer<MMCurStat>(rawStat, itemSize);
        }
        nloaded += nitems;
    }
}

void ProfilingSession::SaveDumpAsText(const DAVA::MMDump* rawDump, const char* filename)
{
    FILE* f = fopen(filename, "wb");
    if (f != nullptr)
    {
        BacktraceSymbolTable table;

        const size_t bktraceSize = sizeof(MMBacktrace) + rawDump->bktraceDepth * sizeof(uint64);
        const MMCurStat* stat = OffsetPointer<MMCurStat>(rawDump, sizeof(MMDump));
        const MMBlock* blocks = OffsetPointer<MMBlock>(stat, stat->size);
        const MMSymbol* symbols = OffsetPointer<MMSymbol>(blocks, sizeof(MMBlock) * rawDump->blockCount);
        const MMBacktrace* bt = OffsetPointer<MMBacktrace>(symbols, sizeof(MMSymbol) * rawDump->symbolCount);

        for (size_t i = 0, n = rawDump->symbolCount;i < n;++i)
        {
            table.AddSymbol(symbols[i].addr, symbols[i].name);
        }
        const MMBacktrace* p = bt;
        for (size_t i = 0, n = rawDump->bktraceCount;i < n;++i)
        {
            const uint64* frames = OffsetPointer<uint64>(p, sizeof(MMBacktrace));
            table.AddBacktrace(p->hash, frames, rawDump->bktraceDepth);
            p = OffsetPointer<MMBacktrace>(p, bktraceSize);
        }

        fprintf(f, "General info\n");
        fprintf(f, "  collectTime   : %u ms\n", uint32(rawDump->collectTime * 10));
        fprintf(f, "  packTime      : %u ms\n", uint32(rawDump->packTime * 10));
        fprintf(f, "  blockCount    : %u\n"   , rawDump->blockCount);
        fprintf(f, "  backtraceCount: %u\n"   , rawDump->bktraceCount);
        fprintf(f, "  nameCount     : %u\n"   , rawDump->symbolCount);
        fprintf(f, "  backtraceDepth: %u\n"   , rawDump->bktraceDepth);

        fprintf(f, "Blocks\n");
        for (uint32 i = 0;i < rawDump->blockCount;++i)
        {
            fprintf(f, "%4d: allocByApp=%u, allocTotal=%u, orderNo=%u, pool=%u, hash=%u, tags=%08X\n", i + 1,
                    blocks[i].allocByApp, blocks[i].allocTotal, blocks[i].orderNo, blocks[i].pool,
                    blocks[i].bktraceHash, blocks[i].tags);
            const Vector<const char8*>& fr = table.GetFrames(blocks[i].bktraceHash);
            for (auto s : fr)
            {
                fprintf(f, "        %s\n", s);
            }
        }

        fclose(f);
    }
}
