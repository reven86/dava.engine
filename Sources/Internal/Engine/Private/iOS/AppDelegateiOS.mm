#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/AppDelegateiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

namespace DAVA
{
namespace Private
{
extern CoreNativeBridge* coreNativeBridge;
}
}

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application willFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    bridge = DAVA::Private::coreNativeBridge;
    return bridge->ApplicationWillFinishLaunchingWithOptions(launchOptions) ? YES : NO;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    return bridge->ApplicationDidFinishLaunchingWithOptions(application, launchOptions) ? YES : NO;
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    bridge->ApplicationDidBecomeActive();
}

- (void)applicationWillResignActive:(UIApplication*)application
{
    bridge->ApplicationWillResignActive();
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
    bridge->ApplicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
    bridge->ApplicationWillEnterForeground();
}

- (void)applicationWillTerminate:(UIApplication*)application
{
    bridge->ApplicationWillTerminate();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application
{
    bridge->ApplicationDidReceiveMemoryWarning();
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
