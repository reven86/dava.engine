#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Functional/Signal.h"

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* engineBackend);
    ~PlatformCore();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void PrepareToQuit();
    void Quit();

    // Signals for distribution UIApplicationDelegate's notifications:
    //  - applicationDidBecomeActive/applicationWillResignActive
    //  - applicationWillEnterForeground/applicationDidEnterBackground
    // WindowBackends usually connect to these signals to manage its focus
    // and visibility states
    Signal<bool> didBecomeResignActive;
    Signal<bool> didEnterForegroundBackground;

private:
    int32 OnFrame();

    // Allows CoreNativeBridge class to access Window's WindowBackend instance
    // as CoreNativeBridge cannot make friends with Window class
    static WindowBackend* GetWindowBackend(Window* window);

    EngineBackend& engineBackend;
    MainDispatcher& dispatcher;

    std::unique_ptr<CoreNativeBridge> bridge;
    std::unique_ptr<NativeService> nativeService;

    // Friends
    friend struct CoreNativeBridge;
};

inline NativeService* PlatformCore::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
