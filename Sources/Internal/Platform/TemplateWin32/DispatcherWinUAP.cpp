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
    {
    }

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
        cv.Wait(lock, [this]() { return taskDone; });
    }

private:
    Mutex mutex;
    ConditionVariable cv;
    std::function<void()> task;
    bool taskDone = false;
};

} // unnamed namespace

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
{
}

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

void DispatcherWinUAP::ScheduleTask(std::function<void()>&& task, bool scheduleFirst)
{
    {
        LockGuard<Mutex> guard(mutex);
        if (scheduleFirst)
        {
            taskQueue.emplace_front(std::move(task));
        }
        else
        {
            taskQueue.emplace_back(std::move(task));
        }
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
    // To avoid some deadlock place blocking calls to main thread first in queue
    // How deadlock appears:
    //  1. WinUAPXamlApp::OnWindowVisibilityChanged occurs which calls main thread and makes blocking call back to UI
    //  2. PrivateTextFieldWinUAP::OnTextChanged occurs which ask delegate and blocks UI thread
    //  3. now main thread's dispatcher begins executing tasks
    //    3.1 execute task placed at step 1 which makes blocking call back to UI thread
    //    3.2 but UI thread already blocked in step 2
    //    3.3 ta-dam - and here is deadlock
    // !!! We MUST avoid interthread blocking calls !!!
    // Placing blocking call first in queue allows task in step 2 run before task 1
    // but such shuffling leads to violating task order, ups
    ScheduleTask(std::function<void()>([&taskWrapper]() { taskWrapper.RunTask(); }), true);
    taskWrapper.WaitTaskComplete();
    blockingCall.clear();
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
