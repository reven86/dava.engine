#pragma once

#include "Core/Receiver.h"

#include <memory>

class BaseTask;

class BaseTaskProcessor
{
public:
    virtual ~BaseTaskProcessor() = default;
    virtual void AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier) = 0;
    virtual void Terminate() = 0;
};
