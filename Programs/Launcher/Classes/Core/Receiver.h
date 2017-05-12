#pragma once

#include <QVector>

#include <functional>

class BaseTask;

class Receiver
{
public:
    Receiver() = default;

    //this c-tor used only to have onSuccess callback. onFinished callback will be replaced with onSuccess
    //use this c-tor only for callback chain
    Receiver(std::function<void(const BaseTask*)> onSuccess);

    //say to a user that application is started
    std::function<void(const BaseTask*)> onStarted;

    //display progress to a user
    std::function<void(const BaseTask*, quint32)> onProgress;

    //task finished success or with a errors
    std::function<void(const BaseTask*)> onFinished;
};

class ReceiverNotifier final
{
public:
    ReceiverNotifier() = default;
    ReceiverNotifier(Receiver receiver);
    ReceiverNotifier(const QVector<Receiver>& receivers);

    //wrappers to avoid iteration over receivers
    void NotifyStarted(const BaseTask* task);
    void NotifyProgress(const BaseTask* task, quint32 progress);
    void NotifyFinished(const BaseTask* task);

private:
    QVector<Receiver> receivers;
};
