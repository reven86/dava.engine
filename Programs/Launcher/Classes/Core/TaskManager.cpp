#include "Core/TaskManager.h"
#include "Core/TaskProcessors/RunTaskProcessor.h"
#include "Core/TaskProcessors/NetworkTaskProcessor.h"
#include "Core/TaskProcessors/ZipTaskProcessor.h"
#include "Core/TaskProcessors/AsyncChainProcessor.h"

TaskManager::TaskManager(QObject* parent)
    : QObject(parent)
{
    taskProcessors[BaseTask::RUN_TASK] = std::make_unique<RunTaskProcessor>();
    taskProcessors[BaseTask::DOWNLOAD_TASK] = std::make_unique<NetworkTaskProcessor>();
    taskProcessors[BaseTask::ZIP_TASK] = std::make_unique<ZipTaskProcessor>();
    taskProcessors[BaseTask::ASYNC_CHAIN] = std::make_unique<AsyncChainProcessor>();
}

TaskManager::~TaskManager() = default;

void TaskManager::AddTask(std::unique_ptr<BaseTask>&& task, const Receiver& receiver)
{
    taskProcessors[task->GetTaskType()]->AddTask(std::move(task), ReceiverNotifier(receiver));
}

void TaskManager::AddTask(std::unique_ptr<BaseTask>&& task, const QVector<Receiver>& receivers)
{
    taskProcessors[task->GetTaskType()]->AddTask(std::move(task), ReceiverNotifier(receivers));
}

void TaskManager::Terminate()
{
    for (auto& taskProcessor : taskProcessors)
    {
        taskProcessor.second->Terminate();
    }
}
