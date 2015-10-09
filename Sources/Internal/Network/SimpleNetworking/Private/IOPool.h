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


#ifndef __DAVAENGINE_IO_POOL_H__
#define __DAVAENGINE_IO_POOL_H__

#include "Functional/Function.h"
#include "Network/Base/Endpoint.h"
#include "Network/SimpleNetworking/IConnection.h"
#include "Network/SimpleNetworking/Private/Common.h"

namespace DAVA
{
namespace Net
{

class IOPool
{
public:
    using Procedure = Function<void()>;
    using ListenCallback = Function<void(socket_t)>;
    using SendCallback = IConnection::WriteCallback;
    using RecvCallback = IConnection::ReadCallback;
    using BufferPtr = std::unique_ptr<const char[]>;

    void AddConnectOperation(socket_t sock, const Endpoint& endPoint, const Procedure& cb) {}
    void AddListenOperation(socket_t sock, const ListenCallback& cb) {}
    void AddSendOperation(socket_t sock,
                          BufferPtr&& data,
                          size_t size, 
                          const SendCallback& cb = nullptr) {}
    void AddReceiveOperation(socket_t sock, 
                             const RecvCallback& cb,
                             size_t* recvBytesCount = nullptr) {}

    enum OperationType
    {
        ConnectOperation = 0x1,
        ListenOperation = 0x2,
        SendOperation = 0x4,
        ReceiveOperation = 0x8,
        AllOperations = ConnectOperation | ListenOperation | SendOperation | ReceiveOperation
    };

    void Execute(OperationType opType = AllOperations, bool force = false) {}
    void CancelAll() {}

private:
};

}  // namespace Net
}  // namespace DAVA

#endif  // __DAVAENGINE_IO_POOL_H__