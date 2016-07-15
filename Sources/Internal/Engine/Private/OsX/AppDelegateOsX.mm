#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/AppDelegateOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"

@implementation OsXAppDelegate

- (id)init:(DAVA::Private::CoreNativeBridgeOsX*)objcBridge
{
    self = [super init];
    if (self != nullptr)
    {
        bridge = objcBridge;
    }
    return self;
}

- (void)applicationWillFinishLaunching:(NSNotification*)notification
{
    bridge->ApplicationWillFinishLaunching();
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    bridge->ApplicationDidFinishLaunching();
}

- (void)applicationDidChangeScreenParameters:(NSNotification*)notification
{
    bridge->ApplicationDidChangeScreenParameters();
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    bridge->ApplicationDidBecomeActive();
}

- (void)applicationDidResignActive:(NSNotification*)notification
{
    bridge->ApplicationDidResignActive();
}

- (void)applicationDidHide:(NSNotification*)notification
{
    bridge->ApplicationDidHide();
}

- (void)applicationDidUnhide:(NSNotification*)notification
{
    bridge->ApplicationDidUnhide();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    bool r = bridge->ApplicationShouldTerminate();
    return r ? NSTerminateNow : NSTerminateCancel;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    bool r = bridge->ApplicationShouldTerminateAfterLastWindowClosed();
    return r ? YES : NO;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    bridge->ApplicationWillTerminate();
}

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
