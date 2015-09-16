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


#ifndef __DAVAENGINE_PROTODRIVER_H__
#define __DAVAENGINE_PROTODRIVER_H__

#include <Base/BaseTypes.h>
#include <Concurrency/Atomic.h>
#include <Concurrency/Mutex.h>
#include <Concurrency/Spinlock.h>

#include <Network/Base/Endpoint.h>
#include <Network/NetworkCommon.h>
#include <Network/IChannel.h>

#include <Network/Private/ITransport.h>
#include <Network/Private/ProtoDecoder.h>

namespace DAVA
{
namespace Net
{

class IOLoop;
class ServiceRegistrar;

class ProtoDriver
{
private:
    struct Packet
    {
        uint32 channelId;
        uint32 packetId;
        uint8* data = nullptr;  // Data
        size_t dataLength;      //  and its length
        size_t sentLength;      // Number of bytes that have been already transfered
        size_t chunkLength;     // Number of bytes transfered during last operation
    };

    struct Channel : public IChannel
    {
        Channel(uint32 id, ProtoDriver* driver);

        bool Send(const void* data, size_t length, uint32 flags, uint32* packetId) override;
        const Endpoint& RemoteEndpoint() const override;

        bool confirmed;     // Channel is confirmed by other side
        uint32 channelId;
        Endpoint remoteEndpoint;
        ProtoDriver* driver = nullptr;
        IChannelListener* service = nullptr;
    };

    friend bool operator == (const Channel& ch, uint32 channelId);

    enum eSendingFrameType
    {
        SENDING_DATA_FRAME = false,
        SENDING_CONTROL_FRAME = true
    };

public:
    ProtoDriver(IOLoop* aLoop, eNetworkRole aRole, const ServiceRegistrar& aRegistrar, void* aServiceContext);
    ~ProtoDriver();

    void SetTransport(IClientTransport* aTransport, const uint32* sourceChannels, size_t channelCount);
    void SendData(uint32 channelId, const void* buffer, size_t length, uint32* outPacketId);

    void ReleaseServices();

    void OnConnected(const Endpoint& endp);
    void OnDisconnected(const char* message);
    bool OnDataReceived(const void* buffer, size_t length);
    void OnSendComplete();
    bool OnTimeout();

private:
    Channel* GetChannel(uint32 channelId);
    void SendControl(uint32 code, uint32 channelId, uint32 packetId);

    bool ProcessDataPacket(ProtoDecoder::DecodeResult* result);
    bool ProcessChannelQuery(ProtoDecoder::DecodeResult* result);
    bool ProcessChannelAllow(ProtoDecoder::DecodeResult* result);
    bool ProcessChannelDeny(ProtoDecoder::DecodeResult* result);
    bool ProcessDeliveryAck(ProtoDecoder::DecodeResult* result);

    void ClearQueues();

    void SendCurPacket();
    void SendCurControl();

    void PreparePacket(Packet* packet, uint32 channelId, const void* buffer, size_t length);
    bool EnqueuePacket(Packet* packet);
    bool DequeuePacket(Packet* dest);
    bool DequeueControl(ProtoHeader* dest);

private:
    IOLoop* loop = nullptr;
    eNetworkRole role;
    const ServiceRegistrar& registrar;
    void* serviceContext = nullptr;
    IClientTransport* transport = nullptr;
    Vector<Channel> channels;

    Spinlock senderLock;
    Mutex queueMutex;
    eSendingFrameType whatIsSending;
    bool pendingPong;

    Packet curPacket;
    Deque<Packet> dataQueue;
    Deque<uint32> pendingAckQueue;

    ProtoHeader curControl;
    Deque<ProtoHeader> controlQueue;

    ProtoDecoder proto;
    ProtoHeader header;

    static Atomic<uint32> nextPacketId;     // Global for all instances
};

//////////////////////////////////////////////////////////////////////////
inline ProtoDriver::Channel::Channel(uint32 id, ProtoDriver* aDriver)
    : confirmed(false)
    , channelId(id)
    , driver(aDriver)
    , service(NULL)
{

}

inline bool ProtoDriver::Channel::Send(const void* data, size_t length, uint32 flags, uint32* outPacketId)
{
    driver->SendData(channelId, data, length, outPacketId);
    return true;
}

inline const Endpoint& ProtoDriver::Channel::RemoteEndpoint() const
{
    return remoteEndpoint;
}

inline bool operator == (const ProtoDriver::Channel& ch, uint32 channelId)
{
    return ch.channelId == channelId;
}

inline ProtoDriver::Channel* ProtoDriver::GetChannel(uint32 channelId)
{
    Vector<Channel>::iterator i = std::find(channels.begin(), channels.end(), channelId);
    return i != channels.end() ? &*i
                               : NULL;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_PROTODRIVER_H__
