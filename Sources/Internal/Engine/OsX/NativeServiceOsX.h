#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
/**
    \ingroup engine_mac
    NativeService provides access to some macOS platform specific services.
    
    NativeService instance is obtained through `Engine::GetNativeService` method.
*/
class NativeService final
{
public:
    /**
        Register a listener to be invoked when `NSApplicationDelegate` lifecycle event occurs.
        Listener should implement `DAVA::NSApplicationDelegateListener` interface.
    */
    void RegisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener);

    /**
        Unregister `NSApplicationDelegate` listener.
    */
    void UnregisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener);

private:
    NativeService(Private::CoreNativeBridge* bridge_);

    Private::CoreNativeBridge* bridge = nullptr;

    // Friends
    friend Private::PlatformCore;
};

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
