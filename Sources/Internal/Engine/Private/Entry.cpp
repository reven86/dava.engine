#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Engine/Private/CommandArgs.h"
#include "Engine/Private/EngineStartup.h"

// clang-format off

#if defined(__DAVAENGINE_QT__) || \
    defined(__DAVAENGINE_MACOS__) || \
    defined(__DAVAENGINE_IPHONE__) || \
    (defined(__DAVAENGINE_WIN32__) && defined(CONSOLE))

int main(int argc, char* argv[])
{
    using namespace DAVA;
    Vector<String> cmdargs = Private::GetCommandArgs(argc, argv);
    return Private::EngineStart(cmdargs);
}

#elif defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)

#include <windows.h>

// Win32
// To use WinMain in static lib with unicode support set entry point to wWinMainCRTStartup:
//  1. through linker commandline option /ENTRY:wWinMainCRTStartup
//  2. property panel Linker -> Advanced -> Entry Point
//  3. cmake script - set_target_properties(target PROPERTIES LINK_FLAGS "/ENTRY:wWinMainCRTStartup")
// https://msdn.microsoft.com/en-us/library/dybsewaf.aspx
// https://support.microsoft.com/en-us/kb/125750

#if defined(__DAVAENGINE_WIN_UAP__)
// Windows Universal Application
// WinMain should have attribute which specifies threading model
[Platform::MTAThread]
#endif
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    using namespace DAVA;
    Vector<String> cmdargs = Private::GetCommandArgs();
    return Private::EngineStart(cmdargs);
}

#endif

// clang-format on

#endif // __DAVAENGINE_COREV2__
