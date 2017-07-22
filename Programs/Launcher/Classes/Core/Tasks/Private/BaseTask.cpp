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

void ErrorHolder::SetError(const QString& error) const
{
    errorText = error;
}

QString ErrorHolder::GetError() const
{
    return errorText;
}

bool ErrorHolder::HasError() const
{
    return errorText.isEmpty() == false;
}

BaseTask::BaseTask(ApplicationManager* appManager_)
    : appManager(appManager_)
{
}

RunTask::RunTask(ApplicationManager* appManager)
    : BaseTask(appManager)
{
}

BaseTask::eTaskType RunTask::GetTaskType() const
{
    return BaseTask::RUN_TASK;
}
