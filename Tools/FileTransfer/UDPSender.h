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
    static const std::size_t INBUF_SIZE = 32;
    static const std::size_t OUTBUF_SIZE = 1024;

    enum {
        STAGE_INIT,
        STAGE_FILE
    };

public:
    typedef UDPSocketEx SocketType;

    UDPSender(IOLoop* loop);
    ~UDPSender();

    bool Start(const Endpoint& endpoint, const char8* buffer, std::size_t length);

private:
    int32 IssueReadRequest();
    int32 IssueWriteRequest();
    int32 IssueWriteRequest(const char8* buf, std::size_t length);
    int32 IssueWriteRequest(std::size_t offset, std::size_t length);
    int32 IssueWaitRequest(uint32 timeout);
    template<typename T>
    int32 IssueWriteRequest(const T* val)
    {
        return IssueWriteRequest(reinterpret_cast<const char8*>(val), sizeof(T));
    }
    void Cleanup();

    void SendInit();
    void SendNextChunk();

    void HandleClose(SocketType* socket);
    void HandleTimer(DeadlineTimer* timer);
    void HandleReceive(SocketType* socket, int32 error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial);
    void HandleSend(SocketType* socket, int32 error, const void* buffer);

private:
    SocketType    socket;
    DeadlineTimer timer;
    Endpoint      endp;
    int32         stage;
    bool          initSent;
    char8         inbuf[INBUF_SIZE];
    char8         outbuf[OUTBUF_SIZE];
    const char8*  fileBuf;
    std::size_t   fileSize;
    std::size_t   curOffset;
    std::size_t   curChunkSize;
    std::size_t   chunkSize;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPSENDER_H__
