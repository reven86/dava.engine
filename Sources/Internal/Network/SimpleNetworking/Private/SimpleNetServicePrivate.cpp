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


#include "Network/SimpleNetworking/Private/SimpleNetServicePrivate.h"

#include "Concurrency/Thread.h"
#include <functional>
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace Net
{

class ChannelBridge : public IChannel
{
public:
    ChannelBridge(IConnectionPtr& conn, IChannelListener* listener);

    bool Send(const void* data, size_t length, uint32 flags, uint32* packetId) override;
    const Endpoint& RemoteEndpoint() const override;

private:
    IConnectionPtr connection;
    IChannelListener* channelListener;
};

ChannelBridge::ChannelBridge(IConnectionPtr& conn, IChannelListener* listener)
    : connection(conn)
    , channelListener(listener)
{
    DVASSERT_MSG(connection,      "Connection cannot be empty");
    DVASSERT_MSG(channelListener, "Channel listener cannot be empty");

    channelListener->OnChannelOpen(this);
}

bool ChannelBridge::Send(const void* data, size_t length, uint32 /*flags*/, uint32* /*packetId*/)
{
    const char* buf = reinterpret_cast<const char*>(data);
    size_t wrote = connection->Write(buf, length);
    return wrote == length;
}

const Endpoint& ChannelBridge::RemoteEndpoint() const
{
    return connection->GetEndpoint();
}

}  // namespace Net
}  // namespace DAVA