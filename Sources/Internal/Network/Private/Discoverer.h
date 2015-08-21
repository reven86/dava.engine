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


#ifndef __DAVAENGINE_DISCOVERER_H__
#define __DAVAENGINE_DISCOVERER_H__

#include <Network/Base/UDPSocket.h>
#include <Network/Base/DeadlineTimer.h>

#include <Network/IController.h>

namespace DAVA
{
namespace Net
{

class IOLoop;

class Discoverer : public IController
{
    static const uint32 RESTART_DELAY_PERIOD = 3000;

public:
    Discoverer(IOLoop* ioLoop, const Endpoint& endp, Function<void (size_t, const void*, const Endpoint&)> dataReadyCallback);
    virtual ~Discoverer();

    // IController
    virtual void Start();
    virtual void Stop(Function<void (IController*)> callback);
    virtual void Restart();

private:
    void DoStart();
    void DoStop();
    void DoObjectClose();
    void DoBye();

    void TimerHandleClose(DeadlineTimer* timer);
    void TimerHandleDelay(DeadlineTimer* timer);

    void SocketHandleClose(UDPSocket* socket);
    void SocketHandleReceive(UDPSocket* socket, int32 error, size_t nread, const Endpoint& endpoint, bool partial);

private:
    IOLoop* loop;
    UDPSocket socket;
    DeadlineTimer timer;
    Endpoint endpoint;
    Array<char8, 30> endpAsString;
    bool isTerminating;
    size_t runningObjects;
    Function<void (IController*)> stopCallback;
    Function<void (size_t, const void*, const Endpoint&)> dataCallback;
    uint8 inbuf[64 * 1024];
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_DISCOVERER_H__
