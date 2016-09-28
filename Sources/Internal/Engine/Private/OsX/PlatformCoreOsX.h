#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

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

    // Through this signal WindowOsX gets notified about application hidden/unhidden state has changed
    // to update its visibility state
    Signal<bool> didHideUnhide;

private:
    int OnFrame();

    // Allows CoreNativeBridge class to access Window's WindowBackend instance
    // as CoreNativeBridge cannot make friends with Window class
    static WindowBackend* GetWindowBackend(Window* window);

private:
    EngineBackend* engineBackend = nullptr;

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

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
