#include "ContinuousUpdater.h"

#include <Debug/DVAssert.h>

#include <QTimer>

ContinuousUpdater::ContinuousUpdater(int updaterInterval)
    : timer(new QTimer(nullptr))
{
    timer->setSingleShot(true);
    timer->setInterval(updaterInterval);

    QObject::connect(timer.get(), &QTimer::timeout, [this]() { OnTimer(); });
}

ContinuousUpdater::~ContinuousUpdater() = default;

void ContinuousUpdater::SetUpdater(const Updater& updater_)
{
    updater = updater_;
}

void ContinuousUpdater::SetStopper(const Stopper& stopper_)
{
    stopper = stopper_;
}

void ContinuousUpdater::Update()
{
    needUpdate = true;

    if (stopper)
    {
        if (stopper())
        {
            Stop();
            return;
        }
    }
    if (!timer->isActive())
    {
        delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ContinuousUpdater::OnTimer));
    }
}

void ContinuousUpdater::Stop()
{
    timer->stop();
    updater();
    needUpdate = false;
}

void ContinuousUpdater::Abort()
{
    timer->stop();
    needUpdate = false;
}

void ContinuousUpdater::OnTimer()
{
    if (needUpdate)
    {
        DVASSERT(updater, "updater is not set");
        updater();
        needUpdate = false;
        timer->start();
    }
}
