#include "BackgroundView.h"

#if defined(__DAVAENGINE_IPHONE__)

#import "Platform/TemplateiOS/NativeViewPool.h"

@implementation BackgroundView
{
    DAVA::NativeViewPool<UIWebView> webViewPool;
    DAVA::NativeViewPool<UITextFieldHolder> textFieldPool;
}

- (UIView*)PrepareView:(UIView*)view
{
    if ([view superview] == nil)
    {
        [self addSubview:view];
    }

    [view setHidden:YES];
    return view;
}

- (UIWebView*)CreateWebView
{
    return (UIWebView*)[self PrepareView:webViewPool.GetOrCreateView()];
}

- (UITextFieldHolder*)CreateTextField
{
    return (UITextFieldHolder*)[self PrepareView:textFieldPool.GetOrCreateView()];
}

- (void)ReleaseWebView:(UIWebView*)webView
{
    [webView setHidden:YES];
    webViewPool.ReleaseView(webView);
}

- (void)ReleaseTextField:(UITextFieldHolder*)textField
{
    [textField setHidden:YES];
    textFieldPool.ReleaseView(textField);
}

@end


#endif // __DAVAENGINE_IPHONE__