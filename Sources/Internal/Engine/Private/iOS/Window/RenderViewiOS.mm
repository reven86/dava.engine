#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/RenderViewiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

#import <UIKit/UIScreen.h>
#import <QuartzCore/CAEAGLLayer.h>

@implementation RenderView

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame andBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        bridge = nativeBridge;

        [self setAutoresizingMask:UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
        [self setAutoresizesSubviews:YES];
        [self setMultipleTouchEnabled:YES];
    }
    return self;
}

- (void)setSurfaceScale:(DAVA::float32)surfaceScale
{
    [self setContentScaleFactor:[[UIScreen mainScreen] scale] * surfaceScale];
}

- (DAVA::float32)surfaceScale
{
    return [self contentScaleFactor] / [[UIScreen mainScreen] scale];
}

- (CGSize)surfaceSize
{
    CGSize size = [self frame].size;
    CGFloat scaleFactor = [self contentScaleFactor];
    return CGSizeMake(size.width * scaleFactor, size.height * scaleFactor);
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    bridge->TouchesBegan(touches);
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    bridge->TouchesMoved(touches);
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    bridge->TouchesEnded(touches);
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self touchesEnded:touches withEvent:event];
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
