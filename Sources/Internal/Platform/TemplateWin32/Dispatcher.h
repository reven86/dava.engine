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

#ifndef __DAVAENGINE_DISPATCHER_H__
#define __DAVAENGINE_DISPATCHER_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Base/BaseTypes.h"
#include "Concurrency/Atomic.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/ConditionVariable.h"
#include "Concurrency/Thread.h"

#include <functional>

namespace DAVA
{

class Dispatcher final
{
public:
    // Wrapper to prepare task for running in UI thread context and waiting task completion
    // While waiting task completion wrapper also processes tasks scheduled to Dispatcher
    //
    // Example:
    //      get wrapper for task
    //  Dispatcher::BlockingTaskWrapper wrapper = dispatcher->GetBlockingTaskWrapper([](){});
    //
    //      run wrapped task on UI thread
    //  core->RunOnUIThread([&wrapper](){wrapper.RunTask();});      !!! non blocking call
    //
    //      wait task completion and process scheduled tasks
    //  wrapper.WaitTaskComplete();
    class BlockingTaskWrapper final
    {
        friend class Dispatcher;

    private:
        template<typename T>
        BlockingTaskWrapper(Dispatcher* disp, T&& task_);

    public:
        // Permit only public move ctor to allow creating BlockingTaskWrapper
        // on stack through Dispatcher::GetBlockingTaskWrapper
        BlockingTaskWrapper(BlockingTaskWrapper&& other);
        ~BlockingTaskWrapper() = default;

        // No need to explicitly delete copy ctor and assignment as these special
        // members are not created if move ctor has been defined

        void RunTask();
        void WaitTaskComplete();

    private:
        Dispatcher* dispatcher = nullptr;
        std::function<void()> task;
        bool taskDone = false;
    };

public:
    Dispatcher();
    ~Dispatcher() = default;

    void BindToCurrentThread();

    // Process all currently scheduled tasks
    void ProcessTasks();

    // Schedule task to run on dispatcher's thread and return immediatly
    template<typename T>
    void Post(T&& task);

    // Schedule task to run on dispatcher's thread and block calling thread waiting for task completion
    template<typename T>
    void Send(T&& task);

    template<typename T>
    BlockingTaskWrapper GetBlockingTaskWrapper(T&& task);

    bool InBlockingCall() const;

private:
    void ScheduleTask(std::function<void()>&& task);
    void ScheduleTaskAndWait(std::function<void()>&& task);

private:
    Mutex mutex;
    ConditionVariable cv;
    std::list<std::function<void()>> taskQueue;
    Thread::Id boundThreadId{};
    mutable std::atomic_flag blockingCall = ATOMIC_FLAG_INIT;
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
void Dispatcher::Post(T&& task)
{
    ScheduleTask(std::function<void()>(std::forward<T>(task)));
}

template<typename T>
void Dispatcher::Send(T&& task)
{
    ScheduleTaskAndWait(std::function<void()>(std::forward<T>(task)));
}

template<typename T>
Dispatcher::BlockingTaskWrapper Dispatcher::GetBlockingTaskWrapper(T&& task)
{
    return BlockingTaskWrapper(this, std::forward<T>(task));
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
Dispatcher::BlockingTaskWrapper::BlockingTaskWrapper(Dispatcher* disp, T&& task_)
    : dispatcher(disp)
    , task(std::move(task_))
{}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_DISPATCHER_H__
