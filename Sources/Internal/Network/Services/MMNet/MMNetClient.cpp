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
    , dumpTotalSize(0)
    , dumpRecvSize(0)
{

}

MMNetClient::~MMNetClient()
{
}

void MMNetClient::InstallCallbacks(ConnEstablishedCallback connEstablishedCallback_, ConnLostCallback connLostCallback_, StatCallback statCallback_, DumpCallback dumpCallback_)
{
    connEstablishedCallback = connEstablishedCallback_;
    connLostCallback = connLostCallback_;
    statCallback = statCallback_;
    dumpCallback = dumpCallback_;
}

void MMNetClient::RequestDump()
{
    if (tokenRequested && canRequestDump)
    {
        canRequestDump = false;
        FastRequest(MMNetProto::TYPE_REQUEST_DUMP);
    }
}

void MMNetClient::ChannelOpen()
{
    FastRequest(MMNetProto::TYPE_REQUEST_TOKEN);
}

void MMNetClient::ChannelClosed(const char8* message)
{
    tokenRequested = false;
    canRequestDump = true;
    
    Cleanup();
    connLostCallback(message);
}

void MMNetClient::PacketReceived(const void* packet, size_t length)
{
    const size_t dataLength = length - sizeof(MMNetProto::PacketHeader);
    const MMNetProto::PacketHeader* header = static_cast<const MMNetProto::PacketHeader*>(packet);
    if (length >= sizeof(MMNetProto::PacketHeader) && header->length == length)
    {
        switch (header->type)
        {
            case MMNetProto::TYPE_REPLY_TOKEN:
                ProcessReplyToken(header, static_cast<const void*>(header + 1), dataLength);
                break;
            case MMNetProto::TYPE_REPLY_DUMP:
                ProcessReplyDump(header, static_cast<const void*>(header + 1), dataLength);
                break;
            case MMNetProto::TYPE_AUTO_STAT:
                ProcessAutoReplyStat(header, static_cast<const void*>(header + 1), dataLength);
                break;
            case MMNetProto::TYPE_AUTO_DUMP:
                ProcessAutoReplyDump(header, static_cast<const void*>(header + 1), dataLength);
                break;
            default:
                break;
        }
    }
}

void MMNetClient::ProcessReplyToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    if (dataLength > 0)     // Config has come, new profiling session
    {
        connToken = inHeader->token;
        const MMStatConfig* config = static_cast<const MMStatConfig*>(packetData);
        DVASSERT(config->size == dataLength);
        connEstablishedCallback(false, config);
    }
    else    // Resume previous profiling session
    {
        connEstablishedCallback(true, nullptr);
    }
    tokenRequested = true;
}

void MMNetClient::ProcessReplyDump(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    canRequestDump = true;
}

void MMNetClient::ProcessAutoReplyStat(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    statCallback(static_cast<const MMCurStat*>(packetData), inHeader->itemCount);
}

void MMNetClient::ProcessAutoReplyDump(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    if (inHeader->status == MMNetProto::STATUS_SUCCESS)
    {
        const MMNetProto::PacketParamDump* param = static_cast<const MMNetProto::PacketParamDump*>(packetData);
        const void* data = static_cast<const void*>(param + 1);
        
        if (0 == dumpTotalSize)
        {
            dumpRecvSize = 0;
            dumpTotalSize = param->dumpSize;
            dumpData.resize(dumpTotalSize);

            dumpCallback(DUMP_STAGE_STARTED, dumpTotalSize, 0, nullptr);
        }
        
        Memcpy(&*dumpData.begin() + dumpRecvSize, data, param->chunkSize);
        dumpRecvSize += param->chunkSize;

        if (dumpRecvSize == dumpTotalSize)
        {
            dumpCallback(DUMP_STAGE_FINISHED, dumpTotalSize, dumpRecvSize, static_cast<const void*>(dumpData.data()));

            dumpTotalSize = 0;
            dumpRecvSize = 0;
            dumpData.clear();
        }
        else
        {
            dumpCallback(DUMP_STAGE_PROGRESS, dumpTotalSize, dumpRecvSize, nullptr);
        }
    }
    else
    {
        dumpCallback(DUMP_STAGE_ERROR, 0, 0, nullptr);

        dumpTotalSize = 0;
        dumpRecvSize = 0;
        dumpData.clear();
    }
}
    
void MMNetClient::FastRequest(uint16 type)
{
    ParcelEx parcel(0);
    parcel.header->length = uint32(sizeof(MMNetProto::PacketHeader));
    parcel.header->type = type;
    parcel.header->status = MMNetProto::STATUS_SUCCESS;
    parcel.header->itemCount = 0;
    parcel.header->token = connToken;
    
    EnqueueParcel(parcel);
}

void MMNetClient::EnqueueParcel(const ParcelEx& parcel)
{
    bool wasEmpty = queue.empty();
    queue.push_back(parcel);
    if (wasEmpty)
    {
        SendParcel(queue.front());
    }
}

void MMNetClient::SendParcel(ParcelEx& parcel)
{
    Send(parcel.buffer, parcel.header->length);
}

void MMNetClient::Cleanup()
{
    for (auto& parcel : queue)
    {
        ::operator delete(parcel.buffer);
    }
    queue.clear();
}

void MMNetClient::PacketDelivered()
{
    DVASSERT(!queue.empty());
    
    ParcelEx parcel = queue.front();
    queue.pop_front();
    
    ::operator delete(parcel.buffer);
    
    if (!queue.empty())
    {
        SendParcel(queue.front());
    }
}

}   // namespace Net
}   // namespace DAVA
