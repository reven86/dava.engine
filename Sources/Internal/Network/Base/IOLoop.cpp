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

#include <Base/FunctionTraits.h>
#include <Thread/LockGuard.h>
#include <Debug/DVAssert.h>

#include <Network/Base/IOLoop.h>

namespace DAVA
{
namespace Net
{

IOLoop::IOLoop(bool useDefaultIOLoop) : uvloop()
                                      , actualLoop(NULL)
                                      , quitFlag(false)
                                      , uvasync()
{
    if (useDefaultIOLoop)
    {
        actualLoop = uv_default_loop();
    }
    else
    {
        DVVERIFY(0 == uv_loop_init(&uvloop));
        actualLoop = &uvloop;
    }
    actualLoop->data = this;

    DVVERIFY(0 == uv_async_init(actualLoop, &uvasync, &HandleAsyncThunk));
    uvasync.data = this;
}

IOLoop::~IOLoop()
{
    // We can close default loop too
    DVVERIFY(0 == uv_loop_close(actualLoop));
}

int32 IOLoop::Run(eRunMode runMode)
{
    static const uv_run_mode modes[] = {
        UV_RUN_DEFAULT,
        UV_RUN_ONCE,
        UV_RUN_NOWAIT
    };
    DVASSERT(RUN_DEFAULT == runMode || RUN_ONCE == runMode || RUN_NOWAIT == runMode);
    return uv_run(actualLoop, modes[runMode]);
}

void IOLoop::Post(UserHandlerType handler)
{
    {
        LockGuard<Mutex> lock(mutex);
        // TODO: maybe do not insert duplicates
        queuedHandlers.push_back(handler);
    }
    uv_async_send(&uvasync);
}

void IOLoop::PostQuit()
{
    if (false == quitFlag)
    {
        quitFlag = true;
        uv_async_send(&uvasync);
    }
}

void IOLoop::HandleAsync()
{
    {
        // Steal queued handlers for execution and release mutex
        // Main reason to do so is to avoid deadlocks if executed
        // handler will want to post another handler
        LockGuard<Mutex> lock(mutex);
        execHandlers.swap(queuedHandlers);
    }

    for (Vector<UserHandlerType>::const_iterator i = execHandlers.begin(), e = execHandlers.end();i != e;++i)
    {
        (*i)();
    }
    execHandlers.clear();

    if (true == quitFlag)
    {
        uv_close(reinterpret_cast<uv_handle_t*>(&uvasync), NULL);
    }
}

void IOLoop::HandleAsyncThunk(uv_async_t* handle)
{
    IOLoop* self = static_cast<IOLoop*>(handle->data);
    self->HandleAsync();
}

}   // namespace Net
}   // namespace DAVA
