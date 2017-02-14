#include "ContinuousUpdater.h"

#include <QTimer>

ContinuousUpdater::ContinuousUpdater(Updater updater_, int updateInterval)
    : updater(updater_)
    , timer(new QTimer(nullptr))
{
    timer->setSingleShot(true);
    timer->setInterval(updateInterval);

    QObject::connect(timer.get(), &QTimer::timeout, [this](){OnTimer(); });
}

ContinuousUpdater::~ContinuousUpdater() = default;

void ContinuousUpdater::Update()
{
    needUpdate = true;

    if (!timer->isActive())
    {
        delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ContinuousUpdater::OnTimer));
    }
}

void ContinuousUpdater::Stop()
{
    timer->stop();
    if (needUpdate)
    {
        updater();
        needUpdate = false;
    }
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
        updater();
        needUpdate = false;
        timer->start();
    }
}
