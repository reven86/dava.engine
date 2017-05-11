#pragma once

#include "Core/Tasks/BaseTask.h"

#include <QObject>

#include <functional>

class AsyncChainTask : public QObject, public RunTask
{
    Q_OBJECT

public:
    AsyncChainTask(ApplicationManager* appManager);

    std::function<void(const BaseTask*)> WrapCB(std::function<void(const BaseTask*)> cb) const;

signals:
    void Finished() const;
};
