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

#ifndef __DAVAENGINE_MMNETSERVER_H__
#define __DAVAENGINE_MMNETSERVER_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "Network/NetService.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MMNetProto.h"

namespace DAVA
{

namespace Net
{

class MMNetServer : public NetService
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
            : bufferSize(sizeof(MMNetProto::Header) + dataSize)
            , buffer(::operator new(bufferSize))
            , header(static_cast<MMNetProto::Header*>(buffer))
            , data(static_cast<void*>(header + 1))
        {}
        ParcelEx(void* buf, size_t dataSize)
            : bufferSize(sizeof(MMNetProto::Header) + dataSize)
            , buffer(buf)
            , header(static_cast<MMNetProto::Header*>(buffer))
            , data(static_cast<void*>(header + 1))
        {}

        template<typename T>
        T* Header() const
        {
            return reinterpret_cast<T*>(header);
        }

        size_t bufferSize;
        void* buffer;
        MMNetProto::Header* header;
        void* data;
    };
    struct Parcel
    {
        Parcel() : header(), data(nullptr), dataSize(0), dataSent(0), chunkSize(0) {}
        Parcel(void* dataBuf, size_t size) : header(), data(dataBuf), dataSize(size), dataSent(0), chunkSize(0) {}

        MMNetProto::Header header;
        void* data;
        size_t dataSize;
        size_t dataSent;
        size_t chunkSize;
    };

public:
    MMNetServer();
    virtual ~MMNetServer();

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;
    
private:
    void ProcessTypeInit(const MMNetProto::HeaderInit* header, const void* packetData, size_t dataLength);
    void ProcessTypeDump(const MMNetProto::HeaderDump* header, const void* packetData, size_t dataLength);

    void SendMemoryStat();

    void DeleteParcelData(Parcel& parcel);
    void EnqueueParcel(const Parcel& parcel);
    void SendParcel(Parcel& parcel);

    void GatherDump();

    void OnUpdate();
    void OnTag(uint32 tag, bool entering);

    void GetDump(uint64 timestamp);

private:
    uint32 sessionId;
    bool commInited;
    uint64 timerBegin;
    size_t statPeriod;
    size_t periodCounter;

    static const size_t OUTBUF_SIZE = 60 * 1024;
    static const size_t OUTBUF_USEFUL_SIZE = OUTBUF_SIZE - sizeof(MMNetProto::Header);
    uint8 outbuf[OUTBUF_SIZE];
    MMNetProto::Header* outHeader;
    void* outData;

    List<ParcelEx> queueEx;
    List<Parcel> queue;
    List<String> readyDumps;
};

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MMNETSERVER_H__
