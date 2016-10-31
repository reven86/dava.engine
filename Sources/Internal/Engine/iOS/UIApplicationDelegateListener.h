#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

DAVA_FORWARD_DECLARE_OBJC_CLASS(UIApplication);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSDictionary);

namespace DAVA
{
/**
    \ingroup engine_ios
    Interface definition for a callbacks to be invoked when UIApplicationDelegate lifecycle event occurs (didFinishLaunchingWithOptions,
    applicationDidBecomeActive, etc).
    Only subset of UIApplicationDelegate methods are mapped to the interface definition, other methods are mapped as required.

    To receive callbacks from UIApplicationDelegate application should declare class derived from UIApplicationDelegateListener, implement
    necessary methods and register it through NativeService::RegisterUIApplicationDelegateListener.
*/
struct UIApplicationDelegateListener
{
    virtual ~UIApplicationDelegateListener() = default;

    virtual void didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions)
    {
    }
    virtual void applicationDidBecomeActive()
    {
    }
    virtual void applicationDidResignActive()
    {
    }
    virtual void applicationWillEnterForeground()
    {
    }
    virtual void applicationDidEnterBackground()
    {
    }
    virtual void applicationWillTerminate()
    {
    }
};

} //namespace DAVA

#endif // __DAVAENGINE_IPHONE__
