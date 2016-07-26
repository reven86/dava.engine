#include "AssetCacheHttpServer.h"

void AssetCacheHttpServer::Start(DAVA::uint16 port_)
{
    port = port_;
    DAVA::Net::HttpServer::Start(DAVA::Net::Endpoint(port));
}

void AssetCacheHttpServer::Stop()
{
    DAVA::Net::HttpServer::Stop();
}

void AssetCacheHttpServer::NotifyStatusRequested(void* channelId)
{
    if (listener)
        listener->OnStatusRequested(channelId);
}

void AssetCacheHttpServer::OnHttpServerStopped()
{
}

void AssetCacheHttpServer::OnHttpRequestReceived(void* channelId, DAVA::Net::HttpRequest& rq)
{
    if (rq.method == DAVA::Net::HttpRequest::GET && CompareCaseInsensitive(rq.uri, "/getstatus") == 0)
    {
        NotifyStatusRequested(channelId);
        return;
    }
    else
    {
        DAVA::Net::HttpResponse resp;
        resp.version = "HTTP/1.1";
        resp.code = "400 Bad Request";
        SendResponse(channelId, resp);
        return;
    }
}

void AssetCacheHttpServer::SendStatus(void* channelId, AssetServerStatus st)
{
    DAVA::Net::HttpResponse resp;
    resp.version = "HTTP/1.1";
    resp.code = "200 OK";
    resp.body = "{\n\t\"status\":\"started\",\n\t\"path\":\"" + st.assetServerPath + "\"\n}";
    SendResponse(channelId, resp);
}
