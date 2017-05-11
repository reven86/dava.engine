#include "Core/TasksLogger.h"
#include "Core/Tasks/BaseTask.h"

#include "Utils/ErrorMessenger.h"

#include <QDateTime>

TasksLogger::TasksLogger()
{
    using namespace std::placeholders;
    receiver.onStarted = std::bind(&TasksLogger::OnTaskStarted, this, _1);
    receiver.onFinished = std::bind(&TasksLogger::OnTaskFinished, this, _1);
}

Receiver TasksLogger::GetReceiver() const
{
    return receiver;
}

void TasksLogger::OnTaskStarted(const BaseTask* task)
{
    ErrorMessenger::LogMessage(QtDebugMsg, QDateTime::currentDateTime().toString() + " : " + task->GetDescription());
}

void TasksLogger::OnTaskFinished(const BaseTask* task)
{
    if (task->HasError())
    {
        ErrorMessenger::LogMessage(QtWarningMsg, QDateTime::currentDateTime().toString() + " : " + "<span style =\"color:#aa0000;\">" + task->GetError() + "</span>");
    }
}
