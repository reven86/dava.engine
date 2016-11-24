#include "silentupdater.h"
#include "applicationmanager.h"

#include <QTimer>

SilentUpdater::SilentUpdater(ApplicationManager* appManager_, QObject* parent /* = nullptr */)
    : QObject(parent)
    , applicationManager(appManager_)
{
    updateTimer = new QTimer(this);
    updateTimer->setSingleShot(false);
    updateTimer->setInterval(1000);
}
