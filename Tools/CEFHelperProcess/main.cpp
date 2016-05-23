#include <cef/include/cef_app.h>
#include "Base/Platform.h"

#ifdef __DAVAENGINE_WINDOWS__

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
    CefMainArgs mainArgs(::GetModuleHandle(nullptr));
    return CefExecuteProcess(mainArgs, nullptr, nullptr);
}

#else

int main(int argc, char* argv[])
{
    CefMainArgs mainArgs(argc, argv);
    return CefExecuteProcess(mainArgs, nullptr, nullptr);
}

#endif