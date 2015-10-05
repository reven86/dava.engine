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


#include "Concurrency/LockGuard.h"
#include "Network/SimpleNetworking/Private/ChannelAdapter.h"

namespace DAVA
{
namespace Net
{

ChannelAdapter::ChannelAdapter(IChannelListener* listener)
    : channelListener(listener)
{
    DVASSERT_MSG(channelListener, "Channel listener cannot be empty");
}

void ChannelAdapter::SetConnection(IConnectionPtr& conn)
{
    LockGuard<RecursiveMutex> guard(mutex);

    connection = conn;
    connectionWasHere = true;
    channelListener->OnChannelOpen(this);
}

void ChannelAdapter::RemoveConnection()
{
    LockGuard<RecursiveMutex> guard(mutex);
    if (!connection)
        return;
    
    //TODO: replace on reset
    connection = IConnectionPtr();
    channelListener->OnChannelClosed(this, "");
}

bool ChannelAdapter::Send(const void* data, size_t length, uint32 /*flags*/, uint32* packetId)
{
    LockGuard<RecursiveMutex> guard(mutex);

    if (!connection)
        return false;

    const char* buf = reinterpret_cast<const char*>(data);
    size_t written = connection->Write(buf, length);
    bool succeed = written == length;

    if (succeed)
    {
        channelListener->OnPacketSent(this, data, length);
        channelListener->OnPacketDelivered(this, packetId ? *packetId : 0);
    }
    return succeed;
}

void ChannelAdapter::Receive(const void* data, size_t length)
{
    LockGuard<RecursiveMutex> guard(mutex);
    channelListener->OnPacketReceived(this, data, length);
}

const Endpoint& ChannelAdapter::RemoteEndpoint() const
{
    LockGuard<RecursiveMutex> guard(mutex);
    return connection->GetEndpoint();
}

bool ChannelAdapter::IsSessionEnded() const 
{ 
    LockGuard<RecursiveMutex> guard(mutex);
    return connectionWasHere && !IsConnectionEstablished();
}

}  // namespace Net
}  // namespace DAVA