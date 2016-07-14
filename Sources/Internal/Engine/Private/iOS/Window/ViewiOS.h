#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIView.h>

#include "Engine/Private/EnginePrivateFwd.h"

@interface ViewiOS : UIView
{
    DAVA::Private::WindowNativeBridgeiOS* bridge;
}

+ (Class)layerClass;

- (id)initWithFrame:(CGRect)frame andBridge:(DAVA::Private::WindowNativeBridgeiOS*)nativeBridge;

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
