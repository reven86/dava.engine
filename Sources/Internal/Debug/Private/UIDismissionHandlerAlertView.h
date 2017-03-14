#if !defined(__DAVAENGINE_COREV2__)

#import <Foundation/Foundation.h>

typedef void (^AlertDismissedHandler)(NSInteger selectedIndex, BOOL didCancel);

@interface UIDismissionHandlerAlertView : NSObject

// Init the UI Alert View.
- (id)initWithTitle:(NSString*)aTitle message:(NSString*)aMessage cancelButtonTitle:(NSString*)aCancelTitle
  otherButtonTitles:(NSString*)otherTitles, ... NS_REQUIRES_NIL_TERMINATION;

// Show the UI Alert View with the dismission handler passed.
- (void)showWithDismissHandler:(AlertDismissedHandler)handler;

// Get the current message displayed.
- (NSString*)getMessage;

// Dismiss the alert view.
- (void)dismiss:(BOOL)isAnimated;

@end

#endif //!defined(__DAVAENGINE_COREV2__)
