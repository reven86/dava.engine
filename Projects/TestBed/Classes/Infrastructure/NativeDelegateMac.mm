#include "Infrastructure/NativeDelegateMac.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

#include <Logger/Logger.h>
#include <Utils/NSStringUtils.h>

void NativeDelegateMac::applicationDidFinishLaunching(NSNotification* notification)
{
    using namespace DAVA;
    String name = StringFromNSString([notification name]);
    Logger::Debug("TestBed.NativeDelegateMac::applicationDidFinishLaunching: enter");
    Logger::Debug("    notification name=%s", name.c_str());
    Logger::Debug("    notification dictionary:");
    NSDictionary* dict = [notification userInfo];
    for (NSString* key in dict)
    {
        String k = StringFromNSString(key);
        String d = StringFromNSString([dict[key] description]);
        Logger::Debug("        %s: %d", k.c_str(), d.c_str());
    }
    Logger::Debug("TestBed.NativeDelegateMac::applicationDidFinishLaunching: leave");
}

void NativeDelegateMac::applicationDidBecomeActive()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateMac::applicationDidBecomeActive");
}

void NativeDelegateMac::applicationDidResignActive()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateMac::applicationDidResignActive");
}

void NativeDelegateMac::applicationWillTerminate()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateMac::applicationWillTerminate");
}

void NativeDelegateMac::didReceiveRemoteNotification(NSApplication* application, NSDictionary* userInfo)
{
    using namespace DAVA;
    Logger::Debug("TestBed.NativeDelegateMac::didReceiveRemoteNotification: enter");
    Logger::Debug("    dictionary:");
    for (NSString* key in userInfo)
    {
        String k = StringFromNSString(key);
        String d = StringFromNSString([userInfo[key] description]);
        Logger::Debug("        %s: %d", k.c_str(), d.c_str());
    }
    Logger::Debug("TestBed.NativeDelegateMac::didReceiveRemoteNotification: leave");
}

void NativeDelegateMac::didRegisterForRemoteNotificationsWithDeviceToken(NSApplication* application, NSData* deviceToken)
{
    DAVA::Logger::Debug("TestBed.NativeDelegateMac::didRegisterForRemoteNotificationsWithDeviceToken");
}

void NativeDelegateMac::didFailToRegisterForRemoteNotificationsWithError(NSApplication* application, NSError* error)
{
    using namespace DAVA;
    String descr = StringFromNSString([error localizedDescription]);
    DAVA::Logger::Debug("TestBed.NativeDelegateMac::didFailToRegisterForRemoteNotificationsWithError: %s", descr.c_str());
}

#endif // __DAVAENGINE_MACOS__
