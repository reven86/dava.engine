#include "Core/Tasks/AsyncChainTask.h"

AsyncChainTask::AsyncChainTask(ApplicationManager* appManager)
    : RunTask(appManager)
{
    taskType = BaseTask::ASYNC_CHAIN;
}

std::function<void(const BaseTask*)> AsyncChainTask::WrapCB(std::function<void(const BaseTask*)> cb) const
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
