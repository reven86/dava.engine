#import <UIKit/UIKit.h>

@interface UIAlertView (Modal)<UIAlertViewDelegate>

- (NSInteger)showModal;
- (void)alertView:(UIAlertView*)alertView clickedButtonAtIndex:(NSInteger)buttonIndex;
- (NSInteger)getClickedButtonIndex;

@end
