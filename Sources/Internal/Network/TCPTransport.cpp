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

#include <Base/FunctionTraits.h>
#include <Debug/DVAssert.h>
#include <Thread/LockGuard.h>

#include <Network/Base/NetworkUtils.h>

#include "TCPTransport.h"

namespace DAVA
{

TCPTransport::TCPTransport(IOLoop* ioLoop, ITransportListener* aListener, eTransportRole aRole, const Endpoint& endp)
                                                : role(aRole)
                                                , loop(ioLoop)
                                                , acceptor(ioLoop)
                                                , socket(ioLoop)
                                                , endpoint(endp)
                                                , listener(aListener)
                                                , isActive(false)
                                                , deactivateFlag(false)
                                                , totalDataSize(0)
                                                , accumulatedSize(0)
                                                , totalRead(0)
                                                , senderLock()
                                                , curPackage()
                                                , header()
{
    DVASSERT(aListener != NULL && (SERVER_ROLE == aRole || CLIENT_ROLE == aRole));
    Memset(inbuf, 0, sizeof(inbuf));
}

TCPTransport::~TCPTransport()
{
    DVASSERT(false == isActive && "Destroying active transport");
}

bool TCPTransport::IsActive() const
{
    return isActive;
}

void TCPTransport::Activate()
{
    DVASSERT(false == isActive && "Activating transport more than once");
    if (!isActive)
    {
        loop->Post(MakeFunction(this, &TCPTransport::DoActivate));
    }
}

void TCPTransport::Deactivate()
{
    if (!deactivateFlag)
    {
        deactivateFlag = true;
        loop->Post(MakeFunction(this, &TCPTransport::DoDeactivate));
    }
}

void TCPTransport::Send(uint32 channelId, const uint8* buffer, size_t length)
{
    DVASSERT(true == isActive && buffer != NULL && length > 0);
    if (isActive)
    {
        if (true == senderLock.TryLock())
        {
            // We can send buffer directly without queueing
            PreparePackage(&curPackage, channelId, buffer, length);

            loop->Post(MakeFunction(this, &TCPTransport::DoSend));
        }
        else 
            Enqueue(channelId, buffer, length);
    }
}

void TCPTransport::DoActivate()
{
    SERVER_ROLE == role ? StartAsServer()
                        : StartAsClient();
}

void TCPTransport::DoDeactivate()
{
    SERVER_ROLE == role ? acceptor.Close(MakeFunction(this, &TCPTransport::AcceptorHandleClose))
                        : CloseSocket();
}

void TCPTransport::DoSend()
{
    SendPackage(&curPackage);
}

void TCPTransport::StartAsServer()
{
    int32 error = acceptor.Bind(endpoint.Port());
    if (0 == error)
    {
        error = acceptor.StartListen(MakeFunction(this, &TCPTransport::AcceptorHandleConnect), 1);
        if (0 == error)
            return;
    }
    CleanUp(INITERROR, error);
}

void TCPTransport::StartAsClient()
{
    int32 error = socket.Connect(endpoint, MakeFunction(this, &TCPTransport::SocketHandleConnect));
    if (error != 0)
    {
        CleanUp(INITERROR, error);
    }
}

void TCPTransport::CleanUp(eDeactivationReason reason, int32 error)
{
    isActive = false;
    listener->OnTransportDeactivated(this, reason, error);
    ClearQueue();

    totalRead = 0;
    totalDataSize = 0;
    senderLock.Unlock();

    if (!deactivateFlag)
        CloseSocket();
}

void TCPTransport::ClearQueue()
{
    // Don't forget to notify about current package
    if (curPackage.buffer)
    {
        listener->OnTransportSendComplete(this, curPackage.channelId, curPackage.buffer, curPackage.totalLength);
        curPackage.buffer = NULL;
    }

    for (Deque<Package>::iterator i = sendQueue.begin(), e = sendQueue.end();i != e;++i)
    {
        Package& package = *i;
        listener->OnTransportSendComplete(this, package.channelId, package.buffer, package.totalLength);
    }
    sendQueue.clear();
}

void TCPTransport::CloseSocket()
{
    if (socket.Shutdown(MakeFunction(this, &TCPTransport::SocketHandleShutdown)) != 0)
        socket.Close(MakeFunction(this, &TCPTransport::SocketHandleClose));
}

void TCPTransport::PreparePackage(Package* package, uint32 channelId, const uint8* buffer, size_t length)
{
    package->channelId   = channelId;
    package->buffer      = buffer;
    package->totalLength = length;
    package->sentLength  = 0;
    package->partLength  = 0;
}

bool TCPTransport::Enqueue(uint32 channelId, const uint8* buffer, size_t length)
{
    Package package;
    PreparePackage(&package, channelId, buffer, length);

    bool queueWasEmpty = false;

    LockGuard<Mutex> lock(queueMutex);
    queueWasEmpty = sendQueue.empty();
    sendQueue.push_back(package);
    return queueWasEmpty;
}

bool TCPTransport::Dequeue(Package* target)
{
    LockGuard<Mutex> lock(queueMutex);
    if (!sendQueue.empty())
    {
        *target = sendQueue.front();
        sendQueue.pop_front();
        return true;
    }
    return false;
}

void TCPTransport::SendPackage(Package* package)
{
    DVASSERT(package->sentLength < package->totalLength);

    package->partLength = BasicProtoDecoder::Encode(&header, package->channelId, package->totalLength, package->sentLength);

    Buffer buffers[2];
    buffers[0] = CreateBuffer(&header);
    buffers[1] = CreateBuffer(package->buffer + package->sentLength, package->partLength);
    DVVERIFY(0 == socket.Write(buffers, 2, MakeFunction(this, &TCPTransport::SocketHandleWrite)));
}

void TCPTransport::AcceptorHandleClose(TCPAcceptor* acceptor)
{
    if (deactivateFlag)
        CloseSocket();
}

void TCPTransport::AcceptorHandleConnect(TCPAcceptor* acceptor, int32 error)
{
    if (0 == error)
    {
        error = acceptor->Accept(&socket);
        if (0 == error)
        {
            isActive = true;
            listener->OnTransportActivated(this);
            DVVERIFY(0 == socket.StartRead(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &TCPTransport::SocketHandleRead)));

            // Close acceptor to stop receiving incoming connections
            acceptor->Close(MakeFunction(this, &TCPTransport::AcceptorHandleClose));
            return;
        }
    }
    CleanUp(NETERROR, error);
}

