#include "HttpServer/ServerTransportHolder.h"
#include "Job/JobManager.h"
#include "Functional/Function.h"

ServerTransportHolder::ServerTransportHolder(DAVA::Net::IOLoop* aLoop, const DAVA::Net::Endpoint& aEndpoint, DAVA::uint32 readTimeout)
    : serverTransport(aLoop, aEndpoint, readTimeout)
{
}

DAVA::int32 ServerTransportHolder::Start()
{
    isStarted = true;
    return serverTransport.Start(this);
}

void ServerTransportHolder::Stop()
{
    if (isStarted)
    {
        serverTransport.Stop();
    }
    else if (listener == nullptr)
    {
        DAVA::JobManager::Instance()->CreateWorkerJob(DAVA::MakeFunction(this, &ServerTransportHolder::DeleteItself));
    }
    else
    {
        DVASSERT_MSG(false, "Listener stops server without unsubscribing. Memory leak is possible");
    }
}

void ServerTransportHolder::OnTransportSpawned(DAVA::Net::IServerTransport* parent, DAVA::Net::IClientTransport* client)
{
    if (listener)
        listener->OnTransportSpawned(parent, client);

    client->Start(this);
}

void ServerTransportHolder::OnTransportTerminated(DAVA::Net::IServerTransport* serv)
{
    isStarted = false;

    if (listener)
    {
        IServerListener* serverListener = listener;
        serverListener->OnTransportTerminated(serv);
    }
    else
    {
        DAVA::JobManager::Instance()->CreateWorkerJob(DAVA::MakeFunction(this, &ServerTransportHolder::DeleteItself));
    }
}

void ServerTransportHolder::DeleteItself()
{
    DVASSERT(isStarted == false);
    delete this;
}

void ServerTransportHolder::OnTransportTerminated(DAVA::Net::IClientTransport* clt)
{
    if (listener)
    {
        IClientListener* clientListener = listener;
        clientListener->OnTransportTerminated(clt);
    }

    serverTransport.ReclaimClient(clt);
}

void ServerTransportHolder::OnTransportConnected(DAVA::Net::IClientTransport* clt, const DAVA::Net::Endpoint& endp)
{
    if (listener)
        listener->OnTransportConnected(clt, endp);
}

void ServerTransportHolder::OnTransportDisconnected(DAVA::Net::IClientTransport* clt, DAVA::int32 error)
{
    if (listener)
        listener->OnTransportDisconnected(clt, error);
}

void ServerTransportHolder::OnTransportDataReceived(DAVA::Net::IClientTransport* clt, const void* buffer, size_t length)
{
    if (listener)
        listener->OnTransportDataReceived(clt, buffer, length);
}

void ServerTransportHolder::OnTransportSendComplete(DAVA::Net::IClientTransport* clt)
{
    if (listener)
        listener->OnTransportSendComplete(clt);
}

void ServerTransportHolder::OnTransportReadTimeout(DAVA::Net::IClientTransport* clt)
{
    if (listener)
        listener->OnTransportReadTimeout(clt);
}
