#pragma once
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace DebugFS
{
struct IOErrorTypes
{
    // most interesting error codes(http://en.cppreference.com/w/cpp/header/cerrno):
    // ENOSPC - No space left on device
    // ENOENT - No such file or directory
    // ENFILE - Too many files open in system
    int32 ioErrorCode = 0;
    bool openOrCreateFailed = false;
    bool writeFailed = false;
    bool readFailed = false;
    bool seekFailed = false;
    bool closeFailed = false;
    bool truncateFailed = false;
};
void GenerateIOErrorOnNextOperation(IOErrorTypes types);
bool GenErrorOnOpenOrCreateFailed();
bool GenErrorOnWriteFailed();
bool GenErrorOnReadFailed();
bool GenErrorOnSeekFailed();
bool GenErrorOnCloseFailed();
bool GenErrorOnTruncateFailed();
}
} // end namespace DAVA
