#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>

@class UIView;

@interface NativeViewPool : NSObject
{
    DAVA::Vector<std::pair<UIView*, bool>> pool;
}

- (UIView*)queryView:(NSString*)viewClassName;
- (void)returnView:(UIView*)view;

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
