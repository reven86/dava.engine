#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Functional/Signal.h"

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* e);
    ~PlatformCore();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void Quit();

    Signal<bool> didBecomeResignActive;
    Signal<bool> didEnterForegroundBackground;

private:
    int OnFrame();

    WindowBackend* CreateNativeWindow(Window* w, float32 width, float32 height);

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* dispatcher = nullptr;
    // TODO: std::unique_ptr
    CoreNativeBridgeiOS* bridge = nullptr;
    std::unique_ptr<NativeService> nativeService;

    // Friends
    friend struct CoreNativeBridgeiOS;
};

inline NativeService* PlatformCore::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
