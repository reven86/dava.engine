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


#ifndef __DAVAENGINE_SIMPLE_CONNECTION_LISTENER_PRIVATE_H__
#define __DAVAENGINE_SIMPLE_CONNECTION_LISTENER_PRIVATE_H__

#include "Concurrency/Atomic.h"
#include "Concurrency/ConcurrentObject.h"
#include "Concurrency/Spinlock.h"
#include "Concurrency/Thread.h"
#include "Network/SimpleNetworking/SimpleConnectionListener.h"

namespace DAVA
{
namespace Net
{

template <typename T>
using ConcurrentList = ConcurrentObject<List<T>>;

template <typename T>
using ConcurrentRefPtr = ConcurrentObject<RefPtr<T>, Spinlock>;

class ConnectionListenerPrivate
{
public:
    ConnectionListenerPrivate(const ConnectionWaitFunction& connWaiter,
                              const Endpoint& endPoint,
                              bool dontReceive);

    ConnectionListenerPrivate(IConnectionPtr& conn, bool dontReceive);
    ~ConnectionListenerPrivate();

    IConnectionPtr GetConnection() const;
    void AddConnectionCallback(const ConnectionCallback& cb);
    void AddDataReceiveCallback(const DataReceiveCallback& cb);
    void AddConnectionCloseCallback(const ConnectionCloseCallback& cb);

    void Start();
private:
    void Start(const ConnectionWaitFunction& connectionWaiter, const Endpoint& endPoint);
    void Start(IConnectionPtr& conn);

    RefPtr<Thread> thread;
    mutable ConcurrentRefPtr<IConnection> connection;
    ConcurrentList<ConnectionCallback> onConnectCallbacks;
    ConcurrentList<DataReceiveCallback> onDataReceiveCallbacks;
    ConcurrentList<ConnectionCloseCallback> onConnectionCloseCallbacks;
    bool dontReceiveData;
};

}  // namespace Net
}  // namespace DAVA

#endif  // __DAVAENGINE_SIMPLE_CONNECTION_LISTENER_PRIVATE_H__