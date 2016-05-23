#ifndef __TOOL_CONTINUOUS_UPDATER_H__
#define __TOOL_CONTINUOUS_UPDATER_H__

#include "Functional/Function.h"

#include <QObject>

class QTimer;
class ContinuousUpdater : public QObject
{
    Q_OBJECT
public:
    using Updater = DAVA::Function<void()>;

public:
    ContinuousUpdater(Updater updater, QObject* parent = nullptr, int updateInterval = 0);

    void Update();
    void Stop(); //sync method to stop timer and call Update if it's needed

private slots:
    void OnTimer();

private:
    Updater updater;
    QTimer* timer = nullptr;
    bool needUpdate = false;
};

#endif // __TOOL_CONTINUOUS_UPDATER_H__
