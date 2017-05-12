#pragma once

#include "Core/Tasks/BaseTask.h"

#include <QObject>

#include <functional>

class AsyncChainTask : public QObject, public BaseTask
{
    Q_OBJECT

public:
    AsyncChainTask(ApplicationManager* appManager);

    virtual void Run() = 0;

signals:
    void Finished() const;

private:
    eTaskType GetTaskType() const override;
};
