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


#ifndef __DAVAENGINE_TCP_CHANNEL_H__
#define __DAVAENGINE_TCP_CHANNEL_H__

#include "Base/BaseTypes.h"
#include "Network/NetService.h"
#include "Base/ScopedPtr.h"

namespace DAVA {

class KeyedArchive;
class TCPChannel;
class DynamicMemoryFile;

class TCPChannelListener
{
public:
    
    virtual void ChannelOpened(TCPChannel *tcpChannel) {};
    virtual void ChannelClosed(TCPChannel *tcpChannel, const char8* message) {};
    virtual void PacketReceived(TCPChannel *tcpChannel, const uint8* packet, size_t length) = 0;
    virtual void PacketSent(TCPChannel *tcpChannel) {};
    virtual void PacketDelivered(TCPChannel *tcpChannel) {};
};

class TCPChannel: public Net::NetService
{
public:

    TCPChannel();
    ~TCPChannel() override;

    
    bool SendData(DynamicMemoryFile* buffer);
    bool SendData(const Vector<uint8>& data);
    bool SendData(const uint8 * data, const size_t dataSize);
    void SetListener(TCPChannelListener * delegate);
    
    //IChannelListener
    void OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length) override;

    //Net::NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketSent() override;
    void PacketDelivered() override;
    
    bool IsConnected() const;
    
    bool SendArchieve(KeyedArchive * archieve);

protected:
    
    TCPChannelListener *listener = nullptr;
    Map<const void*,DynamicMemoryFile*> buffers;
};


inline void TCPChannel::SetListener(TCPChannelListener * _listener)
{
    listener = _listener;
}

inline bool TCPChannel::IsConnected() const
{
    return IsChannelOpen();
}
    
};

#endif // __DAVAENGINE_TCP_CHANNEL_H__

