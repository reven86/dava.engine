#pragma once

#include "Core/TaskProcessors/BaseTaskProcessor.h"

#include <QObject>

class AsyncChainTask;

//designed only to manage memory for async chain tasks
class AsyncChainProcessor final : public QObject, public BaseTaskProcessor
{
    Q_OBJECT

private slots:
    void OnFinished();

private:
    void AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier) override;
    void Terminate() override;

    void StartNextTask();

    struct TaskParams
    {
        TaskParams(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier);
        ~TaskParams();

        std::unique_ptr<AsyncChainTask> task;
        ReceiverNotifier notifier;
    };
    std::list<TaskParams> tasks;

    bool running = false;
};
