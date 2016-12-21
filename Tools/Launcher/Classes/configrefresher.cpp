#include "configrefresher.h"
#include <QTimer>

const char* packageName = "ConfigRefresher";

ConfigRefresher::ConfigRefresher(QObject* parent /*= nullptr*/)
{
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(300 * 1000); //default timeout
    timer->start();
    connect(timer, &QTimer::timeout, this, &ConfigRefresher::RefreshConfig);
}

bool ConfigRefresher::IsEnabled() const
{
    return timer->isActive();
}

int ConfigRefresher::GetTimeout() const
{
    return timer->interval();
}

int ConfigRefresher::GetMinimumTimeout() const
{
    return 60 * 1000; //one minute
}

void ConfigRefresher::SetEnabled(bool enabled)
{
    if (enabled)
    {
        timer->start();
    }
    else
    {
        timer->stop();
    }
}

void ConfigRefresher::SetTimeout(int timeoutMs)
{
    timeoutMs = qMax(timeoutMs, GetMinimumTimeout());
    timer->setInterval(timeoutMs);
}
