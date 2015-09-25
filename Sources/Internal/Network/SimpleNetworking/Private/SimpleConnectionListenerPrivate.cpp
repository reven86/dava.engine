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


#include "Network/SimpleNetworking/Private/SimpleConnectionListenerPrivate.h"

namespace DAVA
{
namespace Net
{

template <typename Container, typename ... Arg>
void Notify(const Container& container, const Arg&... args)
{
    for (const auto& cb : container)
    {
        cb(args...);
    }
}

ConnectionListenerPrivate::ConnectionListenerPrivate(const ConnectionWaitFunction& connWaiter,
                                                     const Endpoint& endPoint)
{
    ConnectionWaitFunction connWaiterLocal = connWaiter;
    Endpoint endPointLocal = endPoint;
    auto threadFunc = [this, connWaiterLocal, endPointLocal] 
    {
        Start(connWaiterLocal, endPointLocal);
    };

    thread = RefPtr<Thread>(Thread::Create(threadFunc));
}

ConnectionListenerPrivate::~ConnectionListenerPrivate()
{
    thread->Cancel();
    thread->Join();
}

ConnectionListenerPrivate::ConnectionListenerPrivate(IConnectionPtr& conn)
{
    auto threadFunc = [this](IConnectionPtr& connection) { Start(connection); };
    thread = RefPtr<Thread>(Thread::Create(std::bind(threadFunc, IConnectionPtr(conn))));
}

IConnectionPtr ConnectionListenerPrivate::GetConnection() const 
{ 
    return connection.Load();
}

void ConnectionListenerPrivate::AddConnectionCallback(const ConnectionCallback& cb) 
{
    onConnectCallbacks.GetAccessor()->emplace_back(cb);

    IConnectionPtr conn = GetConnection();
    if (conn)
    {
        cb(conn);
    }
}
void ConnectionListenerPrivate::AddDataReceiveCallback(const DataReceiveCallback& cb) 
{
    onDataReceiveCallbacks.GetAccessor()->emplace_back(cb);
}

void ConnectionListenerPrivate::AddConnectionCloseCallback(const ConnectionCloseCallback& cb)
{
    onConnectionCloseCallbacks.GetAccessor()->emplace_back(cb);
}

void ConnectionListenerPrivate::Start()
{
    thread->Start();
}

void ConnectionListenerPrivate::Start(const ConnectionWaitFunction& connectionWaiter, 
                                      const Endpoint& endPoint) 
{
    do
    {
        IConnectionPtr connection = connectionWaiter(endPoint);
        if (connection)
        {
            Notify(*onConnectCallbacks.GetAccessor(), connection);
            Start(connection);
        }

        Thread::Sleep(500);
    }
    while (waitSuccessfulConnection && !thread->IsCancelling());
}

void ConnectionListenerPrivate::Start(IConnectionPtr& conn) 
{
    Array<char, 4096> buffer;

    while (conn && 
           conn->GetChannelState() == IReadOnlyConnection::ChannelState::Connected &&
           !thread->IsCancelling())
    {
        size_t read = conn->ReadSome(buffer.data(), buffer.size());
        if (read == 0)
            break;

        DataBuffer buf(buffer.data(), buffer.data() + read);
        Notify(*onDataReceiveCallbacks.GetAccessor(), buf);
    }

    if (conn && (conn->ReadBytesCount() != 0 || conn->WrittenBytesCount() != 0))
    {
        waitSuccessfulConnection = false;
    }
    Notify(*onConnectionCloseCallbacks.GetAccessor());
}

}  // namespace Net
}  // namespace DAVA