#include "Core/Tasks/AsyncChainTask.h"

AsyncChainTask::AsyncChainTask(ApplicationManager* appManager)
    : BaseTask(appManager)
{
}

BaseTask::eTaskType AsyncChainTask::GetTaskType() const
{
    return ASYNC_CHAIN;
}
