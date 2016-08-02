#include "AssetCacheHttpServer.h"

AssetCacheHttpServer::AssetCacheHttpServer(DAVA::Net::IOLoop* loop)
    : httpServer(loop)
{
}

void AssetCacheHttpServer::Start(DAVA::uint16 port_)
{
    port = port_;
    httpServer.AddListener(this);
    httpServer.Start(DAVA::Net::Endpoint(port));
}

void AssetCacheHttpServer::Stop()
{
    httpServer.RemoveListener(this);
    httpServer.Stop();
}

void AssetCacheHttpServer::NotifyStatusRequested(ClientID clientId)
{
    if (listener)
        listener->OnStatusRequested(clientId);
}

void AssetCacheHttpServer::OnHttpServerStopped()
{
    Stop();
}

void AssetCacheHttpServer::OnHttpRequestReceived(ClientID clientId, HttpRequest& rq)
{
    if (rq.method == HttpRequest::GET && rq.uri == "/getstatus")
    {
        NotifyStatusRequested(clientId);
        return;
    }
    else
    {
        HttpResponse resp;
        resp.version = "HTTP/1.1";
        resp.code = "400 Bad Request";
        httpServer.SendResponse(clientId, resp);
        return;
    }
}

void AssetCacheHttpServer::SendStatus(ClientID clientId, const AssetServerStatus& st)
{
    HttpResponse resp;
    resp.version = "HTTP/1.1";
    resp.code = "200 OK";
    resp.body = "{\n\t\"status\":\"started\",\n\t\"path\":\"" + st.assetServerPath + "\"\n}";
    httpServer.SendResponse(clientId, resp);
}
