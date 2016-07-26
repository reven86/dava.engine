#include "HttpServer.h"

namespace DAVA
{
namespace Net
{
HttpRequest::Method HttpMethodFromString(String& s)
{
    static const String strGet = "GET";
    if (CompareCaseInsensitive(s, strGet) == 0)
        return HttpRequest::GET;
    else
        return HttpRequest::UNEXPECTED;
}

bool HttpServer::Start(const Endpoint& endpoint)
{
    return tcpServer.Start(endpoint);
}

void HttpServer::Stop()
{
    tcpServer.Stop();
}

void HttpServer::SendResponse(void* channelId, HttpResponse& resp)
{
    String respString = resp.version + " " + resp.code + "\r\n\r\n" + resp.body;
    return tcpServer.SendData(channelId, respString.data(), respString.size());
}

void HttpServer::OnDataReceived(void* channelId, const void* buffer, size_t length)
{
    String dataString;
    const char* strbuffer = reinterpret_cast<const char*>(buffer);
    dataString.assign(strbuffer, length);

    std::stringstream dataStream;
    dataStream.str(dataString);

    String startingLine;
    std::getline(dataStream, startingLine);

    Vector<String> startingLineTokens;
    Split(startingLine, " ", startingLineTokens);
    size_t numOfTokens = startingLineTokens.size();

    HttpRequest request;
    if (numOfTokens)
    {
        request.method = HttpMethodFromString(startingLineTokens[0]);
    }

    if (numOfTokens > 1)
    {
        request.uri = startingLineTokens[1];
    }

    if (numOfTokens > 2)
    {
        request.version = startingLineTokens[2];
    }

    // remaining lines of http message (i.e. headers, message body) should be read here if needed

    OnHttpRequestReceived(channelId, request);
    return;
}
}
}
