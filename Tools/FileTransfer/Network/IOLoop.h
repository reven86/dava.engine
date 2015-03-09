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

#include <libuv/uv.h>

#include <Base/BaseTypes.h>
#include <Base/Noncopyable.h>

namespace DAVA
{

/*
 Class IOLoop is a thin wrapper on libuv's uv_loop_t object.
 IOLoop's responsibility is to gather event from operating system and call corresponding user callbacks.
 Run is a core method of IOLoop, must be called from at most one thread.
*/
class IOLoop : private Noncopyable
{
public:
    enum eRunMode
    {
        RUN_DEFAULT = UV_RUN_DEFAULT,
        RUN_ONCE    = UV_RUN_ONCE,
        RUN_NOWAIT  = UV_RUN_NOWAIT
    };

public:
    IOLoop(bool useDefaultIOLoop = true);

    ~IOLoop();

    uv_loop_t* Handle();

    int32 Run(eRunMode runMode = RUN_DEFAULT);
    void Stop();

	bool IsAlive() const;

private:
    uv_loop_t  loop;
    uv_loop_t* actualLoop;
    bool       useDefaultLoop;
};

//////////////////////////////////////////////////////////////////////////
inline uv_loop_t* IOLoop::Handle()
{
    return actualLoop;
}

}	// namespace DAVA

#endif  // __DAVAENGINE_IOLOOP_H__
