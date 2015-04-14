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

#include "Network/NetService.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MMNetProto.h"

namespace DAVA
{

class DynamicMemoryFile;

namespace Net
{

class MMNetServer : public NetService
{
    struct Parcel
    {
        size_t size;
        size_t nsent;
        size_t chunk;
        void* buffer;
    };

    static const size_t CHUNK_SIZE = 60 * 1024;

public:
    MMNetServer();
    virtual ~MMNetServer();

    void Update(float32 timeElapsed);

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;
    
private:
    void ProcessInitCommunication(const MMProtoHeader* hdr, const void* packet, size_t length);
    void ProcessDump(const MMProtoHeader* hdr, const void* packet, size_t length);
    void SendMemoryStat();

    Parcel CreateParcel(size_t parcelSize);
    Parcel CreateParcel(size_t parcelSize, void* buf);
    void DestroyParcel(Parcel parcel);
    void EnqueueAndSend(Parcel parcel);

    static void DumpRequestCallback(void* arg, int32 type, uint32 tagOrCheckpoint, uint32 blockBegin, uint32 blockEnd);
    void OnDumpRequest(int32 type, uint32 tagOrCheckpoint, uint32 blockBegin, uint32 blockEnd);

private:
    uint32 sessionId;
    bool commInited;
    uint64 timerBegin;
    size_t statPeriod;
    size_t periodCounter;
    volatile bool allDone;

    DynamicMemoryFile* zipFile;
    Deque<Parcel> parcels;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_MMNETSERVER_H__
