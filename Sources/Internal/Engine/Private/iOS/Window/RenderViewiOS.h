#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIView.h>

#include "Engine/Private/EnginePrivateFwd.h"

@interface RenderView : UIView
{
    DAVA::Private::WindowNativeBridge* bridge;
}

+ (Class)layerClass;

- (id)initWithFrame:(CGRect)frame andBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
