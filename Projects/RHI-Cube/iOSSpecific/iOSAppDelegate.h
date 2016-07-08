#import "Platform/TemplateiOS/HelperAppDelegate.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>

@interface iOSAppDelegate : HelperAppDelegate
{
    UIWindow* window;
}

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
