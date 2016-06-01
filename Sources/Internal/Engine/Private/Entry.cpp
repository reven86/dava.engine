#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Engine/Private/InitEngine.h"

extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

#if defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// To use WinMain in static lib with unicode support set entry point to wWinMainCRTStartup:
//  1. through linker commandline option /ENTRY:wWinMainCRTStartup
//  2. property panel Linker -> Advanced -> Entry Point
//  3. cmake script - set_target_properties(target PROPERTIES LINK_FLAGS "/ENTRY:wWinMainCRTStartup")
// https://msdn.microsoft.com/en-us/library/dybsewaf.aspx
// https://support.microsoft.com/en-us/kb/125750
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    using namespace DAVA;
    Vector<String> cmdline = Private::InitializeEngine(0, nullptr);
    int r = GameMain(cmdline);
    Private::TerminateEngine();
    return r;
}

#else

#endif

#endif // __DAVAENGINE_COREV2__
