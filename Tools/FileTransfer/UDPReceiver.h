#ifndef __DAVAENGINE_UDPRECEIVER_H__
#define __DAVAENGINE_UDPRECEIVER_H__

#include <Network/UDPSocket.h>
#include <Network/DeadlineTimer.h>

namespace DAVA
{

class IOLoop;

class UDPReceiver
{
    static const std::size_t INBUF_SIZE = 1024;
    static const std::size_t OUTBUF_SIZE = 1024;

    enum {
        STAGE_INIT,
        STAGE_FILE
    };
    
public:
    UDPReceiver(IOLoop* loop);
    ~UDPReceiver();
    
    bool Start(uint16 port);
    
    std::size_t FileSize() const { return fileSize; }
    const char8* FileBuffer() const { return fileBuf; }
    
private:
    int32 IssueReadRequest();
    int32 IssueWriteRequest(const char8* buf, std::size_t length);
    void Cleanup();
    
    void SendInitReply();
    void SentFileReply();

    void HandleClose(UDPSocket* socket);
    void HandleTimer(DeadlineTimer* timer);
    void HandleReceive(UDPSocket* socket, int32 error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial);
    void HandleSend(UDPSocket* socket, int32 error, const void* buffer);

    bool CheckRemotePeer(const Endpoint& endpoint);
    
private:
    UDPSocket     socket;
    DeadlineTimer timer;
    Endpoint      endp;
    int32         stage;
    char8         inbuf[INBUF_SIZE];
    char8         outbuf[OUTBUF_SIZE];
    char8*        fileBuf;
    std::size_t   fileSize;
    std::size_t   recievedFileSize;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPRECEIVER_H__
