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


#include "Network/SimpleNetworking/ConnectionImpl.h"

#include "Debug/DVAssert.h"
#include "Network/SimpleNetworking/SimpleAbstractSocket.h"

namespace DAVA
{
namespace Net
{
    
ConnectionImpl::ConnectionImpl(ISimpleAbstractSocketPtr&& abstractSocket)
    : socket(std::move(abstractSocket)) 
{
    DVASSERT_MSG(socket, "Socket cannot be empty");
}
    
IReadOnlyConnection::ChannelState ConnectionImpl::GetChannelState()
{
    bool connectionEstablished = socket->IsConnectionEstablished();
    return connectionEstablished ? ChannelState::kConnected : ChannelState::kDisconnected;
}

size_t ConnectionImpl::ReadSome(char* buffer, size_t bufSize)
{
    size_t read = socket->Recv(buffer, bufSize, false);
    return read > 0;
}

bool ConnectionImpl::ReadAll(char* buffer, size_t bufSize)
{
    size_t read = socket->Recv(buffer, bufSize, true);
    return read > 0;
}

size_t ConnectionImpl::Write(const char* buffer, size_t bufSize)
{
    size_t wrote = socket->Send(buffer, bufSize);
    return wrote;
}
    
}  // namespace Net
}  // namespace DAVA