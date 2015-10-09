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


#ifndef __DAVAENGINE_SIMPLE_TCP_SOCKET_H__
#define __DAVAENGINE_SIMPLE_TCP_SOCKET_H__

#include "Network/Base/Endpoint.h"
#include "Network/SimpleNetworking/Private/Common.h"
#include "Network/SimpleNetworking/Private/IOPool.h"
#include "Network/SimpleNetworking/Private/SimpleAbstractSocket.h"

namespace DAVA
{
namespace Net
{

class SimpleTcpSocket : public ISimpleAbstractSocket
{
public:
    SimpleTcpSocket(IOPool* ioPoolPtr, const Endpoint& endPoint);
    ~SimpleTcpSocket();
    
    const Endpoint& GetEndpoint() override { return socketEndPoint; }
    bool Shutdown() override;
    
    bool Send(IOPool::BufferPtr&& buf, size_t bufSize, 
              const IOPool::SendCallback& cb = nullptr) override;
    bool Recv(const IOPool::RecvCallback& cb, size_t* recvBytesCount = nullptr) override;

    bool IsConnectionEstablished() override { return connectionEstablished; }

    bool IsValid() override { return socketId != DV_INVALID_SOCKET; }
    
protected:
    void Close();
    
    IOPool* ioPool;
    bool connectionEstablished = false;
    Endpoint socketEndPoint;
    socket_t socketId;
};

}  // namespace Net
}  // namespace DAVA

#endif  // __DAVAENGINE_SIMPLE_TCP_SOCKET_H__