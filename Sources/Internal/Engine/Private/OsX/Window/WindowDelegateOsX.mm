#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/WindowDelegateOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

@implementation WindowDelegate

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nullptr)
    {
        bridge = nativeBridge;
    }
    return self;
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
    bridge->WindowDidMiniaturize();
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
    bridge->WindowDidDeminiaturize();
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    bridge->WindowDidBecomeKey();
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    bridge->WindowDidResignKey();
}

- (void)windowDidResize:(NSNotification*)notification
{
    bridge->WindowDidResize();
}

- (void)windowWillStartLiveResize:(NSNotification*)notification
{
    bridge->WindowWillStartLiveResize();
}

- (void)windowDidEndLiveResize:(NSNotification*)notification
{
    bridge->WindowDidEndLiveResize();
}

- (void)windowDidChangeScreen:(NSNotification*)notification
{
    bridge->WindowDidChangeScreen();
}

- (BOOL)windowShouldClose:(id)sender
{
    return bridge->WindowShouldClose() ? YES : NO;
}

- (void)windowWillClose:(NSNotification*)notification
{
    bridge->WindowWillClose();
}

- (void)windowWillEnterFullScreen:(NSNotification*)notification
{
    bridge->WindowWillEnterFullScreen();
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification
{
    bridge->WindowDidEnterFullScreen();
}

- (void)windowWillExitFullScreen:(NSNotification*)notification
{
    bridge->WindowWillExitFullScreen();
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
    bridge->WindowDidExitFullScreen();
}

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
