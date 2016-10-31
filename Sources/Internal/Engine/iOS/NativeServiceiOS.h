#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
/**
    \ingroup engine_ios
    NativeService provides access to some iOS platform specific services.

    NativeService instance is obtained through `Engine::GetNativeService` method.
*/
class NativeService final
{
public:
    /**
        Register a listener to be invoked when `UIApplicationDelegate` lifecycle event occurs.
        Listener should implement `DAVA::UIApplicationDelegateListener` interface.
     */
    void RegisterUIApplicationDelegateListener(UIApplicationDelegateListener* listener);

    /**
        Unregister `UIApplicationDelegate` listener.
     */
    void UnregisterUIApplicationDelegateListener(UIApplicationDelegateListener* listener);

private:
    NativeService(Private::CoreNativeBridge* bridge_);

    Private::CoreNativeBridge* bridge = nullptr;

    // Friends
    friend Private::PlatformCore;
};

} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
