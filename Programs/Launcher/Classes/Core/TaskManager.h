#pragma once

#include "Core/Tasks/BaseTask.h"
#include "Core/Receiver.h"

#include <QObject>
#include <QVector>
#include <QMap>

#include <memory>

class BaseTaskProcessor;
class QString;

class TaskManager : public QObject
{
    Q_OBJECT

public:
    TaskManager(QObject* parent = nullptr);

    void AddTask(std::unique_ptr<BaseTask>&& task, const Receiver& receiver);
    void AddTask(std::unique_ptr<BaseTask>&& task, const QVector<Receiver>& receivers);

public slots:
    void Terminate();

private:
    std::map<BaseTask::eTaskType, std::unique_ptr<BaseTaskProcessor>> taskProcessors;
};
