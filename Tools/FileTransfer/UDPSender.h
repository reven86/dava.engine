#ifndef __DAVAENGINE_UDPSENDER_H__
#define __DAVAENGINE_UDPSENDER_H__

#include <Network/UDPSocket.h>
#include <Network/UDPSocketEx.h>
#include <Network/DeadlineTimer.h>

namespace DAVA
{

class IOLoop;

class UDPSender
{
    static const std::size_t INBUF_SIZE = 1024;
    static const std::size_t OUTBUF_SIZE = 1024;

    enum {
        STAGE_PING,
        STAGE_CHUNK
    };

public:
    typedef UDPSocketEx SocketType;

    UDPSender(IOLoop* loop);
    ~UDPSender();

    bool Start(const Endpoint& endpoint, const char8* buffer, std::size_t length);

private:
    int32 StartReceive();
    int32 StartWait(uint32 timeout);

    void SendPing();
    void SendChunk();

    int32 IssueWriteRequestBuffers(const Buffer* buffers, std::size_t nbuffers);
    int32 IssueWriteRequestGeneric(const void* buffer, std::size_t length);
    template<typename T>
    int32 IssueWriteRequest(const T* val, std::size_t count = 1);

    void Cleanup();

    void HandleClose(SocketType* socket);
    void HandleTimer(DeadlineTimer* timer);
    void HandleReceive(SocketType* socket, int32 error, std::size_t nread, const Endpoint& endpoint, bool partial);
    void HandleSend(SocketType* socket, int32 error, const Buffer* buffers, std::size_t bufferCount);

private:
    SocketType    socket;
    DeadlineTimer timer;
    Endpoint      endp;
    int32         stage;
    bool          pingSent;
    char8         inbuf[INBUF_SIZE];
    char8         outbuf[OUTBUF_SIZE];
    const char8*  fileBuf;
    std::size_t   fileSize;
    std::size_t   curOffset;
    std::size_t   curChunkSize;
    std::size_t   chunkSize;
};

template<typename T>
int32 UDPSender::IssueWriteRequest(const T* val, std::size_t count)
{
    return IssueWriteRequestGeneric(val, sizeof(T) * count);
}

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPSENDER_H__
