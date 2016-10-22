#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

@class UIWindow;
@class UIView;
@class NSSet;

@class RenderView;
@class RenderViewController;
@class NativeViewPool;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for iOS's WindowBackend class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - posts events to dispatcher
//
// iOS window unions several Objective-C classes (UIView subclass,
// UIViewController subclass, etc) and each of these classes
// receive some kind of system notfications or events. WindowNativeBridge
// combines all window-related logic and processes events from Objective-C classes.
// Objective-C classes only forward its notifications to WindowNativeBridge.
//
// WindowNativeBridge is friend of iOS's WindowBackend
struct WindowNativeBridge final
{
    WindowNativeBridge(WindowBackend* windowBackend);
    ~WindowNativeBridge();

    void* GetHandle() const;
    bool CreateWindow();

    void TriggerPlatformEvents();

    void ApplicationDidBecomeOrResignActive(bool becomeActive);
    void ApplicationDidEnterForegroundOrBackground(bool foreground);

    void AddUIView(UIView* uiview);
    void RemoveUIView(UIView* uiview);

    UIView* GetUIViewFromPool(const char8* className);
    void ReturnUIViewToPool(UIView* view);

    //////////////////////////////////////////////////////////////////////////
    // Notifications from RenderViewController
    void LoadView();
    void ViewWillTransitionToSize(float32 w, float32 h);

    //////////////////////////////////////////////////////////////////////////
    // Notifications from RenderView
    void TouchesBegan(NSSet* touches);
    void TouchesMoved(NSSet* touches);
    void TouchesEnded(NSSet* touches);

    //////////////////////////////////////////////////////////////////////////

    WindowBackend* windowBackend = nullptr;
    Window* window = nullptr;
    MainDispatcher* mainDispatcher = nullptr;

    UIWindow* uiwindow = nullptr;
    RenderView* renderView = nullptr;
    RenderViewController* renderViewController = nullptr;
    NativeViewPool* nativeViewPool = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
