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
public:
    typedef Function<void(const MMStatConfig*)> ChOpenCallback;
    typedef Function<void (const char8*)> ChClosedCallback;       // TODO: change to void(const char*) after fixing TypeTraits and Function
    typedef Function<void(const MMStat*)> StatCallback;
    typedef Function<void(size_t total, size_t recv)> DumpGetCallback;
    typedef Function<void(const MMDump*, size_t)> DumpDoneCallback;

public:
    MMNetClient();
    virtual ~MMNetClient();

    void SetCallbacks(ChOpenCallback onOpen, ChClosedCallback onClosed, StatCallback onStat, DumpGetCallback onDumpGet, DumpDoneCallback onDumpDone);

    void RequestDump();

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;

private:
    void ProcessInitCommunication(const MMProtoHeader* hdr, const void* packet, size_t length);
    void ProcessCurrentStatistics(const MMProtoHeader* hdr, const void* packet, size_t length);
    void ProcessDump(const MMProtoHeader* hdr, const void* packet, size_t length);
    void ProcessDumpNext(const void* packet, size_t length);

    void SendInitSession();
    void SendDumpRequest();

    void UnpackDump();

private:
    uint32 sessionId;
    bool commInited;

    uint8 outbuf[128];
    bool outbufBusy;

    bool gettingDump;
    size_t dumpSize;
    size_t dumpRecv;
    size_t unpackedDumpSize;
    std::vector<uint8> dumpV;

    ChOpenCallback openCallback;
    ChClosedCallback closeCallback;
    StatCallback statCallback;
    DumpGetCallback dumpGetCallback;
    DumpDoneCallback dumpDoneCallback;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_MMNetClient_H__
