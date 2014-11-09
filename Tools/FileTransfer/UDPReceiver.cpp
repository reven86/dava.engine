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
    socket.SetCloseHandler(MakeFunction(this, &UDPReceiver::HandleCloseSocket));
    timer.SetCloseHandler(MakeFunction(this, &UDPReceiver::HandleCloseTimer));
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
        StartWait(5000);
        error = IssueReadRequest();
        //endp = Endpoint("127.0.0.1", 9999);
        IssueWriteRequest("XXX", 3);
    }
    else
    {
        printf("*** Bind failed: %s\n", NetworkErrorToString(error));
    }
    return 0 == error;
}

void UDPReceiver::StartWait(uint32 timeout)
{
    timer.AsyncStartWait(timeout, MakeFunction(this, &UDPReceiver::HandleTimer));
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
    ProtoPong pong;
    pong.hdr.type = PROTO_PONG;
    pong.hdr.size = sizeof(ProtoPong);
    IssueWriteRequest(reinterpret_cast<const char8*>(&pong), sizeof(ProtoPong));
}

void UDPReceiver::SentFileReply()
{
    ProtoAck ack;
    ack.hdr.type = PROTO_ACK;
    ack.hdr.size = sizeof(ProtoAck);
    IssueWriteRequest(reinterpret_cast<const char8*>(&ack), sizeof(ProtoAck));
}

void UDPReceiver::HandleCloseSocket(UDPSocket* socket)
{
    printf("Receiver socket closed\n");
}

void UDPReceiver::HandleCloseTimer(DeadlineTimer* timer)
{
    printf("Receiver timer closed\n");
}

void UDPReceiver::HandleTimer(DeadlineTimer* timer)
{
    printf("Receive timeout !!!\n");
    Cleanup();
}

void UDPReceiver::HandleReceive(UDPSocket* socket, int32 error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial)
{
    static int n = 0;
    n += 1;
    inbuf[nread] = '\0';
    printf("%d. Got %u bytes - %s\n", n, (uint32)nread, inbuf);
    StartWait(5000);
    //IssueReadRequest();
    
/*
    if (error != 0 || partial)
    {
        if (partial)
            printf("*** Partial packet received\n");
        else
            printf("*** Receive failed: %s\n", NetworkErrorToString(error));
        Cleanup();
        return;
    }
    if (nread < sizeof(ProtoHeader))
    {
        printf("*** Received too few bytes: nread=%u\n", nread);
        Cleanup();
        return;
    }
    if (!CheckRemotePeer(endpoint))
    {
        printf("*** Unexpected peer %s\n", endpoint.ToString().c_str());
        Cleanup();
        return;
    }

    ProtoHeader* hdr = reinterpret_cast<ProtoHeader*>(inbuf);
    if (nread < hdr->size)
    {
        printf("*** Received too few bytes: nread=%u, proto.size=%u\n", nread, hdr->size);
        Cleanup();
        return;
    }

    if (PROTO_PING == hdr->type)
    {
        if (STAGE_INIT == stage)
        {
            ProtoPing* ping = reinterpret_cast<ProtoPing*>(inbuf);
            printf("PING: nread=%u, filesize=%u\n", nread, ping->fileSize);

            fileSize = ping->fileSize;
            fileBuf  = new char8[fileSize];
            stage    = STAGE_FILE;

            SendInitReply();
        }
        else
            printf("Out of order PING received\n");
    }
    else if (PROTO_CHUNK == hdr->type)
    {
        if (STAGE_FILE == stage)
        {
            ProtoChunk* chunk = reinterpret_cast<ProtoChunk*>(inbuf);
            char8*      ptr   = inbuf + sizeof(ProtoChunk);

            printf("PING: nread=%u, chunksize=%u\n", nread, chunk->chunkSize);

            Memcpy(fileBuf, ptr, chunk->chunkSize);
            recievedFileSize += chunk->chunkSize;
            if (fileSize == recievedFileSize)
            {
                printf("File completely received\n");
                Cleanup();
                return;
            }
            else
                SentFileReply();
        }
        else
            printf("Out of order CHUNK received\n");
    }
    else
    {
        printf("Unknown proto type: %u\n", hdr->type);
        Cleanup();
    }
*/
}

void UDPReceiver::HandleSend(UDPSocket* socket, int32 error, const void* buffer)
{
    if (0 == error)
    {
        IssueWriteRequest("XXX", 3);
    }
    else
    {
        printf("*** Send failed: %s (%s)\n", NetworkErrorToString(error), endp.ToString().c_str());
        //Cleanup();
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
