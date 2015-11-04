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

#include <Functional/Function.h>

#include <Network/Base/DeadlineTimerTemplate.h>

namespace DAVA
{
namespace Net
{

/*
 Class DeadlineTimer provides a waitable timer.
 DeadlineTimer allows to wait for specified amount of time.
 All operations are executed asynchronously and all these operations must be started in thread context
 where IOLoop is running, i.e. they can be started during handler processing or using IOLoop Post method.
 List of methods starting async operations:
    Wait
    Close

 DeadlineTimer notifies user about wait or close  operation completion through user-supplied functional objects (handlers or callbacks).
 Handlers are called with one parameter: pointer to DeadlineTimer instance.

 Note: handlers should not block, this will cause all network system to freeze.

 To start working with DeadlineTimer simply call Wait with desired amount of time in ms.

 Close also is executed asynchronously and in its handler it is allowed to destroy DeadlineTimer object.
*/
class DeadlineTimer : public DeadlineTimerTemplate<DeadlineTimer>
{
    friend DeadlineTimerTemplate<DeadlineTimer>;    // Make base class friend to allow it to call my Handle... methods

public:
    using CloseHandlerType = Function<void(DeadlineTimer* timer)>;
    using WaitHandlerType = Function<void(DeadlineTimer* timer)>;

public:
    DeadlineTimer(IOLoop* loop);

    int32 Wait(uint32 timeout, WaitHandlerType handler);
    void Close(CloseHandlerType handler = CloseHandlerType());

private:
    void HandleClose();
    void HandleTimer();

private:
    CloseHandlerType closeHandler;
    WaitHandlerType waitHandler;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMER_H__
