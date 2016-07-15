#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/OsXWindowDelegate.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

@implementation OsXWindowDelegate

- (id)init:(DAVA::Private::WindowNativeBridgeOsX*)nativeBridge
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
}

- (void)windowDidEndLiveResize:(NSNotification*)notification
{
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

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
