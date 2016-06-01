#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace DAVA
{
class Window;

namespace Private
{
class WindowWin32;

class CoreWin32 final
{
public:
    CoreWin32();
    ~CoreWin32();

    CoreWin32(const CoreWin32&) = delete;
    CoreWin32& operator=(const CoreWin32&) = delete;

    static HINSTANCE Win32AppInstance();

    Vector<String> GetCommandLine(int argc, char* argv[]);

    void Init(bool consoleMode_);
    void Run();
    void Quit();

    WindowWin32* CreateNativeWindow(Window* w);

private:
    void RunGUI();
    void RunConsole();

private:
    bool consoleMode = false;
    bool quitConsole = false;
    static HINSTANCE hinstance;
};

inline HINSTANCE CoreWin32::Win32AppInstance()
{
    return hinstance;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
