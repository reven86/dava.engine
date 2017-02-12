#include "ContinuousUpdater.h"

#include <QTimer>

ContinuousUpdater::ContinuousUpdater(Updater updater_, int updateInterval)
    : QObject(nullptr)
    , updater(updater_)
    , timer(new QTimer(this))
{
    timer->setSingleShot(true);
    timer->setInterval(updateInterval);
    connect(timer, &QTimer::timeout, this, &ContinuousUpdater::OnTimer);
}

void ContinuousUpdater::Update()
{
    needUpdate = true;

    if (!timer->isActive())
    {
        QTimer::singleShot(0, this, &ContinuousUpdater::OnTimer);
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
