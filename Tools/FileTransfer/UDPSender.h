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
