#ifndef __QT_TOOLS_WIN_CONSOLE_IO_LOCKER_H__
#define __QT_TOOLS_WIN_CONSOLE_IO_LOCKER_H__

#include "Base/BaseTypes.h"
#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

namespace WinConsoleIO
{
struct IOHandle;
} //END of WinConsoleIO

class WinConsoleIOLocker final
{
public:
    WinConsoleIOLocker();
    ~WinConsoleIOLocker();

private:
    std::unique_ptr<WinConsoleIO::IOHandle> ioHandle;
};

#endif //#if defined(__DAVAENGINE_WIN32__)

#endif // __QT_TOOLS_WIN_CONSOLE_IO_LOCKER_H__
