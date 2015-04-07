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
#include "Network/PeerDesription.h"

#include "BacktraceSymbolTable.h"

#include "ProfilingSession.h"

using namespace DAVA;

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

void DumpItem::Init(const DAVA::MMDump* rawDump)
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
    : deviceInfo(devInfo)
{
    DVASSERT(config != nullptr);
    InitFileSystem();
    Init(config);
}

ProfilingSession::~ProfilingSession()
{}

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

        statFile.Set(File::Create(statFileName, File::CREATE | File::READ | File::WRITE));
        DVASSERT(statFile.Get() != nullptr);
    }
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
