#import "Platform/TemplateiOS/HelperAppDelegate.h"
#import "DAVAEngine.h"
#import "GameCore.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>

@interface iOSAppDelegate : HelperAppDelegate
{
    UIWindow* window;
}

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
