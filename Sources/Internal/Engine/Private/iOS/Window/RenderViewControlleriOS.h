#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIViewController.h>

#include "Engine/Private/EnginePrivateFwd.h"

@interface RenderViewController : UIViewController
{
    DAVA::Private::WindowNativeBridge* bridge;
}

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
