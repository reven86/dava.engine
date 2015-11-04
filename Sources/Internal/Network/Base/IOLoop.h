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


#ifndef __DAVAENGINE_IOLOOP_H__
#define __DAVAENGINE_IOLOOP_H__

#include "Base/BaseTypes.h"
#include <libuv/uv.h>

#include "Functional/Function.h"
#include "Base/Noncopyable.h"
#include "Concurrency/Mutex.h"

namespace DAVA
{
namespace Net
{

/*
 Class IOLoop provides event loop which polls for events and schedules handlers (callback) to be run.
 There should be at least one IOLoop instance in network application.
 Run is a core method of IOLoop, and must be called from at most one thread.
 Post is a special method to shedule user specified handler to be run in context of thread where Run
 method is running.
 To finish running IOLoop you should finish all network operations and call PostQuit method
*/
class IOLoop : private Noncopyable
{
public:
    // Behaviours of Run method
    enum eRunMode
    {
        RUN_DEFAULT = UV_RUN_DEFAULT,   // Wait for events and execute handlers so long as there are active objects
        RUN_ONCE    = UV_RUN_ONCE,      // Wait for events, execute handlers and exit
        RUN_NOWAIT  = UV_RUN_NOWAIT     // Execute handlers if they are ready and immediatly exit
    };

    using UserHandlerType = Function<void()>;

public:
    IOLoop(bool useDefaultIOLoop = true);
    ~IOLoop();

    uv_loop_t* Handle() const { return actualLoop; }

    int32 Run(eRunMode runMode = RUN_DEFAULT);

    void Post(UserHandlerType handler);
    void PostQuit();

private:
    void HandleAsync();

    static void HandleAsyncThunk(uv_async_t* handle);

private:
    uv_loop_t uvloop;                   // libuv loop handle itself
    uv_loop_t* actualLoop;

    bool quitFlag;
    uv_async_t uvasync;                     // libuv handle for calling callback from different threads
    Vector<UserHandlerType> queuedHandlers; // List of queued user handlers
    Vector<UserHandlerType> execHandlers;   // List of executing user handlers
    Mutex mutex;
};

}   // namespace Net
}	// namespace DAVA

#endif  // __DAVAENGINE_IOLOOP_H__
