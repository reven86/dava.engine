#pragma once

#include "QtTools/Utils/QtDelayedExecutor.h"
#include <Functional/Function.h>

class QTimer;
class ContinuousUpdater
{
public:
    using Updater = DAVA::Function<void()>;

    ContinuousUpdater(Updater updater, int updateInterval = 0);
    ~ContinuousUpdater();

    void Update();
    void Stop(); //sync method to stop timer and call Update if it's needed
    void Abort(); //sync method to stop timer and not call update

    void OnTimer();

private:
    Updater updater;
    std::unique_ptr<QTimer> timer;
    QtDelayedExecutor delayedExecutor;
    bool needUpdate = false;
};
