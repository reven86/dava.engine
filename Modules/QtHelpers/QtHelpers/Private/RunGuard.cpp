#include "QtHelpers/RunGuard.h"

#include <QCryptographicHash>

namespace QtHelpers
{
namespace RunGuardDetail
{
QString generateKeyHash(const QString& key, const QString& salt)
{
    QByteArray data;

    data.append(key.toUtf8());
    data.append(salt.toUtf8());
    data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();

    return data;
}
}

RunGuard::RunGuard(const QString& key)
    : key(key)
    , memLockKey(RunGuardDetail::generateKeyHash(key, "_memLockKey"))
    , sharedmemKey(RunGuardDetail::generateKeyHash(key, "_sharedmemKey"))
    , sharedMem(sharedmemKey)
    , memLock(memLockKey, 1)
{
    QSharedMemory fix(sharedmemKey); // Fix for *nix: http://habrahabr.ru/post/173281/
    fix.attach();
}

RunGuard::~RunGuard()
{
    Release();
}

bool RunGuard::IsAnotherRunning()
{
    if (sharedMem.isAttached())
        return false;

    memLock.acquire();
    const bool isRunning = sharedMem.attach();
    if (isRunning)
        sharedMem.detach();
    memLock.release();

    return isRunning;
}

bool RunGuard::TryToRun()
{
    if (IsAnotherRunning()) // Extra check
        return false;

    memLock.acquire();
    const bool result = sharedMem.create(sizeof(quint64));
    memLock.release();
    if (!result)
    {
        Release();
        return false;
    }

    return true;
}

void RunGuard::Release()
{
    memLock.acquire();
    if (sharedMem.isAttached())
        sharedMem.detach();
    memLock.release();
}
} // namespace QtHelpers
