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

#include "TCPTransport.h"

namespace DAVA
{
namespace Net
{

TCPTransport::TCPTransport(IOLoop* ioLoop, ITransportListener* aListener, eTransportRole aRole, const Endpoint& endp)
                                                : role(aRole)
                                                , loop(ioLoop)
                                                , acceptor(ioLoop)
                                                , socket(ioLoop)
                                                , timer(ioLoop)
                                                , endpoint(endp)
                                                , readTimeout(1000 * 60)
                                                , listener(aListener)
                                                , isActive(false)
                                                , deactivateFlag(false)
                                                , pendingPong(false)
                                                , totalDataSize(0)
                                                , accumulatedSize(0)
                                                , totalRead(0)
                                                , senderLock()
                                                , nextPackageId(0)
                                                , sendingDataPacket(false)
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

void TCPTransport::Send(uint32 channelId, const uint8* buffer, size_t length, uint32* packetId)
{
    DVASSERT(true == isActive && buffer != NULL && length > 0);
    if (isActive)
    {
        uint32 packageId = AtomicIncrement(reinterpret_cast<int32&>(nextPackageId));
        if (packetId != NULL)
            *packetId = packageId;

        if (true == senderLock.TryLock())
        {
            // We can send buffer directly without queueing
            PreparePackage(&curPackage, channelId, packageId, buffer, length);

            loop->Post(MakeFunction(this, &TCPTransport::DoSend));
        }
        else
            Enqueue(channelId, packageId, buffer, length);
    }
}

void TCPTransport::DoActivate()
{
    SERVER_ROLE == role ? StartAsServer()
                        : StartAsClient();
}

void TCPTransport::DoDeactivate()
{
    timer.Close();
    SERVER_ROLE == role ? acceptor.Close(MakeFunction(this, &TCPTransport::AcceptorHandleClose))
                        : CloseSocket(isActive);    // We should shutdown and close socket first to cancel any pending operations
}

void TCPTransport::DoSend()
{
    SendPackage(&curPackage);
}

void TCPTransport::StartAsServer()
{
    int32 error = acceptor.Bind(endpoint);
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
    ClearQueue();
    listener->OnTransportDeactivated(this, reason, error);

    totalRead = 0;
    totalDataSize = 0;
    pendingPong = false;
    senderLock.Unlock();

    if (!deactivateFlag)    // Do not close socket twice
    {
        timer.Close();
        CloseSocket(false); // We enter CleanUp on some kind of error and should not shutdown
    }
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
    specQueue.clear();
    pendingAckQueue.clear();
}

void TCPTransport::CloseSocket(bool shouldShutdown)
{
    // Shutdown socket if flag is set
    // Close socket if flag is not set or shutdown has failed
    if (false == shouldShutdown || socket.Shutdown(MakeFunction(this, &TCPTransport::SocketHandleShutdown)) != 0)
        socket.Close(MakeFunction(this, &TCPTransport::SocketHandleClose));
}

void TCPTransport::PreparePackage(Package* package, uint32 channelId, uint32 packageId, const uint8* buffer, size_t length)
{
    package->channelId   = channelId;
    package->buffer      = buffer;
    package->totalLength = length;
    package->sentLength  = 0;
    package->partLength  = 0;
    package->packageId   = packageId;
}

bool TCPTransport::Enqueue(uint32 channelId, uint32 packageId, const uint8* buffer, size_t length)
{
    Package package;
    PreparePackage(&package, channelId, packageId, buffer, length);

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

    sendingDataPacket = true;
    package->partLength = BasicProtoDecoder::Encode(&header, package->channelId, package->packageId, package->totalLength, package->sentLength);

    Buffer buffers[2];
    buffers[0] = CreateBuffer(&header);
    buffers[1] = CreateBuffer(package->buffer + package->sentLength, package->partLength);
    DVVERIFY(0 == socket.Write(buffers, 2, MakeFunction(this, &TCPTransport::SocketHandleWrite)));
}

void TCPTransport::SendSpecial(BasicProtoHeader* header)
{
    if (true == senderLock.TryLock())
    {
        // We can send buffer directly without queueing
        curSpec = *header;

        sendingDataPacket = false;
        Buffer buffer = CreateBuffer(&curSpec);
        DVVERIFY(0 == socket.Write(&buffer, 1, MakeFunction(this, &TCPTransport::SocketHandleWrite)));
    }
    else
    {
        // We don't need locking as special packets are always sent from read or write handlers
        specQueue.push_back(*header);
    }
}

bool TCPTransport::DequeueSpecial(BasicProtoHeader* target)
{
    // We don't need locking as special packets are always dequeued from read handler
    if (!specQueue.empty())
    {
        *target = specQueue.front();
        specQueue.pop_front();
        return true;
    }
    return false;
}

void TCPTransport::HandleTimer(DeadlineTimer* timer)
{
    // We are here if nothing has been read for some period of time
    // First we send PING and wait again
    // If we get here for the second time we consider other side is hung and disconnect
    if (false == pendingPong)
    {
        BasicProtoHeader header;
        BasicProtoDecoder::EncodePing(&header);
        SendSpecial(&header);

        pendingPong = true;
        timer->Wait(readTimeout, MakeFunction(this, &TCPTransport::HandleTimer));
    }
    else
    {
        CleanUp(TIMEOUT, 0);
    }
}

void TCPTransport::AcceptorHandleClose(TCPAcceptor* acceptor)
{
    if (deactivateFlag)
        CloseSocket(isActive);
}

void TCPTransport::AcceptorHandleConnect(TCPAcceptor* acceptor, int32 error)
{
    if (0 == error)
    {
        error = acceptor->Accept(&socket);
        if (0 == error)
        {
            socket.RemoteEndpoint(remoteEndpoint);

            isActive = true;
            listener->OnTransportActivated(this, remoteEndpoint);
            DVVERIFY(0 == socket.StartRead(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &TCPTransport::SocketHandleRead)));
            DVVERIFY(0 == timer.Wait(readTimeout, MakeFunction(this, &TCPTransport::HandleTimer)));

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
        isActive = false;
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
        socket->RemoteEndpoint(remoteEndpoint);

        isActive = true;
        listener->OnTransportActivated(this, remoteEndpoint);

        DVVERIFY(0 == socket->StartRead(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &TCPTransport::SocketHandleRead)));
        DVVERIFY(0 == timer.Wait(readTimeout, MakeFunction(this, &TCPTransport::HandleTimer)));
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
        pendingPong = false;

        totalRead += nread;
        BasicProtoDecoder::DecodeResult result;
        BasicProtoDecoder::eStatus status = BasicProtoDecoder::PACKET_OK;

        do {
            status = BasicProtoDecoder::Decode(inbuf, totalRead, &result);
            switch(status)
            {
            case BasicProtoDecoder::PACKET_OK:
                if (BasicProtoDecoder::TYPE_DATA == result.header->packetType)
                {
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
                        BasicProtoHeader header;
                        BasicProtoDecoder::EncodeAck(&header, result.header->channelId, result.header->packetId);
                        SendSpecial(&header);   // Reply confirmation about packet is received

                        listener->OnTransportReceive(this, result.header->channelId, &*accum.begin(), totalDataSize);
                        totalDataSize = 0;
                    }
                }
                else if (BasicProtoDecoder::TYPE_PING == result.header->packetType)
                {
                    BasicProtoHeader header;
                    BasicProtoDecoder::EncodePong(&header);
                    SendSpecial(&header);
                }
                else if (BasicProtoDecoder::TYPE_PONG == result.header->packetType)
                {
                    // Do nothing as timeout timer is restarted on each receive
                }
                else if (BasicProtoDecoder::TYPE_ACK == result.header->packetType)
                {
                    DVASSERT(false == pendingAckQueue.empty());
                    if (false == pendingAckQueue.empty())
                    {
                        uint32 pendingId = pendingAckQueue.front();
                        pendingAckQueue.pop_front();
                        DVASSERT(pendingId == result.header->packetId);
                        // Confirmations must arrive in order
                        if (pendingId == result.header->packetId)
                            listener->OnTransportPacketDelivered(this, result.header->channelId, result.header->packetId);
                        else
                            CleanUp(PACKETERROR, 0);
                    }
                }
                break;
            case BasicProtoDecoder::PACKET_INCOMPLETE:
                // Wait until full packet gathered
                break;
            case BasicProtoDecoder::PACKET_INVALID:
                CleanUp(PACKETERROR, 0);
                break;
            }

            if (result.decodedSize > 0)
            {
                DVASSERT(totalRead >= result.decodedSize);
                totalRead -= result.decodedSize;
                Memmove(inbuf, inbuf + result.decodedSize, totalRead);
            }

            socket->ReadHere(CreateBuffer(inbuf + totalRead, sizeof(inbuf) - totalRead));
        } while (BasicProtoDecoder::PACKET_OK == status && totalRead > 0);
        timer.Wait(readTimeout, MakeFunction(this, &TCPTransport::HandleTimer));
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
        if (sendingDataPacket)
        {
            curPackage.sentLength += curPackage.partLength;
            if (curPackage.sentLength == curPackage.totalLength)
            {
                pendingAckQueue.push_back(curPackage.packageId);
                listener->OnTransportSendComplete(this, curPackage.channelId, curPackage.buffer, curPackage.totalLength);
                curPackage.buffer = NULL;   // Mark buffer sent
            }
        }

        if (DequeueSpecial(&curSpec))   // First send special packet
        {
            sendingDataPacket = false;
            Buffer buffer = CreateBuffer(&curSpec);
            DVVERIFY(0 == socket->Write(&buffer, 1, MakeFunction(this, &TCPTransport::SocketHandleWrite)));
        }
        else if (curPackage.buffer != NULL || Dequeue(&curPackage))
        {
            SendPackage(&curPackage);
        }
        else
        {
            // Nothing to send
            senderLock.Unlock();
        }
    }
    else
    {
        CleanUp(NETERROR, error);
    }
}

}   // namespace Net
}   // namespace DAVA
