/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#ifndef __DAVAENGINE_ASYNCTEMPLATE_H__
#define __DAVAENGINE_ASYNCTEMPLATE_H__

#include <Base/Function.h>

#include "HandleBase.h"

namespace DAVA
{

class IOLoop;

/*
 Class AsyncRequest allows to wake up the event loop and run specified callback. Object of this type can be used from
 different threads (all other objects can be used only in IOLoop thread)
 Use case:
    We have two threads:
        thread A - main application thread
        thread B - thread where event loop is running
    Application has connected socket and wants to send text message on pressing key.
    When key is pressed application queues message and wakes event loop, then in async handler (which executes in
    IOLoop's thread context) calls socket.AsyncWrite

 User can provide functional object to be called as async handler:
    void(AsyncRequest* async)

 Libuv can combine multiple invocations of WakeLoop and call handler only once.
*/
class AsyncRequest : public HandleBase<uv_async_t>
{
private:
    typedef HandleBase<uv_async_t> BaseClassType;

public:
    typedef Function<void(AsyncRequest* async)> AsyncHandlerType;

public:
    AsyncRequest(IOLoop* loop, AsyncHandlerType handler = AsyncHandlerType());
    ~AsyncRequest() {}

    void SetAsyncHandler(AsyncHandlerType handler);

    void Close();

    int32 WakeLoop();

private:
    void HandleAsync();

    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleAsyncThunk(uv_async_t* handle);

private:
    AsyncHandlerType asyncHandler;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_ASYNCTEMPLATE_H__
