#import "UI/UIScreenManageriPhoneImpl.h"

#if defined(__DAVAENGINE_IPHONE__)

@interface ScreenManagerImpl ()
- (void)notimerSetViewController:(UIViewController*)viewController;
@end

@implementation ScreenManagerImpl

static ScreenManagerImpl* staticInstance = 0;
+ (ScreenManagerImpl*)instance;
{
    if (!staticInstance)
    {
        staticInstance = [[ScreenManagerImpl alloc] init];
    }
    return staticInstance;
}

- (id)init
{
    if (self = [super init])
    {
        appWindow = [[UIApplication sharedApplication] keyWindow];
        inTransition = NO;
        lastActiveController = 0;
        NSLog(@"[ScreenTransitionManager] create appWindow:%@", appWindow);
    }
    return self;
}

//
- (void)applicationLaunched:(UIViewController*)firstController;
{
    [self notimerSetViewController:firstController];
}

- (void)dealloc
{
    NSLog(@"[ScreenTransitionManager] release");

    [super dealloc];
}

- (void)notimerSetViewController:(UIViewController*)viewController
{
    //NSLog(@"Start Switching\n");
    if (inTransition)
        return;
    //NSLog(@"Continue Switching\n");

    if (nil != activeController)
    {
        //[activeController screenWillDisappear];
        [activeController.view removeFromSuperview];
        activeController = nil;
    }

    if (viewController)
    {
        activeController = viewController;
        //[activeController screenWillAppear];
        [appWindow addSubview:activeController.view];
    }

    if (applyAnimationTransition)
    {
        [UIView beginAnimations:nil context:NULL];
        [UIView setAnimationDuration:0.75];
        [UIView setAnimationTransition:transition forView:appWindow cache:YES];
        [UIView setAnimationDelegate:self];
        [UIView setAnimationDidStopSelector:@selector(animationDidStop:finished:context:)];
        [self animationDidStart:nil];
    }

    if (applyAnimationTransition2)
    {
        //[self recursiveRemoveAnimations: mainView];

        // Set up the animation
        CATransition* animation = [CATransition animation];
        [animation setDelegate:self];

        [animation setType:type];
        [animation setSubtype:subtype];
        [animation setStartProgress:0.0f];
        [animation setEndProgress:1.0f];

        // Set the duration and timing function of the transtion -- duration is passed in as a parameter, use ease in/ease out as the timing function
        [animation setDuration:0.75f];
        [animation setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];

        [[appWindow layer] addAnimation:animation forKey:@"TransferAnimation"];

        applyAnimationTransition2 = FALSE;
    }

    if (applyAnimationTransition)
    {
        [UIView commitAnimations];
        applyAnimationTransition = FALSE;
    }
    //NSLog(@"Switched\n");
}

- (void)setViewControllerByTimer:(NSTimer*)timer
{
    UIViewController* viewController = (UIViewController*)timer.userInfo;
    [timer invalidate];
    [self notimerSetViewController:viewController];
}

- (void)setViewController:(UIViewController*)viewController
{
    [NSTimer scheduledTimerWithTimeInterval:0.03 target:self selector:@selector(setViewControllerByTimer:) userInfo:viewController repeats:NO];
}

/*- (void) startLoading
{
	[[UIApplication sharedApplication] beginIgnoringInteractionEvents];
	
	// trigger animation here
}

- (void) stopLoading
{
	// stop animation here
	
	[[UIApplication sharedApplication] endIgnoringInteractionEvents];
} */

//- (void)showPopupController: (UIViewController*)viewController
//{
//	//NSLog(@"[AppScreens] View controller retain count: %d\n", [viewController retainCount]);
//	if (activeController)
//	{
//		UIView * popupView = optionsController.view;
//		[activeController.view addSubview:popupView];
//
//		popupView.alpha = 0.0;
//		[UIView beginAnimations:nil context:nil];
//		[UIView setAnimationDuration: 0.8];
//		popupView.alpha = 1.0;
//		[UIView commitAnimations];
//	}
//}

- (void)animationDidStart:(CAAnimation*)animation
{
    //NSLog(@"Begin ignoring\n");
    [[UIApplication sharedApplication] beginIgnoringInteractionEvents];
    inTransition = YES;
}

- (void)animationDidStop:(CAAnimation*)animation finished:(BOOL)finished
{
    //NSLog(@"End ignoring\n");
    inTransition = NO;
    [[UIApplication sharedApplication] endIgnoringInteractionEvents];
}

- (void)animationDidStop:(NSString*)animationID finished:(NSNumber*)finished context:(void*)context
{
    [self animationDidStop:nil finished:YES];
}

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
