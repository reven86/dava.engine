#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

namespace DAVA
{
class Window;

namespace Private
{
class WindowWin32;

class CoreQt final
{
public:
    CoreQt();
    ~CoreQt();

    CoreQt(const CoreQt&) = delete;
    CoreQt& operator=(const CoreQt&) = delete;

    Vector<String> GetCommandLine(int argc, char* argv[]);

    void Init();
    void Run();
    void Quit();

    WindowQt* CreateNativeWindow(Window* w);

    void (*AcqureContext())();
    void (*ReleaseContext())();

    void Prepare(void (*acqureContextFunc)(), void (*releaseContextFunc)());
    void OnFrame();

private:
    void (*acqureContext)() = nullptr;
    void (*releaseContext)() = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
