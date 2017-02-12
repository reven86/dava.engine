#pragma once

#include "Functional/Function.h"

#include <QObject>

class QTimer;
class ContinuousUpdater : public QObject
{
    Q_OBJECT
public:
    using Updater = DAVA::Function<void()>;

public:
    ContinuousUpdater(Updater updater, int updateInterval = 0);

    void Update();
    void Stop(); //sync method to stop timer and call Update if it's needed
    void Abort(); //sync method to stop timer and not call update

private slots:
    void OnTimer();

private:
    Updater updater;
    QTimer* timer = nullptr;
    bool needUpdate = false;
};
