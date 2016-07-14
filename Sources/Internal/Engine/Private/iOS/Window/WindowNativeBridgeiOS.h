#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

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
// WindowNativeBridgeiOS is friend of iOS's WindowBackend
struct WindowNativeBridge final
{
    WindowNativeBridge(WindowBackend* wbackend);
    ~WindowNativeBridge();

    void* GetHandle() const;
    bool DoCreateWindow();

    void TriggerPlatformEvents();

    void ApplicationDidBecomeOrResignActive(bool becomeActive);
    void ApplicationDidEnterForegroundOrBackground(bool foreground);

    UIView* CreateNativeControl(const char8* className);
    void ReleaseNativeControl(UIView* view);

    //////////////////////////////////////////////////////////////////////////

    void loadView();
    void viewWillTransitionToSize(float32 w, float32 h);

    //////////////////////////////////////////////////////////////////////////

    void touchesBegan(NSSet* touches);
    void touchesMoved(NSSet* touches);
    void touchesEnded(NSSet* touches);

    //////////////////////////////////////////////////////////////////////////

    WindowBackend* windowBackend = nullptr;

    UIWindow* uiwindow = nullptr;
    UIView* x = nullptr;
    RenderView* renderView = nullptr;
    RenderViewController* renderViewController = nullptr;
    NativeViewPool* nativeViewPool = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
