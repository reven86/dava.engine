#include "Core/Tasks/AsyncChainTask.h"

AsyncChainTask::AsyncChainTask(ApplicationManager* appManager)
    : BaseTask(appManager)
{
}

void AsyncChainTask::SetNotifier(const Notifier& notifier_)
{
    notifier = notifier_;
    Receiver receiver;
    receiver.onFinished = std::bind(&AsyncChainTask::OnFinished, this, std::placeholders::_1);
    notifier.AddReceiver(receiver);

    notifier.SetProgressDelimiter(GetSubtasksCount());
}

int AsyncChainTask::GetSubtasksCount() const
{
    return 1;
}

BaseTask::eTaskType AsyncChainTask::GetTaskType() const
{
    return ASYNC_CHAIN;
}
