#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"

#if defined(__DAVAENGINE_WINDOWS__)
#include <windows.h>
#include <shellapi.h>
#endif

namespace DAVA
{
namespace Private
{
Vector<String> GetCommandArgs(int argc, char* argv[])
{
    Vector<String> cmdargs;
    cmdargs.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        cmdargs.push_back(argv[i]);
    }
    return cmdargs;
}

Vector<String> GetCommandArgs(const String& cmdline)
{
    // TODO: manually break command line into args
    Vector<String> cmdargs;
    if (cmdline.empty())
    {
        cmdargs.push_back(cmdline);
    }
    return cmdargs;
}

#if defined(__DAVAENGINE_WIN32__)

Vector<String> GetCommandArgs()
{
    Vector<String> cmdargs;

    int nargs = 0;
    LPWSTR cmdline = ::GetCommandLineW();
    LPWSTR* arglist = ::CommandLineToArgvW(cmdline, &nargs);
    if (arglist != nullptr)
    {
        cmdargs.reserve(nargs);
        for (int i = 0; i < nargs; ++i)
        {
            cmdargs.push_back(UTF8Utils::EncodeToUTF8(arglist[i]));
        }
        ::LocalFree(arglist);
    }
    return cmdargs;
}

#elif defined(__DAVAENGINE_WIN_UAP__)

Vector<String> GetCommandArgs()
{
    LPWSTR cmdline = ::GetCommandLineW();
    return GetCommandArgs(UTF8Utils::EncodeToUTF8(cmdline));
}

#endif

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
