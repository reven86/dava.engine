#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>

namespace QtHelpers
{
class RunGuard
{
public:
    RunGuard(const QString& key);
    ~RunGuard();

    bool TryToRun();

private:
    bool IsAnotherRunning();
    void Release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    Q_DISABLE_COPY(RunGuard)
};
}
