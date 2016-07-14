#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/AppDelegateiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

namespace DAVA
{
namespace Private
{
extern CoreNativeBridge* coreNativeBridge;
}
}

@implementation AppDelegateiOS

- (BOOL)application:(UIApplication*)application willFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    bridge = DAVA::Private::coreNativeBridge;
    return bridge->applicationWillFinishLaunchingWithOptions(launchOptions) ? YES : NO;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    return bridge->applicationDidFinishLaunchingWithOptions(launchOptions) ? YES : NO;
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    bridge->applicationDidBecomeActive();
}

- (void)applicationWillResignActive:(UIApplication*)application
{
    bridge->applicationWillResignActive();
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
    bridge->applicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
    bridge->applicationWillEnterForeground();
}

- (void)applicationWillTerminate:(UIApplication*)application
{
    bridge->applicationWillTerminate();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application
{
    bridge->applicationDidReceiveMemoryWarning();
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
