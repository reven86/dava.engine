#if !defined(__DAVAENGINE_COREV2__)

#import "Platform/TemplateiOS/HelperAppDelegate.h"
#import <DAVAEngine.h>
#import "Infrastructure/GameCore.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>

@interface iOSAppDelegate : HelperAppDelegate
{
    UIWindow* window;
}

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
#endif //!__DAVAENGINE_COREV2__
