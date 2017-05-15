#include "Core/Receiver.h"
#include "Core/Tasks/BaseTask.h"

ReceiverNotifier::ReceiverNotifier(Receiver receiver)
    : receivers(1, receiver)
{
}

ReceiverNotifier::ReceiverNotifier(const std::vector<Receiver>& receivers_)
    : receivers(receivers_)
{
}

void ReceiverNotifier::NotifyStarted(const BaseTask* task)
{
    for (Receiver& receiver : receivers)
    {
        if (receiver.onStarted)
        {
            receiver.onStarted(task);
        }
    }
}

void ReceiverNotifier::NotifyProgress(const BaseTask* task, quint32 progress)
{
    for (Receiver& receiver : receivers)
    {
        if (receiver.onProgress)
        {
            receiver.onProgress(task, progress);
        }
    }
}

void ReceiverNotifier::NotifyFinished(const BaseTask* task)
{
    //first notify receiver that task if finished
    //receiver can not produce another tasks on finished
    //if you need to produce one - create AsyncChainTask
    for (Receiver& receiver : receivers)
    {
        if (receiver.onFinished)
        {
            receiver.onFinished(task);
        }
    }

    if (task->onFinishedCallback)
    {
        task->onFinishedCallback(task);
    }
}
