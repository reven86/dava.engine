#if !defined(__DAVAENGINE_COREV2__)

#import <UIKit/UIKit.h>

@interface UIAlertView (Modal)<UIAlertViewDelegate>

- (NSInteger)showModal;
- (void)alertView:(UIAlertView*)alertView clickedButtonAtIndex:(NSInteger)buttonIndex;
- (NSInteger)getClickedButtonIndex;

@end

#endif //!defined(__DAVAENGINE_COREV2__)
