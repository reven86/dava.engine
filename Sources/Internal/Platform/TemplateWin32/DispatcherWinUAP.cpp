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

#include "Platform/TemplateWin32/DispatcherWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/UniqueLock.h"
#include "Concurrency/LockGuard.h"

#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

namespace DAVA
{

namespace
{

// Wrapper to prepare task for running in DispatcherWinUAP's thread context and waiting task completion
class TaskWrapper final
{
public:
    TaskWrapper(std::function<void()>&& task_)
        : task(std::move(task_))
    {}

    void RunTask()
    {
        task();
        {
            LockGuard<Mutex> lock(mutex);
            taskDone = true;
        }
        cv.NotifyOne();
    }

    void WaitTaskComplete()
    {
        UniqueLock<Mutex> lock(mutex);
        cv.Wait(lock, [this](){ return taskDone; });
    }

private:
    Mutex mutex;
    ConditionVariable cv;
    std::function<void()> task;
    bool taskDone = false;
};

}   // unnamed namespace

DispatcherWinUAP::BlockingTaskWrapper::BlockingTaskWrapper(BlockingTaskWrapper&& other)
    : dispatcher(std::move(other.dispatcher))
    , task(std::move(other.task))
    , taskDone(std::move(taskDone))
{
    other.dispatcher = nullptr;
    other.taskDone = false;
}

void DispatcherWinUAP::BlockingTaskWrapper::RunTask()
{
    task();
    {
        LockGuard<Mutex> lock(dispatcher->mutex);
        taskDone = true;
    }
    dispatcher->cv.NotifyAll();
}

void DispatcherWinUAP::BlockingTaskWrapper::WaitTaskComplete()
{
    bool foreignThread = dispatcher->boundThreadId != Thread::GetCurrentId();

    UniqueLock<Mutex> lock(dispatcher->mutex);
    while (!taskDone)
    {
        if (!foreignThread && !dispatcher->taskQueue.empty())
        {
            // While waiting task completion process other scheduled tasks to avoid deadlocks
            // Deadlock can occur in the following circumstances:
            //  from main (dispatcher's) thread start blocking call to UI thread
            //  from UI thread start blocking call to main thread (e.g. ask delegate of some action that should be taken)
            lock.Unlock();
            dispatcher->ProcessTasks();
            lock.Lock();
        }
        // Additional check whether blocking task has completed while dispatcher was processing its own tasks
        // This check is necessary because of lock releasing:
        //      RunTask() method can finish blocking task and signal conditional variable before
        //      this method takes chance to wait on that conditional variable
        if (!taskDone)
        {
            dispatcher->cv.Wait(lock);
        }
    }
}

DispatcherWinUAP::DispatcherWinUAP()
    : boundThreadId(Thread::GetCurrentId())
{}

void DispatcherWinUAP::BindToCurrentThread()
{
    boundThreadId = Thread::GetCurrentId();
}

bool DispatcherWinUAP::InBlockingCall() const
{
    if (blockingCall.test_and_set() == false)
    {
        blockingCall.clear();
        return false;
    }
    return true;
}

void DispatcherWinUAP::ProcessTasks()
{
    DVASSERT(boundThreadId == Thread::GetCurrentId());

    std::list<std::function<void()>> tasksToExecute;
    {
        LockGuard<Mutex> guard(mutex);
        tasksToExecute.swap(taskQueue);
    }

    for (std::function<void()>& task : tasksToExecute)
    {
        task();
    }
}

void DispatcherWinUAP::ScheduleTask(std::function<void()>&& task)
{
    {
        LockGuard<Mutex> guard(mutex);
        taskQueue.emplace_back(std::move(task));
    }
    cv.NotifyAll();
}

void DispatcherWinUAP::ScheduleTaskAndWait(std::function<void()>&& task)
{
    // TODO: maybe call task as subroutine without scheduling
    DVASSERT(boundThreadId != Thread::GetCurrentId());
    DVASSERT(InBlockingCall() == false);

    blockingCall.test_and_set();
    TaskWrapper taskWrapper(std::move(task));
    ScheduleTask(std::function<void()>([&taskWrapper]() { taskWrapper.RunTask(); }));
    taskWrapper.WaitTaskComplete();
    blockingCall.clear();
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
