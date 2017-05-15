#pragma once

#include "Core/Tasks/BaseTask.h"
#include "Core/Receiver.h"

#include <QObject>

#include <memory>

class BaseTaskProcessor;

class TaskManager : public QObject
{
    Q_OBJECT

public:
    TaskManager(QObject* parent = nullptr);
    ~TaskManager();

    void AddTask(std::unique_ptr<BaseTask>&& task, const Receiver& receiver);
    void AddTask(std::unique_ptr<BaseTask>&& task, const std::vector<Receiver>& receivers);

public slots:
    void Terminate();

private:
    std::map<BaseTask::eTaskType, std::unique_ptr<BaseTaskProcessor>> taskProcessors;
};
