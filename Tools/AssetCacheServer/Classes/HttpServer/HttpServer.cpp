#include "HttpServer/HttpServer.h"

#include "Network/Private/TCPServerTransport.h"
#include "Utils/Utils.h"

namespace HttpServerDetail
{
const DAVA::uint32 PACKET_WAIT_TIMEOUT_MS = 5 * 1000;
const DAVA::String CRLF = "\r\n";
const DAVA::String DOUBLE_CRLF = "\r\n\r\n";

HttpRequest::Method HttpMethodFromString(const DAVA::String& s)
{
    static const DAVA::String strGet = "GET";
    if (s == strGet)
        return HttpRequest::GET;
    else
        return HttpRequest::UNEXPECTED;
}
} // namespace HttpServerDetail

HttpServer::HttpServer(DAVA::Net::IOLoop* loop)
    : loop(loop)
{
}

HttpServer::~HttpServer()
{
    Stop();
}

void HttpServer::AddListener(HttpServerListener* listener)
{
    auto insertRes = listeners.insert(listener);
    DVASSERT_MSG(insertRes.second == true, "Listener was already inserted");
}

void HttpServer::RemoveListener(HttpServerListener* listener)
{
    size_t erasedCount = listeners.erase(listener);
    DVASSERT_MSG(erasedCount == 1, "Listener was already removed");
}

void HttpServer::Start(const DAVA::Net::Endpoint& endpoint)
{
    DVASSERT(started == false);
    DVASSERT(serverTransportHolder == nullptr);
    serverTransportHolder = new ServerTransportHolder(loop, endpoint, HttpServerDetail::PACKET_WAIT_TIMEOUT_MS);
    serverTransportHolder->SetOwner(this);
    serverTransportHolder->Start();
    started = true;
}

void HttpServer::Stop()
{
    if (started)
    {
        started = false;

        RemoveAllClients();

        if (serverTransportHolder)
        {
            serverTransportHolder->SetOwner(nullptr);
            serverTransportHolder->Stop();
            serverTransportHolder = nullptr;
        }

        NotifyServerStopped();
    }
}

void HttpServer::RemoveClient(DAVA::Net::IClientTransport* clt)
{
    DVASSERT(clt);

    size_t erasedCount = clientSessions.erase(clt);
    if (erasedCount > 0)
    {
        clt->Stop();
    }
}

void HttpServer::RemoveAllClients()
{
    for (auto& session : clientSessions)
    {
        DAVA::Net::IClientTransport* clt = session.first;
        clt->Stop();
    }
    clientSessions.clear();
}

void HttpServer::OnTransportSpawned(DAVA::Net::IServerTransport* parent, DAVA::Net::IClientTransport* client)
{
    clientSessions.emplace(std::make_pair(client, ClientSession(client)));
}

void HttpServer::OnTransportTerminated(DAVA::Net::IServerTransport* serv)
{
    Stop();
}

void HttpServer::OnTransportTerminated(DAVA::Net::IClientTransport* clt)
{
    RemoveClient(clt);
}

void HttpServer::OnTransportConnected(DAVA::Net::IClientTransport* clt, const DAVA::Net::Endpoint& endp)
{
}

void HttpServer::OnTransportDisconnected(DAVA::Net::IClientTransport* clt, DAVA::int32 error)
{
    RemoveClient(clt);
}

void HttpServer::OnTransportDataReceived(DAVA::Net::IClientTransport* clt, const void* buffer, size_t length)
{
    const auto& sessionEntry = clientSessions.find(clt);
    if (sessionEntry != clientSessions.end())
    {
        ClientSession& session = sessionEntry->second;

        if (session.state == ClientSession::WaitingForHeaderPart || session.state == ClientSession::WaitingForContentPart)
        {
            const char* strbuffer = static_cast<const char*>(buffer);
            session.data.append(strbuffer, length);
            OnDataChunkAdded(session);
        }
        else
        {
            DAVA::Logger::Error("Data receiving is unexpected for clientID=%d", session.clientID);
            RemoveClient(clt);
            return;
        }
    }
    else
    {
        DVASSERT_MSG(false, DAVA::Format("Can't find session for client transport %d", clt).c_str());
    }
}

void HttpServer::OnTransportSendComplete(DAVA::Net::IClientTransport* clt)
{
    const auto& sessionEntry = clientSessions.find(clt);
    if (sessionEntry != clientSessions.end())
    {
        ClientSession& session = sessionEntry->second;

        if (session.state == ClientSession::WaitingForSendComplete)
        {
            RemoveClient(clt);
            return;
        }
        else
        {
            DAVA::Logger::Error("Unexpected send complete is received for current state %d", session.state);
            RemoveClient(clt);
            return;
        }
    }
    else
    {
        DVASSERT_MSG(false, DAVA::Format("Can't find session for client transport %d", clt).c_str());
    }
}

void HttpServer::OnTransportReadTimeout(DAVA::Net::IClientTransport* clt)
{
    RemoveClient(clt);
}

