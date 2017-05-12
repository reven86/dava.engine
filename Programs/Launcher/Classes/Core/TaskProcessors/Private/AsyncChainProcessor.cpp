#include "Core/TaskProcessors/AsyncChainProcessor.h"
#include "Core/Tasks/AsyncChainTask.h"

void AsyncChainProcessor::OnFinished()
{
    tasks.pop_front();

    running = false;
    StartNextTask();
}

void AsyncChainProcessor::AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier)
{
    Q_ASSERT(task->GetTaskType() == BaseTask::ASYNC_CHAIN);
    AsyncChainTask* chainTask = static_cast<AsyncChainTask*>(task.get());
    connect(chainTask, &AsyncChainTask::Finished, this, &AsyncChainProcessor::OnFinished);

    tasks.emplace_back(std::move(task), notifier);

    StartNextTask();
}

void AsyncChainProcessor::StartNextTask()
{
    if (running)
    {
        return;
    }
    if (tasks.empty() == false)
    {
        running = true;

        TaskParams& params = tasks.front();
        params.notifier.NotifyStarted(params.task.get());
        params.task->Run();
    }
}

void AsyncChainProcessor::Terminate()
{
    //do nothing for now
}

AsyncChainProcessor::TaskParams::TaskParams(std::unique_ptr<BaseTask>&& task_, ReceiverNotifier notifier_)
    : task(static_cast<AsyncChainTask*>(task_.release()))
    , notifier(notifier_)
{
}

AsyncChainProcessor::TaskParams::~TaskParams()
{
    notifier.NotifyFinished(task.get());
}
