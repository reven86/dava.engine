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

#ifndef __DAVAENGINE_DEADLINETIMER_H__
#define __DAVAENGINE_DEADLINETIMER_H__

#include <libuv/uv.h>

#include <Base/BaseTypes.h>
#include <Base/Function.h>
#include <Debug/DVAssert.h>

#include "DeadlineTimerTemplate.h"

namespace DAVA
{

class IOLoop;

/*
 Class DeadlineTimer - fully functional waitable timer implementation which can be used in most cases.
 User can provide functional object which is called on timer expiration.
 Functional objects prototypes:
    WaitHandlerType - called on timer expiration
        void f(DeadlineTimer* timer)
*/
class DeadlineTimer : public DeadlineTimerTemplate<DeadlineTimer, false>
{
private:
    typedef DeadlineTimerTemplate<DeadlineTimer, false> BaseClassType;

public:
    typedef Function<void(DeadlineTimer* timer)> WaitHandlerType;

public:
    explicit DeadlineTimer(IOLoop* loop);
    ~DeadlineTimer() {}

    template <typename Handler>
    int32 AsyncStartWait(uint32 timeout, uint32 repeat, Handler handler);

    void HandleTimer();

private:
    WaitHandlerType waitHandler;
};

//////////////////////////////////////////////////////////////////////////
template <typename Handler>
int32 DeadlineTimer::AsyncStartWait(uint32 timeout, uint32 repeat, Handler handler)
{
    waitHandler = handler;
    return BaseClassType::InternalAsyncStartWait(timeout, repeat);
}

}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMER_H__
