#pragma once

#include "Core/Receiver.h"

#include <memory>

class BaseTask;

class BaseTaskProcessor
{
public:
    virtual ~BaseTaskProcessor() = default;

    //give ownership of f task and make it run as soon as it possible
    virtual void AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier) = 0;

    //stop any active process
    //inherited processor must add a error to task to mark it unfinished
    virtual void Terminate() = 0;
};
