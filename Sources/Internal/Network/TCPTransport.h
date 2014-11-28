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

#ifndef __DAVAENGINE_TCPTRANSPORT_H__
#define __DAVAENGINE_TCPTRANSPORT_H__

#include <Base/Noncopyable.h>
#include <Thread/Spinlock.h>
#include <Platform/Mutex.h>

#include <Network/Base/Endpoint.h>
#include <Network/Base/Buffer.h>
#include <Network/Base/TCPAcceptor.h>
#include <Network/Base/TCPSocket.h>
#include <Network/Base/AsyncRequest.h>

#include "NetworkCommon.h"
#include "ITransport.h"
#include "BasicProtoDecoder.h"

namespace DAVA
{

class IOLoop;

/*
 Class TCPTransport implements data transfer over TCP using basic packet protocol.

 TCPTransport can be used in two roles:
    - TRANSPORT_SERVER_ROLE: listens incoming connections
    - TRANSPORT_CLIENT_ROLE: connects to remote server
 There is restriction when operating as server: only one connection supported at a time.

 TCPTransport takes pointer to interface ITransportListener through which it notifies client
 about data arrival or errors.

 Sending data:
    - data are sent in the order of Send method calls
    - if neccesary data buffer can be sent in several iterations (in the case of buffer is bigger than
      basic protocol packet size)
    - send buffer must be valid until it is sent
    - incoming data buffers are queued

 Receiving data:
    - data always gathered till full basic protocol packet

 All operations are performed in context of the thread running IOLoop's method Run (as required by libuv).
*/
class TCPTransport : public ITransport
                   , private Noncopyable
{
private:
    // Package - small object holding information about send buffer
    struct Package
    {
        uint32       channelId;
        const uint8* buffer;        // Buffer and
        size_t       totalLength;   //  its length
        size_t       sentLength;    // Number of bytes that have been already transfered
        size_t       partLength;    // Number of bytes transfered in last iteration
    };

protected:
    // Make constructor and destructor protected to allow only friend TransportFactory to create and destroy transports
    friend class TransportFactory;

    TCPTransport(IOLoop* loop, ITransportListener* aListener, eTransportRole aRole, const Endpoint& endp);
    virtual ~TCPTransport();

public:
    // ITransport
    virtual bool IsActive() const;
    virtual void Activate();
    virtual void Deactivate();
    virtual void Send(uint32 channelId, const uint8* buffer, size_t length);

private:
    void StartAsServer();
    void StartAsClient();

private:
    void CleanUp(eDeactivationReason reason, int32 error);
    void ClearQueue();
    void CloseSocket();

    void PreparePackage(Package* package, uint32 channelId, const uint8* buffer, size_t length);
    bool Enqueue(uint32 channelId, const uint8* buffer, size_t length);
    bool Dequeue(Package* target);
    void SendPackage(Package* package);

    void HandleWake(AsyncRequest* async);

    void AcceptorHandleClose(TCPAcceptor* acceptor);
    void AcceptorHandleConnect(TCPAcceptor* acceptor, int32 error);

    void SocketHandleClose(TCPSocket* socket);
    void SocketHandleShutdown(TCPSocket* socket, int32 error);
    void SocketHandleConnect(TCPSocket* socket, int32 error);
    void SocketHandleRead(TCPSocket* socket, int32 error, size_t nread);
    void SocketHandleWrite(TCPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount);

    inline bool LockSender();
    inline void UnlockSender();

private:
    eTransportRole      role;
    TCPAcceptor         acceptor;
    TCPSocket           socket;
    AsyncRequest        async;
    Endpoint            endpoint;
    ITransportListener* listener;

    bool isActive;
    bool terminateFlag;
    bool sendFlag;

    size_t        totalDataSize;                // Total data size to accumulate before sending it to client
    size_t        accumulatedSize;              // Number of accumulated bytes
    Vector<uint8> accum;                        // Buffer to accumulate entire data that have been sent by other side

    uint8  inbuf[BasicProtoDecoder::MAX_PACKET_SIZE];   // Read buffer
    size_t totalRead;                                   // Number of bytes in read buffer

    Spinlock         sendInProgress;    // Transition 
    Package          curPackage;        // Current package to send
    Deque<Package>   sendQueue;
    Mutex            queueMutex;
    BasicProtoHeader header;            // Header which is filled for each sending packet
};

//////////////////////////////////////////////////////////////////////////
inline bool TCPTransport::LockSender()
{
    return sendInProgress.TryLock();
}

inline void TCPTransport::UnlockSender()
{
    sendInProgress.Unlock();
}

}   // namespace DAVA

#endif  // __DAVAENGINE_TCPTRANSPORT_H__
