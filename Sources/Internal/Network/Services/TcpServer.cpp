#include "TcpServer.h"

namespace DAVA
{
namespace Net
{
TCPServer::~TCPServer()
{
    Stop();
}

bool TCPServer::Start(const Endpoint& endpoint)
{
    DVASSERT(started == false);
    serverTransport = new TCPServerTransport(loop, endpoint, 100000);
    serverTransport->Start(this);
    started = true;
    return true;
}

void TCPServer::Stop()
{
    if (started)
    {
        started = false;

        for (IClientTransport* client : clients)
        {
            RemoveClient(client);
        }

        if (serverTransport)
        {
            serverTransport->Stop();
            serverTransport = nullptr;
        }
    }
}

void TCPServer::RemoveClient(IClientTransport* tr)
{
    DVASSERT(tr);
    auto erasedCount = clients.erase(tr);
    if (erasedCount > 0)
    {
        tr->Stop();
    }
}

void TCPServer::SendData(void* channelId, const void* data, size_t size)
{
    DAVA::Net::Buffer buf = CreateBuffer(data, size);
    reinterpret_cast<IClientTransport*>(channelId)->Send(&buf, 1);
}

void TCPServer::OnTransportSpawned(IServerTransport* parent, IClientTransport* client)
{
    DVASSERT(parent == serverTransport && parent != nullptr);
    DVASSERT(client != nullptr);
    auto insertResult = clients.insert(client);
    DVASSERT(insertResult.second == true);
    client->Start(this);
}

void TCPServer::OnTransportTerminated(IServerTransport* tr)
{
    NotifyServerTerminated();
}

void TCPServer::OnTransportTerminated(IClientTransport* tr)
{
    RemoveClient(tr);
}

void TCPServer::OnTransportConnected(IClientTransport* tr, const Endpoint& endp)
{
}

void TCPServer::OnTransportDisconnected(IClientTransport* tr, int32 error)
{
    RemoveClient(tr);
}

void TCPServer::OnTransportDataReceived(IClientTransport* tr, const void* buffer, size_t length)
{
    listener.OnDataReceived(tr, buffer, length);
}

void TCPServer::OnTransportSendComplete(IClientTransport* tr)
{
    RemoveClient(tr);
}

void TCPServer::OnTransportReadTimeout(IClientTransport* tr)
{
}

void TCPServer::NotifyServerTerminated()
{
    listener.OnServerTerminated();
}

void TCPServer::NotifyDataReceived(void* channelId, const void* buffer, size_t length)
{
    listener.OnDataReceived(channelId, buffer, length);
}
}
}
