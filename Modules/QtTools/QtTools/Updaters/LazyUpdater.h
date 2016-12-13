#ifndef __TOOL_LAZY_UPDATER_H__
#define __TOOL_LAZY_UPDATER_H__

#include "Functional/Function.h"

#include <QObject>

class LazyUpdater : public QObject
{
    Q_OBJECT
public:
    using Updater = DAVA::Function<void()>;

public:
    LazyUpdater(Updater updater, QObject* parent = nullptr);

    void Update();

private slots:

    void OnTimer();

private:
    Updater updater;
    int counter = 0;
};

#endif // __TOOL_LAZY_UPDATER_H__
