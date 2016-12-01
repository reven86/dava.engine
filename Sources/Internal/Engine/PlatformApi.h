#pragma once

#include "Base/BaseTypes.h"

/**
    \ingroup engine
    Suite of functions to access some platform specific facilities.
*/

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)

class QApplication;

namespace DAVA
{
class RenderWidget;
class Window;
namespace PlatformApi
{
namespace Qt
{
void AcquireWindowContext(Window* targetWindow);
void ReleaseWindowContext(Window* targetWindow);

QApplication* GetApplication();
RenderWidget* GetRenderWidget();

} // namespace Qt
} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_WIN_UAP__)

/**
    \defgroup engine_win10 Engine facilities specific to Windows Unoversal Platform (Win10)
*/

namespace DAVA
{
class Window;
namespace PlatformApi
{
namespace Win10
{
/**
    \ingroup engine_win10
    Interface definition for a callbacks to be invoked when `Windows::UI::Xaml::Application` lifecycle event occurs (OnLaunched, etc).
    Only subset of `Windows::UI::Xaml::Application` methods are mapped to the interface definition, other methods are mapped as required.

    To receive callbacks application should declare class derived from `XamlApplicationListener`, implement necessary methods
    and register it through `RegisterXamlApplicationListener` function.

    Methods of `XamlApplicationListener` are always called in the context of primary UI thread (thread where first window is created).
*/
struct XamlApplicationListener
{
    // clang-format off
    virtual ~XamlApplicationListener() = default;
    virtual void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ args) {}
    virtual void OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args) {}
    // clang-format on
};

void AddXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl);
void RemoveXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl);
void PositionXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
void UnfocusXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl);
::Windows::UI::Xaml::Input::Pointer ^ GetLastPressedPointer(Window* targetWindow);

/**
    \ingroup engine_win10
    Register a callback to be invoked in response of `Windows::UI::Xaml::Application` lifecycle events.

    Application can register a callback from any thread, but callbacks are invoked in the context of primary UI thread.
    The best place to call this function is before calling `Engine::Run` or in `Engine::gameLoopStarted` signal handler.

    \pre `listener` should not be null pointer
    \pre Function shall not be called before `Engine::Init` or after `Engine::cleanup` signal.
*/
void RegisterXamlApplicationListener(XamlApplicationListener* listener);

/**
    \ingroup engine_win10
    Unregister a callback previously registered by `RegisterXamlApplicationListener` function.

    Application can unregister a callback from any thread, even during callback invocation.

    \pre `listener` should be previously registered
    \pre Function shall not be called after `Engine::cleanup` signal
*/
void UnregisterXamlApplicationListener(XamlApplicationListener* listener);

} // namespace Win10
} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_MACOS__)

/**
    \defgroup engine_mac Engine facilities specific to macOS platform
*/

DAVA_FORWARD_DECLARE_OBJC_CLASS(NSApplication);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSNotification);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSDictionary);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSData);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSError);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSView);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSUserNotification);

namespace DAVA
{
class Window;
namespace PlatformApi
{
namespace Mac
{
/**
    \ingroup engine_mac
    Interface definition for a callbacks to be invoked when `NSApplicationDelegate` lifecycle event occurs (applicationDidFinishLaunching,
    applicationWillTerminate, etc).
    Only subset of NSApplicationDelegate methods are mapped to the interface definition, other methods are mapped as required.

    To receive callbacks from `NSApplicationDelegate` application should declare class derived from `NSApplicationDelegateListener`, implement
    necessary methods and register it through `RegisterNSApplicationDelegateListener` function.

    Methods of `NSApplicationDelegateListener` are always called in the context of UI thread (for Mac UI thread and main thread are the same).
*/
struct NSApplicationDelegateListener
{
    // clang-format off
    virtual ~NSApplicationDelegateListener() = default;
    virtual void applicationDidFinishLaunching(NSNotification* notification) {}
    virtual void applicationDidBecomeActive() {}
    virtual void applicationDidResignActive() {}
    virtual void applicationWillTerminate() {}
    virtual void didReceiveRemoteNotification(NSApplication* application, NSDictionary* userInfo) {}
    virtual void didRegisterForRemoteNotificationsWithDeviceToken(NSApplication* application, NSData* deviceToken) {}
    virtual void didFailToRegisterForRemoteNotificationsWithError(NSApplication* application, NSError* error) {}
    virtual void didActivateNotification(NSUserNotification* notification) {}
    // clang-format on
};

void AddNSView(Window* targetWindow, NSView* nsview);
void RemoveNSView(Window* targetWindow, NSView* nsview);
void PrimaryWindowDeminiaturize();

/**
    \ingroup engine_mac
    Register a callback to be invoked in response of `NSApplicationDelegate` lifecycle events.

    Application can register a callback from any thread, but callbacks are invoked in the context of UI thread.
    The best place to call this function is before calling `Engine::Run` or in `Engine::gameLoopStarted` signal handler.

    \pre `listener` should not be null pointer
    \pre Function shall not be called before `Engine::Init` or after `Engine::cleanup` signal.
*/
void RegisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener);

/**
    \ingroup engine_mac
    Unregister a callback previously registered by `RegisterNSApplicationDelegateListener` function.

    Application can unregister a callback from any thread, even during callback invocation.

    \pre `listener` should be previously registered
    \pre Function shall not be called after `Engine::cleanup` signal
*/
void UnregisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener);

} // namespace Mac
} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_IPHONE__)

