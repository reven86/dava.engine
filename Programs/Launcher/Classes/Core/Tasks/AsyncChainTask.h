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

protected:
    //use this function only to wrap children tasks
    CallbackFn WrapCallback(CallbackFn cb) const;

private:
    eTaskType GetTaskType() const override;
};
