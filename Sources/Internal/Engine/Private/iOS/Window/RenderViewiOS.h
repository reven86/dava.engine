#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

#import <UIKit/UIView.h>

@interface RenderView : UIView
{
    DAVA::Private::WindowNativeBridge* bridge;
}

+ (Class)layerClass;

- (id)initWithFrame:(CGRect)frame andBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;

- (void)setSurfaceScale:(DAVA::float32)surfaceScale;
- (DAVA::float32)surfaceScale;
- (CGSize)surfaceSize;

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
