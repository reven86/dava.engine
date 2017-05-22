#include "FileSystem/Private/CheckIOError.h"

namespace DAVA
{
static IOErrorTypes ioErrors;
void GenerateIOErrorOnNextOperation(IOErrorTypes types)
{
    ioErrors = types;
}
bool GenErrorOnOpenFile()
{
    if (ioErrors.openFile)
    {
        errno = ioErrors.ioErrorCode;
        return true;
    }
    return false;
}
bool GenErrorOnWriteToFile()
{
    if (ioErrors.writeFile)
    {
        errno = ioErrors.ioErrorCode;
        return true;
    }
    return false;
}
} // end namespace DAVA
