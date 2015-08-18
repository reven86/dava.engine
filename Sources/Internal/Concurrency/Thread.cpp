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


#include <thread>
#include "Concurrency/Thread.h"
#include "Concurrency/LockGuard.h"

#ifndef __DAVAENGINE_WINDOWS__
#   include <time.h>
#   include <errno.h>
#endif

namespace DAVA
{

ConcurrentObject<Set<Thread *>> Thread::threadList;
Thread::Id Thread::mainThreadId;

void Thread::InitMainThread()
{
    mainThreadId = GetCurrentId();
}

bool Thread::IsMainThread()
{
    if (Thread::Id() == mainThreadId)
    {
        Logger::Error("Main thread not initialized");
    }

    Id currentId = GetCurrentId();
    return currentId == mainThreadId;
}

Thread *Thread::Create(const Message& msg)
{
    return new Thread(msg);
}

Thread *Thread::Create(const Procedure& proc)
{
    return new Thread(proc);
}

void Thread::Kill()
{
    // it is possible to kill thread just after creating or starting and the problem is - thred changes state
    // to STATE_RUNNING insite threaded function - so that could not happens in that case. Need some time.
    DVASSERT(STATE_CREATED != state);

    // Important - DO NOT try to wait RUNNING state because that state wll not appear if thread is not started!!!
    // You can wait RUNNING state, but not from thred which should call Start() for created Thread.

    if (STATE_RUNNING == state)
    {
        KillNative();
        state = STATE_KILLED;
        Release();
    }
}

void Thread::KillAll()
{
    auto threadListAccessor = threadList.GetAccessor();
    for (auto& x : *threadListAccessor)
    {
        x->Kill();
    }
}

void Thread::CancelAll()
{
    auto threadListAccessor = threadList.GetAccessor();
    for (auto& x : *threadListAccessor)
    {
        x->Cancel();
    }
} 


Thread::Thread()
    : state(STATE_CREATED)
    , threadPriority(PRIORITY_NORMAL)
    , isCancelling(false)
    , stackSize(0)
    , id(Id())
    , handle(Handle())
    , name("DAVA::Thread")
{
    Init();

    auto threadListAccessor = threadList.GetAccessor();
    threadListAccessor->insert(this);
}

Thread::Thread(const Message &msg) : Thread()
{
    Message message = msg;
    Thread* caller = this;
    threadFunc = [=] { message(caller); };
}

Thread::Thread(const Procedure &proc) : Thread()
{
    threadFunc = proc;
}

Thread::~Thread()
{
    Shutdown();

    auto threadListAccessor = threadList.GetAccessor();
    threadListAccessor->erase(this);
}
    
void Thread::ThreadFunction(void *param)
{
    Thread * t = (Thread *)param;
    t->id = GetCurrentId();

    t->threadFunc();
    t->state = STATE_ENDED;

    t->Release();
}

void Thread::Yield()
{
    std::this_thread::yield();
}

void Thread::Sleep(uint32 timeMS)
{
    std::chrono::milliseconds ms(timeMS);
    std::this_thread::sleep_for(ms);
}
    
};
