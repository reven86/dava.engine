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


#ifndef __DAVAENGINE_LOGCONSUMER_H__
#define __DAVAENGINE_LOGCONSUMER_H__

#include "Base/Optional.h"
#include "Functional/Signal.h"

#include "Network/NetService.h"

namespace DAVA
{

class File;

namespace Net
{

/*
 This is a simple log consumer: each log message is treated as string
*/
class LogConsumer : public NetService
{
public:
    LogConsumer();
    ~LogConsumer() override;

    LogConsumer(const LogConsumer&) = delete;
    Noncopyable& operator = (const Noncopyable&) = delete;

    void OnPacketReceived(IChannel* channel, const void* buffer, size_t length) override;

    SignalConnection SubscribeOnReceivedData(const Function<void(const String&)>& func);

private:
    Signal<const String&> newDataNotifier;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_LOGCONSUMER_H__
