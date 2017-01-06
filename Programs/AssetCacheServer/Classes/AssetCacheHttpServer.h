#pragma once

#include "HttpServer/HttpServer.h"
#include "Base/BaseTypes.h"

struct AssetCacheHttpServerListener
{
    virtual void OnStatusRequested(ClientID clientId){};
};

struct AssetServerStatus
{
    bool started = false;
    DAVA::String assetServerPath;
};

struct AssetCacheHttpServer : public HttpServerListener
{
    AssetCacheHttpServer(DAVA::Net::IOLoop*);

    void Start(DAVA::uint16 port);
    void Stop();

    DAVA::uint16 GetListenPort() const;

    void SendStatus(ClientID clientId, const AssetServerStatus& st);

    void SetListener(AssetCacheHttpServerListener*);

private:
    // HttpServerListener
    void OnHttpServerStopped() override;
    void OnHttpRequestReceived(ClientID clientId, HttpRequest& rq) override;

    void NotifyStatusRequested(ClientID clientId);

    HttpServer httpServer;
    AssetCacheHttpServerListener* listener = nullptr;
    DAVA::uint16 port = 0;
};

inline DAVA::uint16 AssetCacheHttpServer::GetListenPort() const
{
    return port;
}

inline void AssetCacheHttpServer::SetListener(AssetCacheHttpServerListener* listener_)
{
    listener = listener_;
}
