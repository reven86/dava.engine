#pragma once

#include "Core/Tasks/BaseTask.h"
#include "Core/Receiver.h"

#include <QObject>

#include <functional>

class AsyncChainTask : public QObject, public BaseTask
{
    Q_OBJECT

public:
    AsyncChainTask(ApplicationManager* appManager);

    void SetNotifier(const Notifier& notifier);

    virtual void Run() = 0;

    //this signal is used only by AsyncChainTaskProcessor
    //later AsyncChainProcessor and AsyncChainTask must be replaced by std::function
    //it must be done when launcher will be able to start multiple tasks at once
signals:
    void Finished() const;

protected:
    Notifier notifier;

private:
    virtual int GetSubtasksCount() const;

    eTaskType GetTaskType() const override;
    virtual void OnFinished(const BaseTask* task) = 0;
};
