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

    //this isgnal is used only by AsyncChainTaskProcessor
    //later AsyncChainProcessor and AsyncChainTask must be replaced by std::function
    //it must be done when launcher will be able to start multiple tasks at once
signals:
    void Finished() const;

protected:
    //use this function only to wrap children tasks
    CallbackFn WrapCallback(CallbackFn cb) const;

private:
    eTaskType GetTaskType() const override;
};
