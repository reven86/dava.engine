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

#include "Debug/DVAssert.h"

#include "MemoryManager/MemoryManager.h"

#include "MMNetServer.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cstdlib>

namespace DAVA
{
namespace Net
{

MMNetServer::MMNetServer()
    : NetService()
    , sessionId(rand())
    , commInited(false)
    , statPeriod(1000)
    , periodCounter(0)
{

}

MMNetServer::~MMNetServer()
{

}

void MMNetServer::Update(float32 timeElapsed)
{
    if (!commInited) return;

    periodCounter += static_cast<size_t>(timeElapsed * 1000.0f);
    if (periodCounter >= statPeriod)
    {
        periodCounter = 0;
    }
}

void MMNetServer::ChannelOpen()
{

}

void MMNetServer::ChannelClosed(const char8* message)
{
    commInited = false;

    for (auto x : parcels)
        DestroyParcel(x);
    parcels.clear();
}

void MMNetServer::PacketReceived(const void* packet, size_t length)
{
    DVASSERT(length >= sizeof(MMProtoHeader));

    const MMProtoHeader* hdr = static_cast<const MMProtoHeader*>(packet);
    const eMMProtoCmd cmd = static_cast<eMMProtoCmd>(hdr->cmd);
    switch (cmd)
    {
    case eMMProtoCmd::INIT_COMM:
        ProcessInitCommunication(hdr, static_cast<const uint8*>(packet) + sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    }
}

void MMNetServer::PacketDelivered()
{
    DVASSERT(!parcels.empty());

    if (!commInited)
    {
        // As reply to eMMProtoCmd::INIT_COMM is always first delivered packet after connection
        // so we can simply set
        commInited = true;
    }

    parcels.pop_front();
    if (!parcels.empty())
    {
        Parcel next = parcels.front();
        Send(next.buffer, next.size);
    }
}

void MMNetServer::ProcessInitCommunication(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    size_t dataSize = 0;
    if (hdr->sessionId != sessionId)
    {
        dataSize = MemoryManager::CalcStatConfigSize();
    }
    
    Parcel parcel = CreateParcel(sizeof(MMProtoHeader) + dataSize);
    if (dataSize > 0)
    {
        MMStatConfig* config = reinterpret_cast<MMStatConfig*>(static_cast<uint8*>(parcel.buffer) + sizeof(MMProtoHeader));
        MemoryManager::GetStatConfig(config);
    }
    
    MMProtoHeader* outHdr = static_cast<MMProtoHeader*>(parcel.buffer);
    outHdr->sessionId = sessionId;
    outHdr->cmd = static_cast<uint32>(eMMProtoCmd::INIT_COMM);
    outHdr->status = static_cast<uint32>(eMMProtoStatus::ACK);
    outHdr->length = static_cast<uint32>(dataSize);
    
    EnqueueAndSend(parcel);
}

MMNetServer::Parcel MMNetServer::CreateParcel(size_t parcelSize)
{
    Parcel parcel = {
        parcelSize,
        new uint8[parcelSize]
    };
    return parcel;
}

void MMNetServer::DestroyParcel(Parcel parcel)
{
    delete [] static_cast<uint8*>(parcel.buffer);
}

void MMNetServer::EnqueueAndSend(Parcel parcel)
{
    bool wasEmpty = parcels.empty();
    parcels.push_back(parcel);
    if (wasEmpty)
        Send(parcel.buffer, parcel.size);
}

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
