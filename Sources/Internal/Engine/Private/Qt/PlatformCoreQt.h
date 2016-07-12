#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

namespace DAVA
{
class Window;

namespace Private
{
class PlatformCore final
{
public:
    PlatformCore();
    ~PlatformCore();

    PlatformCore(const PlatformCore&) = delete;
    PlatformCore& operator=(const PlatformCore&) = delete;

    void Init();
    void Run();
    void Quit();

    WindowBackend* CreateNativeWindow(Window* w);

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
