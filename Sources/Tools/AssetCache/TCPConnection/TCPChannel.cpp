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



#include "AssetCache/TCPConnection/TCPChannel.h"

#include "Base/FunctionTraits.h"

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"

#include "Network/NetworkCommon.h"
#include "Network/NetConfig.h"
#include "Network/NetCore.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{
    
TCPChannel::TCPChannel()
    : Net::NetService()
    , listener(nullptr)
{
}
    
    
TCPChannel::~TCPChannel()
{
    listener = nullptr;
}
    
void TCPChannel::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    Net::NetService::OnPacketSent(channel, buffer, length);

    // archieve has been sent and buffer can be deleted
    delete [] static_cast<const uint8*>(buffer);
}
    

bool TCPChannel::SendArchieve(KeyedArchive * archieve)
{
    DVASSERT(archieve);
    
    auto packedSize = archieve->Serialize(nullptr, 0);
    uint8 *packedData = new uint8[packedSize];
    
    DVVERIFY(packedSize == archieve->Serialize(packedData, packedSize));
    
    return SendData(packedData, packedSize);
}

bool TCPChannel::SendData(const DynamicMemoryFile* buffer)
{
    DVASSERT(buffer);
    return SendData(buffer->GetData(), buffer->GetSize());
}

bool TCPChannel::SendData(const Vector<uint8>& data)
{
    return SendData(data.data(), data.size());
}

bool TCPChannel::SendData(const uint8 * data, const size_t dataSize)
{
    uint32 packetID = 0;
    bool sent = Send(data, dataSize, &packetID);
    if (sent)
    {
        return (packetID != 0);
    }
    else
    {
        return false;
    }
}
    
    
void TCPChannel::ChannelOpen()
{
    if(listener)
    {
        listener->ChannelOpened(this);
    }
}

void TCPChannel::ChannelClosed(const char8* message)
{
    if (listener)
    {
        listener->ChannelClosed(this, message);
    }
}

void TCPChannel::PacketReceived(const void* packet, size_t length)
{
    if (listener)
    {
        listener->PacketReceived(this, static_cast<const uint8*>(packet), length);
    }
}

void TCPChannel::PacketSent()
{
    if (listener)
    {
        listener->PacketSent(this);
    }
}

void TCPChannel::PacketDelivered()
{
    if (listener)
    {
        listener->PacketDelivered(this);
    }
}
    


}; // end of namespace DAVA

