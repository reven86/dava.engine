#include "HttpServer/ServerTransportHolder.h"
#include "Job/JobManager.h"
#include "Functional/Function.h"

ServerTransportHolder::ServerTransportHolder(DAVA::Net::IOLoop* aLoop, const DAVA::Net::Endpoint& aEndpoint, DAVA::uint32 readTimeout)
    : serverTransport(aLoop, aEndpoint, readTimeout)
{
}

ServerTransportHolder::~ServerTransportHolder()
{
    DVASSERT(isWorking == false);
}

DAVA::int32 ServerTransportHolder::Start()
{
    DVASSERT(isWorking == false);
    isWorking = true;
    return serverTransport.Start(this);
}

void ServerTransportHolder::Stop()
{
    if (isWorking)
    {
        serverTransport.Stop();
    }
    else if (owner == nullptr)
    {
        DAVA::JobManager::Instance()->CreateWorkerJob(DAVA::MakeFunction(this, &ServerTransportHolder::DeleteItself));
    }
    else
    {
        DVASSERT_MSG(false, "Owner stops server without unsubscribing. Memory leak is possible");
    }
}

void ServerTransportHolder::OnTransportSpawned(DAVA::Net::IServerTransport* parent, DAVA::Net::IClientTransport* client)
{
    if (owner)
    {
        owner->OnTransportSpawned(parent, client);
        client->Start(this);
    }
}

void ServerTransportHolder::OnTransportTerminated(DAVA::Net::IServerTransport* serv)
{
    isWorking = false;

    if (owner)
    {
        IServerListener* serverListener = owner;
        serverListener->OnTransportTerminated(serv);
    }
    else
    {
        DAVA::JobManager::Instance()->CreateWorkerJob(DAVA::MakeFunction(this, &ServerTransportHolder::DeleteItself));
    }
}

void ServerTransportHolder::DeleteItself()
{
    DVASSERT(isWorking == false);
    delete this;
}

void ServerTransportHolder::OnTransportTerminated(DAVA::Net::IClientTransport* clt)
{
    if (owner)
    {
        IClientListener* clientListener = owner;
        clientListener->OnTransportTerminated(clt);
    }

    serverTransport.ReclaimClient(clt);
}

void ServerTransportHolder::OnTransportConnected(DAVA::Net::IClientTransport* clt, const DAVA::Net::Endpoint& endp)
{
    if (owner)
        owner->OnTransportConnected(clt, endp);
}

void ServerTransportHolder::OnTransportDisconnected(DAVA::Net::IClientTransport* clt, DAVA::int32 error)
{
    if (owner)
        owner->OnTransportDisconnected(clt, error);
}

void ServerTransportHolder::OnTransportDataReceived(DAVA::Net::IClientTransport* clt, const void* buffer, size_t length)
{
    if (owner)
        owner->OnTransportDataReceived(clt, buffer, length);
}

void ServerTransportHolder::OnTransportSendComplete(DAVA::Net::IClientTransport* clt)
{
    if (owner)
        owner->OnTransportSendComplete(clt);
}

void ServerTransportHolder::OnTransportReadTimeout(DAVA::Net::IClientTransport* clt)
{
    if (owner)
        owner->OnTransportReadTimeout(clt);
}
