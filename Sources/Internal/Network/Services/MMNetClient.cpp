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
{

}

MMNetClient::~MMNetClient()
{

}

void MMNetClient::SetCallbacks(ChOpenCallback onOpen, ChClosedCallback onClosed, StatCallback onStat)
{
    openCallback = onOpen;
    closeCallback = onClosed;
    statCallback = onStat;
}

void MMNetClient::ChannelOpen()
{
    SendInitSession();
}

void MMNetClient::ChannelClosed(const char8* message)
{
    commInited = false;
    outbufBusy = false;

    closeCallback(const_cast<char8*>(message)); // TODO: remove const_cast after fixing TypeTraits and Function
}

void MMNetClient::PacketReceived(const void* packet, size_t length)
{
    DVASSERT(length >= sizeof(MMProtoHeader));

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

    openCallback(const_cast<MMStatConfig*>(config));
}

void MMNetClient::ProcessCurrentStatistics(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    const MMStat* stat = static_cast<const MMStat*>(packet);
    statCallback(const_cast<MMStat*>(stat));
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

}   // namespace Net
}   // namespace DAVA
