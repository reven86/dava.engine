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

#include "Base/Function.h"

#include "Debug/DVAssert.h"

#include "FileSystem/DynamicMemoryFile.h"
#include "DLC/Patcher/ZLibStream.h"
#include "MemoryManager/MemoryManager.h"

#include "MMNetClient.h"

namespace DAVA
{
namespace Net
{

MMNetClient::MMNetClient()
    : NetService()
    , sessionId(0)
    , commInited(false)
    , outbufBusy(false)
    , gettingDump(false)
    , dumpSize(0)
    , dumpRecv(0)
{

}

MMNetClient::~MMNetClient()
{

}

void MMNetClient::SetCallbacks(ChOpenCallback onOpen, ChClosedCallback onClosed, StatCallback onStat, DumpGetCallback onDumpGet, DumpDoneCallback onDumpDone)
{
    openCallback = onOpen;
    closeCallback = onClosed;
    statCallback = onStat;
    dumpGetCallback = onDumpGet;
    dumpDoneCallback = onDumpDone;
}

void MMNetClient::RequestDump()
{
    if (commInited && !outbufBusy)
    {
        SendDumpRequest();
    }
}

void MMNetClient::ChannelOpen()
{
    SendInitSession();
}

void MMNetClient::ChannelClosed(const char8* message)
{
    commInited = false;
    outbufBusy = false;

    closeCallback(message);
}

void MMNetClient::PacketReceived(const void* packet, size_t length)
{
    DVASSERT(length >= sizeof(MMProtoHeader));

    if (gettingDump)
    {
        ProcessDumpNext(packet, length);
        return;
    }

    const MMProtoHeader* hdr = static_cast<const MMProtoHeader*>(packet);
    const eMMProtoCmd cmd = static_cast<eMMProtoCmd>(hdr->cmd);
    switch (cmd)
    {
    case eMMProtoCmd::INIT_COMM:
        ProcessInitCommunication(hdr, static_cast<const uint8*>(packet) + sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    case eMMProtoCmd::CUR_STAT:
        ProcessCurrentStatistics(hdr, static_cast<const uint8*>(packet)+sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    case eMMProtoCmd::DUMP:
        ProcessDump(hdr, static_cast<const uint8*>(packet)+sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    default:
        break;
    }
}

void MMNetClient::PacketDelivered()
{
    outbufBusy = false;
}

void MMNetClient::ProcessInitCommunication(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    const MMStatConfig* config = nullptr;
    if (hdr->length > 0)
    {
        sessionId = hdr->sessionId;
        config = reinterpret_cast<const MMStatConfig*>(hdr + 1);
    }
    commInited = true;

    openCallback(config);
}

void MMNetClient::ProcessCurrentStatistics(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    const MMStat* stat = static_cast<const MMStat*>(packet);
    statCallback(stat);
}

void MMNetClient::ProcessDump(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    //const MMDump* dump = static_cast<const MMDump*>(packet);

    gettingDump = true;
    //dumpSize = sizeof(MMDump) 
    //    + sizeof(MMBlock) * dump->blockCount 
    //    + sizeof(MMBacktrace) * dump->backtraceCount
    //    + sizeof(MMSymbol) * dump->symbolCount;
    dumpSize = hdr->length;
    unpackedDumpSize = hdr->status;
    dumpRecv = length;

    dumpV.resize(dumpSize);
    Memcpy(dumpV.data(), packet, dumpRecv);

    dumpGetCallback(dumpSize, dumpRecv);

    if (dumpRecv >= dumpSize)
    {
        //MMDump* dump = reinterpret_cast<MMDump*>(dumpV.data());
        //dumpDoneCallback(dump);
        //gettingDump = false;
        UnpackDump();
    }
}

void MMNetClient::ProcessDumpNext(const void* packet, size_t length)
{
    DVASSERT(dumpRecv + length <= dumpSize);

    Memcpy(dumpV.data() + dumpRecv, packet, length);
    dumpRecv += length;

    if (dumpRecv < dumpSize)
    {
        dumpGetCallback(dumpSize, dumpRecv);
    }
    else
    {
        //MMDump* dump = reinterpret_cast<MMDump*>(dumpV.data());
        //dumpDoneCallback(dump);
        //gettingDump = false;
        UnpackDump();
    }
}

void MMNetClient::UnpackDump()
{
    Vector<uint8> v(unpackedDumpSize, 0);
    DynamicMemoryFile* zipFile = DynamicMemoryFile::Create(dumpV.data() + sizeof(MMDump), uint32(dumpV.size() - sizeof(MMDump)), File::CREATE | File::READ);
    ZLibIStream zipStream(zipFile);
    zipStream.Read((char8*)(v.data() + sizeof(MMDump)), uint32(unpackedDumpSize - sizeof(MMDump)));
    Memcpy(v.data(), dumpV.data(), sizeof(MMDump));

    MMDump* dump = reinterpret_cast<MMDump*>(v.data());
    dumpDoneCallback(dump, dumpSize);
    gettingDump = false;
}

void MMNetClient::SendInitSession()
{
    MMProtoHeader* hdr = reinterpret_cast<MMProtoHeader*>(outbuf);
    hdr->sessionId = sessionId;
    hdr->cmd = static_cast<uint32>(eMMProtoCmd::INIT_COMM);
    hdr->status = 0;
    hdr->length = 0;

    outbufBusy = true;
    Send(hdr);
}

void MMNetClient::SendDumpRequest()
{
    MMProtoHeader* hdr = reinterpret_cast<MMProtoHeader*>(outbuf);
    hdr->sessionId = sessionId;
    hdr->cmd = static_cast<uint32>(eMMProtoCmd::DUMP);
    hdr->status = 0;
    hdr->length = 0;

    outbufBusy = true;
    Send(hdr);
}

}   // namespace Net
}   // namespace DAVA
