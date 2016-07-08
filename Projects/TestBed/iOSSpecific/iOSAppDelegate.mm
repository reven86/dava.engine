#import "iOSAppDelegate.h"

#if defined(__DAVAENGINE_IPHONE__)

@implementation iOSAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(UIApplication*)application
{
    window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    [window makeKeyAndVisible];
    window.backgroundColor = [UIColor redColor];

    [super applicationDidFinishLaunching:application];
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    [window makeKeyAndVisible];
    window.backgroundColor = [UIColor redColor];
    
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 80000
    // The following line must only run under iOS 8. This runtime check prevents
    // it from running if it doesn't exist (such as running under iOS 7 or earlier).

    if ([application respondsToSelector:@selector(registerUserNotificationSettings:)])
    {
        [application registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert | UIUserNotificationTypeBadge | UIUserNotificationTypeSound categories:nil]];
    }
    
#endif

    return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

- (void)dealloc
{
    [window release];
    [super dealloc];
}

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
