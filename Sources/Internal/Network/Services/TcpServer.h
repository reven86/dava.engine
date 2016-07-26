#pragma once

#include "Debug/DVAssert.h"
#include "Network/Private/TCPServerTransport.h"
#include "Network/Private/TCPClientTransport.h"

namespace DAVA
{
namespace Net
{
struct TCPServerListener
{
    virtual void OnServerTerminated()
    {
    }
    virtual void OnDataReceived(void* channelId, const void* buffer, size_t length)
    {
    }
};

class TCPServer : public IServerListener, public IClientListener
{
public:
    TCPServer(TCPServerListener& listener_, IOLoop* loop_)
        : listener(listener_)
        , loop(loop_)
    {
    }
    ~TCPServer() override;

    bool Start(const Endpoint& endpoint_);
    void Stop();
    void SendData(void* channelId, const void* data, size_t size);
    bool IsStarted()
    {
        return started;
    }

protected:
    // IServerListener
    void OnTransportSpawned(IServerTransport* parent, IClientTransport* child) override;
    void OnTransportTerminated(IServerTransport* tr) override;

    // IClientListener
    void OnTransportTerminated(IClientTransport* tr) override;
    void OnTransportConnected(IClientTransport* tr, const Endpoint& endp) override;
    void OnTransportDisconnected(IClientTransport* tr, int32 error) override;
    void OnTransportDataReceived(IClientTransport* tr, const void* buffer, size_t length) override;
    void OnTransportSendComplete(IClientTransport* tr) override;
    void OnTransportReadTimeout(IClientTransport* tr) override;

    void RemoveClient(IClientTransport* tr);

    void NotifyServerTerminated();
    void NotifyDataReceived(void* channelId, const void* buffer, size_t length);

private:
    TCPServerListener& listener;
    TCPServerTransport* serverTransport = nullptr;
    UnorderedSet<IClientTransport*> clients;
    IOLoop* loop = nullptr;
    bool started = false;
};
}
}
