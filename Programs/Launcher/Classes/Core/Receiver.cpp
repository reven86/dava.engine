#include "Core/Receiver.h"
#include "Core/Tasks/BaseTask.h"

Receiver::Receiver(std::function<void(const BaseTask*)> onFinished_)
    : onFinished(onFinished_)
{
}

ReceiverNotifier::ReceiverNotifier(Receiver receiver)
    : receivers(1, receiver)
{
}

ReceiverNotifier::ReceiverNotifier(const QVector<Receiver>& receivers_)
    : receivers(receivers_)
{
}

void ReceiverNotifier::AddReceiver(Receiver receiver)
{
    receivers.push_back(receiver);
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
    for (Receiver& receiver : receivers)
    {
        if (receiver.onFinished)
        {
            receiver.onFinished(task);
        }
    }
}
