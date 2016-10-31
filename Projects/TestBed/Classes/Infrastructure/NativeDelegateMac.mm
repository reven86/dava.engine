#include "Infrastructure/NativeDelegateMac.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

#include <Logger/Logger.h>
#include <Utils/NSStringUtils.h>

void NativeDelegateMac::applicationWillFinishLaunching()
{
    DAVA::Logger::Debug("TestBedMacNativeDelegate::applicationWillFinishLaunching");
}

void NativeDelegateMac::applicationDidFinishLaunching(NSNotification* notification)
{
    using namespace DAVA;
    String name = StringFromNSString([notification name]);
    Logger::Debug("TestBedMacNativeDelegate::applicationDidFinishLaunching: enter");
    Logger::Debug("    notification name=%s", name.c_str());
    Logger::Debug("    notification dictionary:");
    NSDictionary* dict = [notification userInfo];
    for (NSString* key in dict)
    {
        String k = StringFromNSString(key);
        String d = StringFromNSString([dict[key] description]);
        Logger::Debug("        %s: %d", k.c_str(), d.c_str());
    }
    Logger::Debug("TestBedMacNativeDelegate::applicationDidFinishLaunching: leave");
}

void NativeDelegateMac::applicationDidBecomeActive()
{
    DAVA::Logger::Debug("TestBedMacNativeDelegate::applicationDidBecomeActive");
}

void NativeDelegateMac::applicationDidResignActive()
{
    DAVA::Logger::Debug("TestBedMacNativeDelegate::applicationDidResignActive");
}

void NativeDelegateMac::applicationWillTerminate()
{
    DAVA::Logger::Debug("TestBedMacNativeDelegate::applicationWillTerminate");
}

void NativeDelegateMac::didReceiveRemoteNotification(NSApplication* application, NSDictionary* userInfo)
{
    using namespace DAVA;
    Logger::Debug("TestBedMacNativeDelegate::didReceiveRemoteNotification: enter");
    Logger::Debug("    dictionary:");
    for (NSString* key in userInfo)
    {
        String k = StringFromNSString(key);
        String d = StringFromNSString([userInfo[key] description]);
        Logger::Debug("        %s: %d", k.c_str(), d.c_str());
    }
    Logger::Debug("TestBedMacNativeDelegate::didReceiveRemoteNotification: leave");
}

void NativeDelegateMac::didRegisterForRemoteNotificationsWithDeviceToken(NSApplication* application, NSData* deviceToken)
{
    DAVA::Logger::Debug("TestBedMacNativeDelegate::didRegisterForRemoteNotificationsWithDeviceToken");
}

#endif // __DAVAENGINE_MACOS__
