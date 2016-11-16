#include "Infrastructure/NativeDelegateIos.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>

#include <Logger/Logger.h>
#include <Utils/NSStringUtils.h>

void NativeDelegateIos::didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions)
{
    using namespace DAVA;
    Logger::Debug("NativeDelegateIos::didFinishLaunchingWithOptions: enter");
    Logger::Debug("    launch options:");
    for (NSString* key in launchOptions)
    {
        String k = StringFromNSString(key);
        String d = StringFromNSString([launchOptions[key] description]);
        Logger::Debug("        %s: %d", k.c_str(), d.c_str());
    }
    Logger::Debug("TestBed.NativeDelegateIos::didFinishLaunchingWithOptions: leave");
}

void NativeDelegateIos::applicationDidBecomeActive()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateIos::applicationDidBecomeActive");
}

void NativeDelegateIos::applicationDidResignActive()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateIos::applicationDidResignActive");
}

void NativeDelegateIos::applicationWillEnterForeground()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateIos::applicationWillEnterForeground");
}

void NativeDelegateIos::applicationDidEnterBackground()
{
    DAVA::Logger::Debug("NativeDelegateIos::applicationDidEnterBackground");
}

void NativeDelegateIos::applicationWillTerminate()
{
    DAVA::Logger::Debug("NativeDelegateIos::applicationWillTerminate");
}

void NativeDelegateIos::didActivateNotification(UILocalNotification* notification)
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        const DAVA::String& uidStr = DAVA::StringFromNSString(uid);
        mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
    }
    DAVA::Logger::Debug("NativeDelegateIos::didActivateNotification");
}
#endif // __DAVAENGINE_MACOS__
