#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
/**
    \ingroup engine_win10
    NativeService provides access to some Win10 (Universal Windows Platform) platform specific services.

    NativeService instance is obtained through `Engine::GetNativeService` method.
*/
class NativeService final
{
public:
    /**
        Register a listener to be invoked when `Windows::UI::Xaml::Application` lifecycle event occurs.
        Listener should implement `DAVA::XamlApplicationListener` interface.
    */
    void RegisterXamlApplicationListener(XamlApplicationListener* listener);

    /**
        Unregister `Windows::UI::Xaml::Application` listener.
    */
    void UnregisterXamlApplicationListener(XamlApplicationListener* listener);

private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
