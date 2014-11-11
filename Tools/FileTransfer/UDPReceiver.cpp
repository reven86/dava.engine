#include <Base/BaseTypes.h>
#include <Base/FunctionTraits.h>
#include <Network/NetworkUtils.h>

#include "CommonTypes.h"
#include "UDPReceiver.h"

namespace DAVA
{

UDPReceiver::UDPReceiver(IOLoop* loop) : socket(loop)
                                       , timer(loop)
                                       , endp()
                                       , stage(STAGE_INIT)
                                       , fileBuf(NULL)
                                       , fileSize(0)
                                       , recievedFileSize(0)
{
    socket.SetCloseHandler(MakeFunction(this, &UDPReceiver::HandleClose));
}

UDPReceiver::~UDPReceiver()
{
    delete [] fileBuf;
}

bool UDPReceiver::Start(uint16 port)
{
    int32 error = socket.Bind(port);
    if (0 == error)
    {
        error = IssueReadRequest();
    }
    else
    {
        printf("*** Bind failed: %s\n", NetworkErrorToString(error));
    }
    return 0 == error;
}

int32 UDPReceiver::IssueReadRequest()
{
    int32 error = socket.AsyncReceive(inbuf, INBUF_SIZE, MakeFunction(this, &UDPReceiver::HandleReceive));
    if (error != 0)
    {
        printf("*** AsyncReceive failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPReceiver::IssueWriteRequest(const char8* buf, std::size_t length)
{
    Memcpy(outbuf, buf, length);
    int32 error = socket.AsyncSend(endp, outbuf, length, MakeFunction(this, &UDPReceiver::HandleSend));
    if (error != 0)
    {
        printf("*** AsyncSend failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

void UDPReceiver::Cleanup()
{
    socket.Close();
    timer.Close();
}

void UDPReceiver::SendInitReply()
{
    InitReply rep = {0};
    rep.sign   = 0xACCA10DA;
    rep.status = 0;
    IssueWriteRequest(reinterpret_cast<const char8*>(&rep), sizeof(InitReply));
}

void UDPReceiver::SentFileReply()
{
    FileReply rep = {0};
    rep.sign   = 0xBABAABBA;
    rep.status = 0;
    IssueWriteRequest(reinterpret_cast<const char8*>(&rep), sizeof(FileReply));
}

void UDPReceiver::HandleClose(UDPSocket* socket)
{
    printf("Socket closed\n");
}

void UDPReceiver::HandleTimer(DeadlineTimer* timer)
{
}

void UDPReceiver::HandleReceive(UDPSocket* socket, int32 error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial)
{
    if (error != 0 || partial)
    {
        if (partial)
            printf("*** Partial packet received\n");
        else
            printf("*** Receive failed: %s\n", NetworkErrorToString(error));
        Cleanup();
        return;
    }

    static int n = 0;
    ++n;
    printf("Recieved %d, nread=%u\n", n, nread);
    //Sleep(1000);
#if 0
    if (!CheckRemotePeer(endpoint))
    {
        printf("*** Unexpected peer %s\n", endpoint.ToString().c_str());
        Cleanup();
        return;
    }

    if (STAGE_INIT == stage)
    {
        InitRequest* req = reinterpret_cast<InitRequest*>(inbuf);
        if (nread == sizeof(InitRequest) && req->fileSize > 0 && req->sign == 0xDEADBEEF)
        {
            fileSize = static_cast<std::size_t>(req->fileSize);
            fileBuf  = new char8[fileSize];
            stage    = STAGE_FILE;

            printf("Sender catched: filesize="SIZET_FMT"\n", fileSize);

            SendInitReply();
        }
        else
        {
            printf("*** Unrecognizable packet: nread="SIZET_FMT", size=%u, sign=%08X\n", nread, req->fileSize, req->sign);
            Cleanup();
            return;
        }
    }
    else if (STAGE_FILE == stage)
    {
        std::size_t nleft = fileSize - recievedFileSize;
        if (nread > nleft)
        {
            printf("*** More data arrived: fileSize="SIZET_FMT", recievedFileSize="SIZET_FMT", nread="SIZET_FMT"\n", fileSize
                                                                                                                   , recievedFileSize
                                                                                                                   , nread);
            Cleanup();
            return;
        }
        Memcpy(fileBuf, inbuf, nread);
        recievedFileSize += nread;
        printf("Data arrived: nread="SIZET_FMT", nleft="SIZET_FMT"\n", nread, fileSize - recievedFileSize);
        if (fileSize == recievedFileSize)
        {
            printf("File completely received\n");
            Cleanup();
            return;
        }
        else
            SentFileReply();
    }
    //IssueReadRequest();
#endif
}

void UDPReceiver::HandleSend(UDPSocket* socket, int32 error, const void* buffer)
{
    if (0 == error)
    {
    }
    else
    {
        printf("*** Send failed: %s\n", NetworkErrorToString(error));
        Cleanup();
    }
}

bool UDPReceiver::CheckRemotePeer(const Endpoint& endpoint)
{
    if (endp.Address().IsUnspecified())
    {
        endp = endpoint;
        return true;
    }
    else if (endpoint.Address().ToULong() == endp.Address().ToULong())
        return true;
    return false;
}

}   // namespace DAVA
