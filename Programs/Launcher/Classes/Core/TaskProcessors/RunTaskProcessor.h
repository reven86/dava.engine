#pragma once

#include "Core/TaskProcessors/BaseTaskProcessor.h"

class RunTaskProcessor final : public BaseTaskProcessor
{
    void AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier) override;
    void Terminate() override;
};
