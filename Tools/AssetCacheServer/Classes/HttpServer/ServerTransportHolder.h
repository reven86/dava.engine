#pragma once

#include "Network/Private/ITransport.h"
#include "Network/Private/TCPServerTransport.h"

namespace DAVA
{
namespace Net
{
class IOLoop;
}
}

struct ServerTransportListener
: public DAVA::Net::IServerListener
  ,
  public DAVA::Net::IClientListener
{
    //     // IServerListener
    //     void OnTransportSpawned(DAVA::Net::IServerTransport* parent, DAVA::Net::IClientTransport* child) override {};
    //     void OnTransportTerminated(DAVA::Net::IServerTransport* tr) override {};
    //
    //     // IClientListener
    //     void OnTransportTerminated(DAVA::Net::IClientTransport* tr) override {};
    //     void OnTransportConnected(DAVA::Net::IClientTransport* tr, const DAVA::Net::Endpoint& endp) override {};
    //     void OnTransportDisconnected(DAVA::Net::IClientTransport* tr, DAVA::int32 error) override {};
    //     void OnTransportDataReceived(DAVA::Net::IClientTransport* tr, const void* buffer, size_t length) override {};
    //     void OnTransportSendComplete(DAVA::Net::IClientTransport* tr) override {};
    //     void OnTransportReadTimeout(DAVA::Net::IClientTransport* tr) override {};
};

class ServerTransportHolder
: public DAVA::Net::IServerListener
  ,
  public DAVA::Net::IClientListener
{
public:
    ServerTransportHolder(DAVA::Net::IOLoop* aLoop, const DAVA::Net::Endpoint& aEndpoint, DAVA::uint32 readTimeout);

    DAVA::int32 Start();
    void Stop();
    void Reset();
    void ReclaimClient(DAVA::Net::IClientTransport* client);

    void SetListener(ServerTransportListener* listener);

private:
    // IServerListener
    void OnTransportSpawned(DAVA::Net::IServerTransport* parent, DAVA::Net::IClientTransport* child) override;
    void OnTransportTerminated(DAVA::Net::IServerTransport* tr) override;

    // IClientListener
    void OnTransportTerminated(DAVA::Net::IClientTransport* tr) override;
    void OnTransportConnected(DAVA::Net::IClientTransport* tr, const DAVA::Net::Endpoint& endp) override;
    void OnTransportDisconnected(DAVA::Net::IClientTransport* tr, DAVA::int32 error) override;
    void OnTransportDataReceived(DAVA::Net::IClientTransport* tr, const void* buffer, size_t length) override;
    void OnTransportSendComplete(DAVA::Net::IClientTransport* tr) override;
    void OnTransportReadTimeout(DAVA::Net::IClientTransport* tr) override;

    void DeleteItself();

private:
    bool isStarted = false;
    DAVA::Net::TCPServerTransport serverTransport;
    ServerTransportListener* listener = nullptr;
};

inline void ServerTransportHolder::SetListener(ServerTransportListener* listener_)
{
    listener = listener_;
}
