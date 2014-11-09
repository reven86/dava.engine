#include <Base/BaseTypes.h>
#include <Base/FunctionTraits.h>
#include <Network/NetworkUtils.h>
#include <Network/Buffer.h>

#include "CommonTypes.h"
#include "UDPSender.h"

namespace DAVA
{

UDPSender::UDPSender(IOLoop* loop) : socket(loop)
                                   , timer(loop)
                                   , endp()
                                   , stage(STAGE_PING)
                                   , pingSent(false)
                                   , fileBuf(NULL)
                                   , fileSize(0)
                                   , curOffset(0)
                                   , curChunkSize(0)
                                   , chunkSize(1000)
{
    Memset(inbuf, 0, sizeof(inbuf));
    Memset(outbuf, 0, sizeof(outbuf));

    socket.SetCloseHandler(MakeFunction(this, &UDPSender::HandleClose));
    socket.SetReceiveHandler(MakeFunction(this, &UDPSender::HandleReceive));
    socket.SetSendHandler(MakeFunction(this, &UDPSender::HandleSend));
}

UDPSender::~UDPSender()
{
    
}

bool UDPSender::Start(const Endpoint& endpoint, const char8* buffer, std::size_t length)
{
    endp     = endpoint;
    fileBuf  = buffer;
    fileSize = length;

    StartWait(0);
    return true;
}

int32 UDPSender::StartReceive()
{
    int32 error = socket.AsyncReceive(CreateBuffer(inbuf, INBUF_SIZE));
    if (error != 0)
    {
        printf("*** AsyncReceive failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPSender::StartWait(uint32 timeout)
{
    int32 error = timer.AsyncStartWait(timeout, MakeFunction(this, &UDPSender::HandleTimer));
    if (error != 0)
    {
        printf("*** AsyncStartWait failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

void UDPSender::SendPing()
{
    printf("Sending PING..\n");

    ProtoPing proto;
    proto.hdr.type = PROTO_PING;
    proto.hdr.size = sizeof(ProtoPing);
    proto.fileSize = static_cast<uint32>(fileSize);
    IssueWriteRequest(&proto);
}

void UDPSender::SendChunk()
{
    std::size_t nleft = fileSize - curOffset;
    curChunkSize      = Min(nleft, chunkSize);

    ProtoChunk proto;
    proto.hdr.type  = PROTO_CHUNK;
    proto.hdr.size  = sizeof(ProtoChunk) + curChunkSize;
    proto.chunkSize = curChunkSize;

    Buffer buf[2] = {
        CreateBuffer(&proto, 1),
        CreateBuffer(const_cast<char8*>(fileBuf + curOffset), curChunkSize)
    };
    IssueWriteRequestBuffers(buf, 2);
}

int32 UDPSender::IssueWriteRequestBuffers(const Buffer* buffers, std::size_t nbuffers)
{
    int32 error = socket.AsyncSend(endp, buffers, nbuffers);
    if (error != 0)
    {
        printf("*** AsyncSend failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPSender::IssueWriteRequestGeneric(const void* buffer, std::size_t length)
{
    Buffer buf = CreateBuffer(outbuf, length);
    Memcpy(outbuf, buffer, length);
    return IssueWriteRequestBuffers(&buf, 1);
}

void UDPSender::Cleanup()
{
    socket.Close();
    timer.Close();
}

void UDPSender::HandleClose(SocketType* socket)
{
    printf("Socket closed\n");
}

void UDPSender::HandleTimer(DeadlineTimer* timer)
{
    static int32 n = 0;
    n += 1;
    if (n > 30)
        Cleanup();
    else
        SendPing();
}

void UDPSender::HandleReceive(SocketType* socket, int32 error, std::size_t nread, const Endpoint& endpoint, bool partial)
{
    timer.StopAsyncWait();
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

    ProtoHeader* hdr = reinterpret_cast<ProtoHeader*>(inbuf);
    if (nread < hdr->size)
    {
        printf("*** Received too few bytes: nread=%u, proto.size=%u\n", nread, hdr->size);
        Cleanup();
        return;
    }

    if (PROTO_PONG == hdr->type)
    {
        if (STAGE_PING == stage)
        {
            ProtoPong* pong = reinterpret_cast<ProtoPong*>(inbuf);

            stage = STAGE_CHUNK;
            SendChunk();
        }
        else
            printf("Abandoned PONG received\n");
    }
    else if (PROTO_ACK == hdr->type)
    {
        if (STAGE_CHUNK == stage)
        {
            ProtoAck* ack = reinterpret_cast<ProtoAck*>(inbuf);
            SendChunk();
        }
        else
            printf("Abandoned ACK received\n");
    }
    else
    {
        printf("Unknown proto type: %u\n", hdr->type);
        Cleanup();
    }
}

void UDPSender::HandleSend(SocketType* socket, int32 error, const Buffer* buffers, std::size_t bufferCount)
{
    if (error != 0)
    {
        printf("*** Send failed: %s\n", NetworkErrorToString(error));
        Cleanup();
        return;
    }

    if (STAGE_PING == stage)
    {
        if (!pingSent)
        {
            StartReceive();
            pingSent = true;
        }
        StartWait(1000);
    }
    else if (STAGE_CHUNK == stage)
    {
        curOffset += curChunkSize;
        if (fileSize == curOffset)
        {
            printf("File has been sent\n");
            Cleanup();
        }
    }
}

}   // namespace DAVA
