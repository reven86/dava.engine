#ifndef __DAVAENGINE_HELPER_APP_DELEGATE_H__
#define __DAVAENGINE_HELPER_APP_DELEGATE_H__


#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_IPHONE__)


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "UI/UIScreenManager.h"
#import "Platform/TemplateiOS/RenderViewController.h"

enum
{
    CONTROLLER_GL = 10000,
};

@interface HelperAppDelegate : NSObject<UIApplicationDelegate>
{
    RenderViewController* renderViewController;
}

@property(nonatomic, retain) RenderViewController* renderViewController;

@end

#endif //__DAVAENGINE_IPHONE__
#endif //__DAVAENGINE_HELPER_APP_DELEGATE_H__
