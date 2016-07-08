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
class DispatcherWinUAP final
{
public:
    // Wrapper to prepare task for running in UI thread context and waiting task completion
    // While waiting task completion wrapper also processes tasks scheduled to DispatcherWinUAP
    //
    // Example:
    //      get wrapper for task
    //  DispatcherWinUAP::BlockingTaskWrapper wrapper = dispatcher->GetBlockingTaskWrapper([](){});
    //
    //      run wrapped task on UI thread
    //  core->RunOnUIThread([&wrapper](){wrapper.RunTask();});      !!! non blocking call
    //
    //      wait task completion and process scheduled tasks
    //  wrapper.WaitTaskComplete();
    class BlockingTaskWrapper final
    {
        friend class DispatcherWinUAP;

    private:
        template <typename T>
        BlockingTaskWrapper(DispatcherWinUAP* disp, T&& task_);

    public:
        // Permit only public move ctor to allow creating BlockingTaskWrapper
        // on stack through DispatcherWinUAP::GetBlockingTaskWrapper
        BlockingTaskWrapper(BlockingTaskWrapper&& other);
        ~BlockingTaskWrapper() = default;

        // No need to explicitly delete copy ctor and assignment as these special
        // members are not created if move ctor has been defined

        void RunTask();
        void WaitTaskComplete();

    private:
        DispatcherWinUAP* dispatcher = nullptr;
        std::function<void()> task;
        bool taskDone = false;
    };

public:
    DispatcherWinUAP();
    ~DispatcherWinUAP() = default;

    void BindToCurrentThread();
    Thread::Id BoundThreadId() const;

    // Process all currently scheduled tasks
    void ProcessTasks();

    // Schedule task to run on dispatcher's thread and return immediatly
    template <typename T>
    void RunAsync(T&& task);

    // Schedule task to run on dispatcher's thread and block calling thread waiting for task completion
    template <typename T>
    void RunAsyncAndWait(T&& task);

    template <typename T>
    BlockingTaskWrapper GetBlockingTaskWrapper(T&& task);

    bool InBlockingCall() const;

private:
    void ScheduleTask(std::function<void()>&& task, bool scheduleFirst);
    void ScheduleTaskAndWait(std::function<void()>&& task);

private:
    Mutex mutex;
    ConditionVariable cv;
    std::list<std::function<void()>> taskQueue;
    Thread::Id boundThreadId{};
    mutable std::atomic_flag blockingCall = ATOMIC_FLAG_INIT;
};

//////////////////////////////////////////////////////////////////////////
inline Thread::Id DispatcherWinUAP::BoundThreadId() const
{
    return boundThreadId;
}

template <typename T>
void DispatcherWinUAP::RunAsync(T&& task)
{
    ScheduleTask(std::function<void()>(std::forward<T>(task)), false);
}

template <typename T>
void DispatcherWinUAP::RunAsyncAndWait(T&& task)
{
    ScheduleTaskAndWait(std::function<void()>(std::forward<T>(task)));
}

template <typename T>
DispatcherWinUAP::BlockingTaskWrapper DispatcherWinUAP::GetBlockingTaskWrapper(T&& task)
{
    return BlockingTaskWrapper(this, std::forward<T>(task));
}

//////////////////////////////////////////////////////////////////////////
template <typename T>
DispatcherWinUAP::BlockingTaskWrapper::BlockingTaskWrapper(DispatcherWinUAP* disp, T&& task_)
    : dispatcher(disp)
    , task(std::move(task_))
{
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_DISPATCHER_H__