void HttpServer::OnDataChunkAdded(ClientSession& session)
{
    switch (session.state)
    {
    case ClientSession::WaitingForHeaderPart:
    {
        size_t crlfPos = session.data.rfind(HttpServerDetail::DOUBLE_CRLF);
        if (crlfPos == std::string::npos)
        {
            return; // continue to wait for header part
        }

        ParseHttpHeaders(session);

        switch (session.state)
        {
        case ClientSession::WaitingForContentPart:
        {
            session.data.erase(0, crlfPos + HttpServerDetail::DOUBLE_CRLF.length());
            OnDataChunkAdded(session);
            return;
        }
        case ClientSession::RequestReady:
        {
            OnRequestAssembled(session);
            return;
        }
        case ClientSession::Error:
        default:
        {
            RemoveClient(session.clientID);
            return;
        }
        }
        break;
    }
    case ClientSession::WaitingForContentPart:
    {
        if (session.data.size() >= session.contentLength)
        {
            OnRequestAssembled(session);
            return;
        }
        else
        {
            return; // continue to wait for remaining content
        }
    }
    default:
    {
        DAVA::Logger::Error("Unexpected state: %d", session.state);
        RemoveClient(session.clientID);
        return;
    }
    }
}

HttpServer::ClientSession::ClientSession(ClientID clt)
    : clientID(clt)
{
    Clear();
}

void HttpServer::ClientSession::Clear()
{
    contentLength = 0;
    data.clear();
    request.method = HttpRequest::UNEXPECTED;
    request.uri.clear();
    request.version.clear();
}

void HttpServer::OnRequestAssembled(ClientSession& session)
{
    session.state = ClientSession::WaitingForOurResponse;
    NotifyRequestReceived(session.clientID, session.request);
}

void HttpServer::ParseHttpHeaders(ClientSession& session)
{
    DAVA::Vector<DAVA::String> lines;
    DAVA::Split(session.data, HttpServerDetail::CRLF, lines, false, false, true);

    if (lines.empty())
    {
        DAVA::Logger::Error("Can't detect any lines in request string '%s'", session.data.c_str());
        session.state = ClientSession::Error;
        return;
    }

    DAVA::String& startingLine = lines[0];

    DAVA::Vector<DAVA::String> startingLineTokens;
    DAVA::Split(startingLine, " ", startingLineTokens);
    size_t numOfTokens = startingLineTokens.size();

    if (numOfTokens > 0)
    {
        session.request.method = HttpServerDetail::HttpMethodFromString(startingLineTokens[0]);

        if (numOfTokens > 1)
        {
            session.request.uri = startingLineTokens[1];

            if (numOfTokens > 2)
            {
                session.request.version = startingLineTokens[2];
            }
        }
    }

    for (auto linesIt = lines.begin() + 1; linesIt != lines.end(); ++linesIt)
    {
        DAVA::String& headerLine = *linesIt;

        DAVA::Vector<DAVA::String> tokens;
        DAVA::Split(headerLine, " ", tokens);
        if (tokens.size() >= 2 && DAVA::CompareCaseInsensitive(tokens[0], "Content-Length:") == 0)
        {
            int scannedCount = sscanf(tokens[1].c_str(), "%u", &session.contentLength);
            if (scannedCount == 1)
            {
                session.state = ClientSession::WaitingForContentPart;
            }
            else
            {
                DAVA::Logger::Warning("Can't parse content-length from '%s' token", tokens[1].c_str());
                session.state = ClientSession::Error;
            }
            return;
        }

        // other headers (Accept etc.) should be read here if needed
    };

    session.state = ClientSession::RequestReady;
}

void HttpServer::NotifyServerStopped()
{
    for (HttpServerListener* listener : listeners)
    {
        listener->OnHttpServerStopped();
    }
}

void HttpServer::NotifyRequestReceived(ClientID clientId, HttpRequest& rq)
{
    for (HttpServerListener* listener : listeners)
    {
        listener->OnHttpRequestReceived(clientId, rq);
    }
}

void HttpServer::SendResponse(ClientID clientId, HttpResponse& resp)
{
    const auto& sessionEntry = clientSessions.find(clientId);
    if (sessionEntry != clientSessions.end())
    {
        ClientSession& session = sessionEntry->second;

        if (session.state == ClientSession::WaitingForOurResponse)
        {
            session.data =
            resp.version + " " + resp.code + HttpServerDetail::CRLF +
            "Content-Length: " + std::to_string(resp.body.size()) + HttpServerDetail::CRLF +
            "Connection: close" + HttpServerDetail::DOUBLE_CRLF +
            resp.body;
            session.state = ClientSession::WaitingForSendComplete;
            SendRawData(clientId, session.data.data(), session.data.size());
            return;
        }
        else
        {
            DAVA::Logger::Error("Sending response is unexpected for current state %d", session.state);
            RemoveClient(clientId);
            return;
        }
    }
    else
    {
        DVASSERT_MSG(false, DAVA::Format("Can't find session for client transport %d", clientId).c_str());
    }
}

void HttpServer::SendRawData(ClientID clientId, const void* data, size_t size)
{
    DAVA::Net::Buffer buf = DAVA::Net::CreateBuffer(data, size);
    clientId->Send(&buf, 1);
}
