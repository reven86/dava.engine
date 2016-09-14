#pragma once

#if !defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>
#import "UI/Private/iOS/UITextFieldHolder.h"

@interface BackgroundView : UIView

- (UIWebView*)CreateWebView;
- (void)ReleaseWebView:(UIWebView*)webView;

- (UITextFieldHolder*)CreateTextField;
- (void)ReleaseTextField:(UITextFieldHolder*)textField;
- (UIView*)PrepareView:(UIView*)view;

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)
#endif // !__DAVAENGINE_COREV2__
