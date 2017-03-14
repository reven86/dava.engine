#import "UIDismissionHandlerAlertView.h"

#if !defined(__DAVAENGINE_COREV2__)

#import <UIKit/UIKit.h>

@interface UIDismissionHandlerAlertView ()<UIAlertViewDelegate>

@property(retain, nonatomic) UIDismissionHandlerAlertView* alertReference;
@property(copy) AlertDismissedHandler activeDismissHandler;
@property(retain, nonatomic) UIAlertView* activeAlert;

@end

@implementation UIDismissionHandlerAlertView

- (id)initWithTitle:(NSString*)aTitle message:(NSString*)aMessage cancelButtonTitle:(NSString*)aCancelTitle otherButtonTitles:(NSString*)otherTitles, ...
{
    self = [super init];
    if (self)
    {
        UIAlertView* alert = [[UIAlertView alloc] initWithTitle:aTitle message:aMessage delegate:self cancelButtonTitle:aCancelTitle otherButtonTitles:nil];
        if (otherTitles != nil)
        {
            [alert addButtonWithTitle:otherTitles];
            va_list args;
            va_start(args, otherTitles);
            NSString* title = nil;
            while ((title = va_arg(args, NSString*)))
            {
                [alert addButtonWithTitle:title];
            }

            va_end(args);
        }

        self.activeAlert = alert;
    }

    return self;
}

- (void)showWithDismissHandler:(AlertDismissedHandler)handler
{
    self.activeDismissHandler = handler;
    self.alertReference = self;
    [self.activeAlert show];
}

- (void)alertView:(UIAlertView*)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (self.activeDismissHandler)
    {
        self.activeDismissHandler(buttonIndex, buttonIndex == alertView.cancelButtonIndex);
    }

    self.activeAlert = nil;
    self.alertReference = nil;
}

- (NSString*)getMessage
{
    if (self.activeAlert)
    {
        return self.activeAlert.message;
    }

    return nil;
}

- (void)dismiss:(BOOL)isAnimated
{
    if (self.activeAlert)
    {
        [self.activeAlert dismissWithClickedButtonIndex:0 animated:isAnimated];
        self.activeAlert = nil;
        self.alertReference = nil;
    }
}

@end

#endif // !defined(__DAVAENGINE_COREV2__)
