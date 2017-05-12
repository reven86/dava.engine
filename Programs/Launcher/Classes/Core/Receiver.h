#pragma once

#include <Qt>
#include <vector>
#include <functional>

class BaseTask;

class Receiver
{
public:
    Receiver() = default;

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
    ReceiverNotifier(const std::vector<Receiver>& receivers);

    //wrappers to avoid iteration over receivers
    void NotifyStarted(const BaseTask* task);
    void NotifyProgress(const BaseTask* task, quint32 progress);
    void NotifyFinished(const BaseTask* task);

private:
    std::vector<Receiver> receivers;
};
