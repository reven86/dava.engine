#include "LazyUpdater.h"

#include <QTimer>

LazyUpdater::LazyUpdater(Updater updater_, QObject* parent /* = nullptr */)
    : QObject(parent)
    , updater(updater_)
{
}

void LazyUpdater::Update()
{
    ++counter;
    QTimer::singleShot(0, this, &LazyUpdater::OnTimer);
}

void LazyUpdater::OnTimer()
{
    if (counter > 1)
    {
        --counter;
        return;
    }

    counter = 0;

    updater();
}
