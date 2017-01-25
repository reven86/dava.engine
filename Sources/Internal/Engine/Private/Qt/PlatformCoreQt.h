#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EnginePrivateFwd.h"
#include "Functional/Signal.h"

class QApplication;
namespace DAVA
{
class RenderWidget;
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* engineBackend);
    ~PlatformCore();

    PlatformCore(const PlatformCore&) = delete;
    PlatformCore& operator=(const PlatformCore&) = delete;

    void Init();
    void Run();
    void PrepareToQuit();
    void Quit();

    QApplication* GetApplication();
    RenderWidget* GetRenderWidget();

private:
    EngineBackend& engineBackend;
    WindowBackend* primaryWindowBackend = nullptr;

    Signal<bool> applicationFocusChanged;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
