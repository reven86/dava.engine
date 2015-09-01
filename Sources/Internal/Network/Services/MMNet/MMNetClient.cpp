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

#include "Functional/Function.h"

#include "Debug/DVAssert.h"

#include "FileSystem/DynamicMemoryFile.h"
#include "DLC/Patcher/ZLibStream.h"
#include "MemoryManager/MemoryManager.h"

#include "Network/Base/Endpoint.h"
#include "Network/Services/MMNet/MMNetClient.h"
#include "Network/Services/MMNet/MMBigDataTransferService.h"

namespace DAVA
{
namespace Net
{

MMNetClient::MMNetClient()
    : NetService()
    , transferService(new MMBigDataTransferService(CLIENT_ROLE))
{
}

MMNetClient::~MMNetClient() = default;

void MMNetClient::InstallCallbacks(ConnEstablishedCallback connEstablishedCallback_, ConnLostCallback connLostCallback_, StatCallback statCallback_, SnapshotCallback snapshotCallback_)
{
    connEstablishedCallback = connEstablishedCallback_;
    connLostCallback = connLostCallback_;
    statCallback = statCallback_;
    transferService->SetSnapshotCallback(snapshotCallback_);
}

void MMNetClient::RequestSnapshot()
{
    if (tokenRequested && canRequestSnapshot)
    {
        canRequestSnapshot = false; // Do not permit to request next snapshot until snapshot reply is received
        SendPacket(CreateHeaderOnlyPacket(MMNetProto::TYPE_REQUEST_SNAPSHOT, MMNetProto::STATUS_SUCCESS));
    }
}

void MMNetClient::ChannelOpen()
{
    canRequestSnapshot = true;
    SendPacket(CreateHeaderOnlyPacket(MMNetProto::TYPE_REQUEST_TOKEN, MMNetProto::STATUS_SUCCESS));
}

void MMNetClient::ChannelClosed(const char8* message)
{
    tokenRequested = false;
    
    packetQueue.clear();
    transferService->Stop();
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
            case MMNetProto::TYPE_REPLY_SNAPSHOT:
                ProcessReplySnapshot(header, static_cast<const void*>(header + 1), dataLength);
                break;
            case MMNetProto::TYPE_AUTO_STAT:
                ProcessAutoReplyStat(header, static_cast<const void*>(header + 1), dataLength);
                break;
            default:
                break;
        }
    }
}

void MMNetClient::PacketDelivered()
{
    DVASSERT(!packetQueue.empty());

    packetQueue.pop_front();
    if (!packetQueue.empty())
    {
        MMNetProto::Packet& x = packetQueue.front();
        Send(x.PlainBytes(), x.Header()->length);
    }
}

void MMNetClient::ProcessReplyToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    bool newSession = dataLength > 0;
    if (newSession)     // Config has come, new profiling session
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
    transferService->Start(newSession, connToken, channel->RemoteEndpoint().Address());
}

void MMNetClient::ProcessReplySnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    canRequestSnapshot = true;
}

void MMNetClient::ProcessAutoReplyStat(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    statCallback(static_cast<const MMCurStat*>(packetData), inHeader->itemCount);
}

void MMNetClient::SendPacket(MMNetProto::Packet&& packet)
{
    bool wasEmpty = packetQueue.empty();
    packetQueue.emplace_back(std::forward<MMNetProto::Packet>(packet));
    if (wasEmpty)
    {
        MMNetProto::Packet& x = packetQueue.front();
        Send(x.PlainBytes(), x.Header()->length);
    }
}

MMNetProto::Packet MMNetClient::CreateHeaderOnlyPacket(uint16 type, uint16 status)
{
    MMNetProto::Packet packet(0);

    MMNetProto::PacketHeader* header = packet.Header();
    header->length = sizeof(MMNetProto::PacketHeader);
    header->type = type;
    header->status = status;
    header->itemCount = 0;
    header->token = connToken;
    return packet;
}

}   // namespace Net
}   // namespace DAVA
