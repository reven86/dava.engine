#include "FileSystem/Private/CheckIOError.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace DebugFS
{
static IOErrorTypes ioErrors;
void GenerateIOErrorOnNextOperation(IOErrorTypes types)
{
    if (types.ioErrorCode != 0)
    {
        bool anyError = types.closeFailed ||
        types.openOrCreateFailed ||
        types.readFailed ||
        types.seekFailed ||
        types.truncateFailed ||
        types.writeFailed;

        DVASSERT(anyError);
    }
    ioErrors = types;
}
bool GenErrorOnOpenOrCreateFailed()
{
    if (ioErrors.openOrCreateFailed)
    {
        errno = ioErrors.ioErrorCode;
        return true;
    }
    return false;
}
bool GenErrorOnWriteFailed()
{
    if (ioErrors.writeFailed)
    {
        errno = ioErrors.ioErrorCode;
        return true;
    }
    return false;
}
bool GenErrorOnReadFailed()
{
    if (ioErrors.readFailed)
    {
        errno = ioErrors.ioErrorCode;
        return true;
    }
    return false;
}
bool GenErrorOnSeekFailed()
{
    if (ioErrors.seekFailed)
    {
        return true;
    }
    return false;
}
bool GenErrorOnCloseFailed()
{
    if (ioErrors.closeFailed)
    {
        errno = ioErrors.ioErrorCode;
        return true;
    }
    return false;
}
bool GenErrorOnTrancateFailed()
{
    if (ioErrors.truncateFailed)
    {
        errno = ioErrors.truncateFailed;
        return true;
    }
    return false;
}
}
} // end namespace DAVA