/**
    \defgroup engine_ios Engine facilities specific to iOS platform
*/

DAVA_FORWARD_DECLARE_OBJC_CLASS(UIApplication);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSDictionary);
DAVA_FORWARD_DECLARE_OBJC_CLASS(UIView);
DAVA_FORWARD_DECLARE_OBJC_CLASS(UIImage);
DAVA_FORWARD_DECLARE_OBJC_CLASS(UILocalNotification);

namespace DAVA
{
class Image;
class Window;
namespace PlatformApi
{
namespace Ios
{
/**
    \ingroup engine_ios
    Interface definition for a callbacks to be invoked when `UIApplicationDelegate` lifecycle event occurs (didFinishLaunchingWithOptions,
    applicationDidBecomeActive, etc).
    Only subset of `UIApplicationDelegate` methods are mapped to the interface definition, other methods are mapped as required.

    To receive callbacks from `UIApplicationDelegate` application should declare class derived from `UIApplicationDelegateListener`, implement
    necessary methods and register it through `RegisterUIApplicationDelegateListener` function.
 
    Methods of `UIApplicationDelegateListener` are always called in the context of UI thread (for iOS UI thread and main thread are the same).
*/
struct UIApplicationDelegateListener
{
    // clang-format off
    virtual ~UIApplicationDelegateListener() = default;
    virtual void didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions) {}
    virtual void applicationDidBecomeActive() {}
    virtual void applicationDidResignActive() {}
    virtual void applicationWillEnterForeground() {}
    virtual void applicationDidEnterBackground() {}
    virtual void applicationWillTerminate() {}
    virtual void didReceiveLocalNotification(UILocalNotification* notification) {}
    // clang-format on
};

void AddUIView(Window* targetWindow, UIView* uiview);
void RemoveUIView(Window* targetWindow, UIView* uiview);

UIView* GetUIViewFromPool(Window* targetWindow, const char8* className);
void ReturnUIViewToPool(Window* targetWindow, UIView* view);

UIImage* RenderUIViewToUIImage(UIView* view);
Image* ConvertUIImageToImage(UIImage* nativeImage);

/**
    \ingroup engine_ios
    Register a callback to be invoked in response of `UIApplicationDelegate` lifecycle events.

    Application can register a callback from any thread, but callbacks are invoked in the context of UI thread.
    The best place to call this function is before calling `Engine::Run` or in `Engine::gameLoopStarted` signal handler.

    \pre `listener` should not be null pointer
    \pre Function shall not be called before `Engine::Init` or after `Engine::cleanup` signal.
*/
void RegisterUIApplicationDelegateListener(UIApplicationDelegateListener* listener);

/**
    \ingroup engine_mac
    Unregister a callback previously registered by `RegisterUIApplicationDelegateListener` function.

    Application can unregister a callback from any thread, even during callback invocation.

    \pre `listener` should be previously registered
    \pre Function shall not be called after `Engine::cleanup` signal
*/
void UnregisterUIApplicationDelegateListener(UIApplicationDelegateListener* listener);

} // namespace Ios
} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"

namespace DAVA
{
class Window;
namespace PlatformApi
{
namespace Android
{
jobject CreateNativeControl(Window* targetWindow, const char8* controlClassName, void* backendPointer);

} // namespace Android
} // namespace PlatformApi
} // namespace DAVA

#endif
#endif // defined(__DAVAENGINE_COREV2__)
