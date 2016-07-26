#pragma once

#include "Network/Services/TcpServer.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace Net
{
struct HttpRequest
{
    enum Method
    {
        GET,
        UNEXPECTED
    }; // other methods can be added here: HEAD, POST etc

    Method method = UNEXPECTED;
    String uri;
    String version;
};

HttpRequest::Method HttpMethodFromString(String& s);

struct HttpResponse
{
    String version;
    String code;
    String body;
};

class HttpServer : public TCPServerListener
{
public:
    HttpServer(IOLoop* loop_)
        : tcpServer(*this, loop_)
    {
    }

    bool Start(const Endpoint& endpoint_);
    void Stop();
    bool IsStarted();

protected:
    void SendResponse(void* channelId, HttpResponse&);

    virtual void OnHttpServerStopped() = 0;
    virtual void OnHttpRequestReceived(void* channelId, HttpRequest&) = 0;

private:
    // TCPServerListener
    void OnDataReceived(void* channelId, const void* buffer, size_t length);

    TCPServer tcpServer;
};

inline bool HttpServer::IsStarted()
{
    return tcpServer.IsStarted();
}
}
}
