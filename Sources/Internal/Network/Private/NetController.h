#ifndef __DAVAENGINE_NETCONTROLLER_H__
#define __DAVAENGINE_NETCONTROLLER_H__

#include "Base/BaseTypes.h"

#include "Network/NetworkCommon.h"
#include "Network/IController.h"
#include "Network/Private/ITransport.h"

namespace DAVA
{
namespace Net
{
class IOLoop;
class ServiceRegistrar;
class NetConfig;
class ProtoDriver;

class NetController : public IController
                      ,
                      public IServerListener
                      ,
                      public IClientListener
{
private:
    struct ClientEntry
    {
        ClientEntry(IClientTransport* aClient, ProtoDriver* aDriver, IServerTransport* aParent = NULL);

        IClientTransport* client;
        IServerTransport* parent;
        ProtoDriver* driver;
    };

    friend bool operator==(const ClientEntry& entry, const IClientTransport* obj);

public:
    NetController(IOLoop* aLoop, const ServiceRegistrar& aRegistrar, void* aServiceContext, uint32 readTimeout = DEFAULT_READ_TIMEOUT);
    virtual ~NetController();

    bool ApplyConfig(const NetConfig& config, size_t trIndex = 0);

    // IController
    virtual void Start();
    virtual void Stop(Function<void(IController*)> handler);
    virtual void Restart();

    // IServerListener
    virtual void OnTransportSpawned(IServerTransport* parent, IClientTransport* child);
    virtual void OnTransportTerminated(IServerTransport* tr);

    // IClientListener
    virtual void OnTransportTerminated(IClientTransport* tr);
    virtual void OnTransportConnected(IClientTransport* tr, const Endpoint& endp);
    virtual void OnTransportDisconnected(IClientTransport* tr, int32 error);
    virtual void OnTransportDataReceived(IClientTransport* tr, const void* buffer, size_t length);
    virtual void OnTransportSendComplete(IClientTransport* tr);
    virtual void OnTransportReadTimeout(IClientTransport* tr);

private:
    void DoStartServers();
    void DoStartClients();
    void DoRestart();
    void DoStopServers();
    void DoStopClients();

    ClientEntry* GetClientEntry(IClientTransport* client);

    IServerTransport* CreateServerTransport(eTransportType type, const Endpoint& endpoint);
    IClientTransport* CreateClientTransport(eTransportType type, const Endpoint& endpoint);

private:
    IOLoop* loop;
    eNetworkRole role;
    const ServiceRegistrar& registrar;
    void* serviceContext;
    size_t runningObjects;
    Function<void(IController*)> stopHandler;
    bool isTerminating;
    uint32 readTimeout = 0;

    Vector<uint32> serviceIds;
    Vector<IServerTransport*> servers;
    List<ClientEntry> clients;
};

//////////////////////////////////////////////////////////////////////////
inline NetController::ClientEntry::ClientEntry(IClientTransport* aClient, ProtoDriver* aDriver, IServerTransport* aParent)
    : client(aClient)
    , parent(aParent)
    , driver(aDriver)
{
}

inline bool operator==(const NetController::ClientEntry& entry, const IClientTransport* obj)
{
    return entry.client == obj;
}

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_NETCONTROLLER_H__
