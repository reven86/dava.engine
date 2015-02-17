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
        delete [] x.buffer;
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
        ProcessInitCommunication(hdr, static_cast<const uint8*>(packet)+sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    }
}

void MMNetServer::PacketDelivered()
{
    DVASSERT(!parcels.empty());

    if (!commInited)
    {
        Parcel parcel = parcels.front();
        MMProtoHeader* hdr = static_cast<MMProtoHeader*>(parcel.buffer);
        if (static_cast<uint32>(eMMProtoCmd::INIT_COMM) == hdr->cmd)
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
    GeneralInfo* generalInfo = hdr->sessionId != sessionId ? MemoryManager::GetGeneralInfo()
                                                           : nullptr;
    ReplyInitSession(generalInfo);
    delete [] reinterpret_cast<uint8*>(generalInfo);    // TODO: i don't like such cast and delete
}

void MMNetServer::ReplyInitSession(const GeneralInfo* generalInfo)
{
    size_t generalInfoSize = 0;
    if (generalInfo != nullptr)
    {
        const size_t ntotal = generalInfo->tagCount + generalInfo->allocPoolCount + generalInfo->counterCount + generalInfo->poolCounterCount;
        generalInfoSize = sizeof(GeneralInfo) + (ntotal - 1) * GeneralInfo::NAME_LENGTH;
    }

    Parcel parcel = CreateParcel(sizeof(MMProtoHeader) + generalInfoSize);

    MMProtoHeader* hdr = static_cast<MMProtoHeader*>(parcel.buffer);
    hdr->sessionId = sessionId;
    hdr->cmd = static_cast<uint32>(eMMProtoCmd::INIT_COMM);
    hdr->status = static_cast<uint32>(eMMProtoStatus::ACK);
    hdr->length = static_cast<uint32>(generalInfoSize);

    if (generalInfo != nullptr)
        Memcpy(hdr + 1, generalInfo, generalInfoSize);

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
