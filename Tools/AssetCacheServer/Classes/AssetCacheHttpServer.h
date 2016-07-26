#pragma once

#include "Network/Services/HttpServer.h"
#include "Base/BaseTypes.h"

struct AssetCacheHttpServerListener
{
    virtual void OnStatusRequested(void* channelId)
    {
    }
};

struct AssetServerStatus
{
    bool started = false;
    DAVA::String assetServerPath;
};

struct AssetCacheHttpServer : public DAVA::Net::HttpServer
{
    AssetCacheHttpServer(DAVA::Net::IOLoop* loop_)
        : DAVA::Net::HttpServer(loop_)
    {
    }

    void Start(DAVA::uint16 port);
    void Stop();

    DAVA::uint16 GetListenPort() const;

    void SendStatus(void* channelId, AssetServerStatus st);

    void SetListener(AssetCacheHttpServerListener*);

private:
    void OnHttpServerStopped() override;
    void OnHttpRequestReceived(void* channelId, DAVA::Net::HttpRequest&) override;

    void NotifyStatusRequested(void* channelId);

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
