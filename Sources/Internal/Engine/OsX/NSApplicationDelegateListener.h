#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

DAVA_FORWARD_DECLARE_OBJC_CLASS(NSApplication);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSNotification);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSDictionary);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSData);
DAVA_FORWARD_DECLARE_OBJC_CLASS(NSError);

namespace DAVA
{
/**
    \ingroup engine_mac
    Interface definition for a callbacks to be invoked when NSApplicationDelegate lifecycle event occurs (applicationDidFinishLaunching, 
    applicationWillTerminate, etc).
    Only subset of NSApplicationDelegate methods are mapped to the interface definition, other methods are mapped as required.
    
    To receive callbacks from NSApplicationDelegate application should declare class derived from NSApplicationDelegateListener, implement
    necessary methods and register it through NativeService::RegisterNSApplicationDelegateListener.
*/
struct NSApplicationDelegateListener
{
    virtual ~NSApplicationDelegateListener() = default;

    virtual void applicationDidFinishLaunching(NSNotification* notification)
    {
    }
    virtual void applicationDidBecomeActive()
    {
    }
    virtual void applicationDidResignActive()
    {
    }
    virtual void applicationWillTerminate()
    {
    }

    virtual void didReceiveRemoteNotification(NSApplication* application, NSDictionary* userInfo)
    {
    }
    virtual void didRegisterForRemoteNotificationsWithDeviceToken(NSApplication* application, NSData* deviceToken)
    {
    }
    virtual void didFailToRegisterForRemoteNotificationsWithError(NSApplication* application, NSError* error)
    {
    }
};

} //namespace DAVA

#endif // __DAVAENGINE_MACOS__
