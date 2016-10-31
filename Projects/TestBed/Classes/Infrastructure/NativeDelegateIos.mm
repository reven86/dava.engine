#include "Infrastructure/NativeDelegateIos.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>

#include <Logger/Logger.h>
#include <Utils/NSStringUtils.h>

void NativeDelegateIos::didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions)
{
    using namespace DAVA;
    Logger::Debug("TestBed.NativeDelegateIos::didFinishLaunchingWithOptions: enter");
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
    DAVA::Logger::Debug("TestBed.NativeDelegateIos::applicationDidEnterBackground");
}

void NativeDelegateIos::applicationWillTerminate()
{
    DAVA::Logger::Debug("TestBed.NativeDelegateIos::applicationWillTerminate");
}

#endif // __DAVAENGINE_MACOS__
