#include "CEFDavaRenderApp.h"
#include "Base/Platform.h"

/*#ifdef __DAVAENGINE_WINDOWS__

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
    CefMainArgs mainArgs(hInstance);
    return CefExecuteProcess(mainArgs, nullptr, nullptr);
}

#else

int main(int argc, char* argv[])
{
    CefMainArgs mainArgs(argc, argv);
    return CefExecuteProcess(mainArgs, nullptr, nullptr);
}

#endif*/

int main(int argc, char* argv[])
{
    CefMainArgs mainArgs(::GetModuleHandle(nullptr));
    return CefExecuteProcess(mainArgs, new CEFDavaRenderApp, nullptr);
}