#pragma once

#include "QtTools/Utils/QtDelayedExecutor.h"
#include <Functional/Function.h>

class QTimer;
class ContinuousUpdater
{
public:
    using Updater = DAVA::Function<void()>;
    using Stopper = DAVA::Function<bool()>;

    ContinuousUpdater(int updateInterval);
    ~ContinuousUpdater();

    void SetUpdater(const Updater& updater);
    void SetStopper(const Stopper& stopper);

    void Update();
    void Stop(); //sync method to stop timer and call Update
    void Abort(); //sync method to stop timer and don't call update

    void OnTimer();

private:
    Updater updater;
    Stopper stopper;
    std::unique_ptr<QTimer> timer;
    QtDelayedExecutor delayedExecutor;
    bool needUpdate = false;
};
