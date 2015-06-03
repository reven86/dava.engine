/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "BackgroundView.h"

#if defined(__DAVAENGINE_IPHONE__)

#import "Platform/TemplateiOS/NativeViewPool.h"

@implementation BackgroundView
{
    DAVA::NativeViewPool<UIWebView> webViewPool;
    DAVA::NativeViewPool<UITextFieldHolder> textFieldPool;
}

- (UIView *) PrepareView: (UIView *)view
{
    if([view superview] == nil)
    {
        [self addSubview:view];
    }

    [view setHidden:YES];
    return view;
}


- (UIWebView *) CreateWebView
{
    return  (UIWebView *) [self PrepareView:webViewPool.GetOrCreateView()];
}

- (UITextFieldHolder *) CreateTextField
{
    return (UITextFieldHolder *)[self PrepareView:textFieldPool.GetOrCreateView()];
}

- (void) ReleaseWebView: (UIWebView *)webView
{
    [webView setHidden:YES];
    webViewPool.ReleaseView(webView);
}

- (void) ReleaseTextField: (UITextFieldHolder *)textField
{
    [textField setHidden:YES];
    textFieldPool.ReleaseView(textField);
}


@end


#endif // __DAVAENGINE_IPHONE__