void TCPTransport::SocketHandleClose(TCPSocket* socket)
{
    if (deactivateFlag)
    {
        // I ensure that this is the last method called on termination
        listener->OnTransportDeactivated(this, REQUEST, 0);
        listener->OnTransportTerminated(this);
    }
    else
    {
        DoActivate();
    }
}

void TCPTransport::SocketHandleShutdown(TCPSocket* socket, int32 error)
{
    socket->Close(MakeFunction(this, &TCPTransport::SocketHandleClose));
}

void TCPTransport::SocketHandleConnect(TCPSocket* socket, int32 error)
{
    if (0 == error)
    {
        isActive = true;
        listener->OnTransportActivated(this);

        DVVERIFY(0 == socket->StartRead(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &TCPTransport::SocketHandleRead)));
    }
    else
    {
        // TODO: should I notify about connection failure every time
        CleanUp(NETERROR, error);
    }
}

void TCPTransport::SocketHandleRead(TCPSocket* socket, int32 error, size_t nread)
{
    if (0 == error)
    {
        totalRead += nread;
        BasicProtoDecoder::DecodeResult result;
        BasicProtoDecoder::eStatus status = BasicProtoDecoder::PACKET_OK;

        do {
            status = BasicProtoDecoder::Decode(inbuf, totalRead, &result);
            switch(status)
            {
            case BasicProtoDecoder::PACKET_OK:
                if (0 == totalDataSize)     // Prepare for new data block
                {
                    totalDataSize   = result.header->totalSize;
                    accumulatedSize = 0;
                    if (accum.size() < totalDataSize)
                        accum.resize(totalDataSize);
                }

                Memcpy(&*accum.begin() + accumulatedSize, result.packetData, result.packetDataSize);
                accumulatedSize += result.packetDataSize;
                if (accumulatedSize == totalDataSize)
                {
                    listener->OnTransportReceive(this, result.header->channelId, &*accum.begin(), totalDataSize);
                    totalDataSize = 0;
                }

                DVASSERT(totalRead >= result.decodedSize);
                totalRead -= result.decodedSize;
                Memmove(inbuf, inbuf + result.decodedSize, totalRead);
                break;
            case BasicProtoDecoder::PACKET_INCOMPLETE:
                // Wait until full packet gathered
                break;
            case BasicProtoDecoder::PACKET_INVALID:
                CleanUp(PACKETERROR, 0);
                break;
            }
            socket->ReadHere(CreateBuffer(inbuf + totalRead, sizeof(inbuf) - totalRead));
        } while (BasicProtoDecoder::PACKET_OK == status && totalRead > 0);
    }
    else
    {
        socket->IsEOF(error) ? CleanUp(OTHERSIDE, 0)
                             : CleanUp(NETERROR, error);
    }
}

void TCPTransport::SocketHandleWrite(TCPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount)
{
    if (0 == error)
    {
        curPackage.sentLength += curPackage.partLength;
        if (curPackage.sentLength == curPackage.totalLength)
        {
            listener->OnTransportSendComplete(this, curPackage.channelId, curPackage.buffer, curPackage.totalLength);
            curPackage.buffer = NULL;   // Mark buffer sent
            Dequeue(&curPackage) ? SendPackage(&curPackage)
                                 : senderLock.Unlock();
        }
        else
            SendPackage(&curPackage);
    }
    else
    {
        CleanUp(NETERROR, error);
    }
}

}   // namespace DAVA
