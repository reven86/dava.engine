#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

@interface ScreenManagerImpl : NSObject
{
@private

    UIViewController* loadingViewController;

    UIWindow* appWindow;
    UIViewController* activeController;

    bool applyAnimationTransition;
    UIViewAnimationTransition transition;
    bool applyAnimationTransition2;
    NSString* type;
    NSString* subtype;
    bool inTransition;
    // End of screen switching data

    UIViewController* lastActiveController;
}

//
+ (ScreenManagerImpl*)instance;

- (void)applicationLaunched:(UIViewController*)firstController;
- (void)setViewController:(UIViewController*)viewController;

@end

#endif // #include "BaseTypes.h"
