#include "Core/Tasks/BaseTask.h"
#include "Core/ApplicationManager.h"

void TaskDataHolder::SetUserData(const QVariant& data)
{
    userData = data;
}

const QVariant& TaskDataHolder::GetUserData() const
{
    return userData;
}

void OperationResult::SetError(const QString& error) const
{
    errorText = error;
}

QString OperationResult::GetError() const
{
    return errorText;
}

bool OperationResult::HasError() const
{
    return errorText.isEmpty() == false;
}

BaseTask::BaseTask(ApplicationManager* appManager_)
    : appManager(appManager_)
{
}

void BaseTask::SetOnFinishCallback(CallbackFn callback)
{
    onFinishedCallback = callback;
}

void BaseTask::SetOnSuccessCallback(CallbackFn callback)
{
    onSuccessCallback = callback;
}

RunTask::RunTask(ApplicationManager* appManager)
    : BaseTask(appManager)
{
}

BaseTask::eTaskType RunTask::GetTaskType() const
{
    return BaseTask::RUN_TASK;
}
