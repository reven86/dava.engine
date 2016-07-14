#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/RenderViewiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

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

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    bridge->touchesBegan(touches);
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    bridge->touchesMoved(touches);
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    bridge->touchesEnded(touches);
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self touchesEnded:touches withEvent:event];
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
