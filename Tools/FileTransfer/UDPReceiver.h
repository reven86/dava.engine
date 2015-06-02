/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
