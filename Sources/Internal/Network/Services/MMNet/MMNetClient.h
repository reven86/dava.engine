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

#ifndef __DAVAENGINE_MMNetClient_H__
#define __DAVAENGINE_MMNetClient_H__

#include "Base/BaseTypes.h"
#include "Base/Function.h"

#include "Network/NetService.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MMNetProto.h"

namespace DAVA
{
namespace Net
{

class MMNetClient : public NetService
{
    struct ParcelEx
    {
        ParcelEx()
            : bufferSize(0)
            , buffer(nullptr)
            , header(nullptr)
            , data(nullptr)
        {}
        ParcelEx(size_t dataSize)
            : bufferSize(sizeof(MMNetProto::PacketHeader) + dataSize)
            , buffer(::operator new(bufferSize))
            , header(static_cast<MMNetProto::PacketHeader*>(buffer))
            , data(static_cast<void*>(header + 1))
        {}
        
        size_t bufferSize;
        void* buffer;
        MMNetProto::PacketHeader* header;
        void* data;
    };
    
public:
    enum {
        DUMP_STAGE_STARTED = 0,
        DUMP_STAGE_PROGRESS,
        DUMP_STAGE_FINISHED,
        DUMP_STAGE_ERROR
    };

    using ConnEstablishedCallback = Function<void (bool, const MMStatConfig*)>;
    using ConnLostCallback = Function<void (const char8*)>;
    using StatCallback = Function<void (const MMCurStat*, size_t)>;
    using DumpCallback = Function<void (int, size_t, size_t, const void*)>;

public:
    MMNetClient();
    virtual ~MMNetClient();

    void InstallCallbacks(ConnEstablishedCallback connEstablishedCallback, ConnLostCallback connLostCallback, StatCallback statCallback, DumpCallback dumpCallback);

    void RequestDump();

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;

private:
    void ProcessReplyToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    void ProcessReplyDump(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    void ProcessAutoReplyStat(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    void ProcessAutoReplyDump(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    
    void FastRequest(uint16 type);
    
    void EnqueueParcel(const ParcelEx& parcel);
    void SendParcel(ParcelEx& parcel);
    
    void Cleanup();

private:
    uint32 connToken = 0;
    bool tokenRequested = false;
    
    List<ParcelEx> queue;

    bool canRequestDump = true;
    size_t dumpTotalSize = 0;
    size_t dumpRecvSize = 0;
    std::vector<uint8> dumpData;

    ConnEstablishedCallback connEstablishedCallback;
    ConnLostCallback connLostCallback;
    StatCallback statCallback;
    DumpCallback dumpCallback;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_MMNetClient_H__
