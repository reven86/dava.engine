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

    return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

- (void)dealloc
{
    [window release];
    [super dealloc];
}

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
