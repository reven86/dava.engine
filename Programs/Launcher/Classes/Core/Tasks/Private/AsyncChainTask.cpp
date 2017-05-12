#include "Core/Tasks/AsyncChainTask.h"

AsyncChainTask::AsyncChainTask(ApplicationManager* appManager)
    : BaseTask(appManager)
{
}

BaseTask::eTaskType AsyncChainTask::GetTaskType() const
{
    return ASYNC_CHAIN;
}

BaseTask::CallbackFn AsyncChainTask::WrapCallback(CallbackFn cb) const
{
    return [cb, this](const BaseTask* task) {
        if (task->HasError())
        {
            emit Finished();
        }
        else
        {
            cb(task);
        }
    };
}
