/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "WebViewControliOS.h"
#include "DAVAEngine.h"

#import <UIKit/UIKit.h>
#import <HelperAppDelegate.h>

@interface WebViewURLDelegate : NSObject<UIWebViewDelegate>
{
	DAVA::IUIWebViewDelegate* delegate;
	DAVA::UIWebView* webView;
}

- (id)init;

- (void)setDelegate:(DAVA::IUIWebViewDelegate*)d andWebView:(DAVA::UIWebView*)w;

- (BOOL)webView: (UIWebView*)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType;

- (void)webViewDidFinishLoad:(UIWebView *)webView;

@end

@implementation WebViewURLDelegate

- (id)init
{
	self = [super init];
	if (self)
	{
		delegate = NULL;
		webView = NULL;
	}
	return self;
}

- (void)setDelegate:(DAVA::IUIWebViewDelegate *)d andWebView:(DAVA::UIWebView *)w
{
	if (d && w)
	{
		delegate = d;
		webView = w;
	}
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
	BOOL process = YES;
	
	if (delegate && self->webView)
	{
		NSString* url = [[request URL] absoluteString];
		
		if (url)
		{
		    bool isRedirectedByMouseClick = navigationType == UIWebViewNavigationTypeLinkClicked;
			DAVA::IUIWebViewDelegate::eAction action = delegate->URLChanged(self->webView, [url UTF8String], isRedirectedByMouseClick);
			
			switch (action) {
				case DAVA::IUIWebViewDelegate::PROCESS_IN_WEBVIEW:
					DAVA::Logger::Debug("PROCESS_IN_WEBVIEW");
					process = YES;
					break;
					
				case DAVA::IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					DAVA::Logger::Debug("PROCESS_IN_SYSTEM_BROWSER");
					[[UIApplication sharedApplication] openURL:[request URL]];
					process = NO;
					break;
					
				case DAVA::IUIWebViewDelegate::NO_PROCESS:
					DAVA::Logger::Debug("NO_PROCESS");
					
				default:
					process = NO;
					break;
			}
		}
	}
	
	return process;
}


- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
    }
}
@end


namespace DAVA
{
	typedef DAVA::UIWebView DAVAWebView;

	//Use unqualified UIWebView and UIScreen from global namespace, i.e. from UIKit
	using ::UIWebView;
	using ::UIScreen;

WebViewControl::WebViewControl()
{
	CGRect emptyRect = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	webViewPtr = [[UIWebView alloc] initWithFrame:emptyRect];
	SetBounces(false);

	UIWebView* localWebView = (UIWebView*)webViewPtr;
	HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
	[[[appDelegate glController] view] addSubview:localWebView];

	webViewURLDelegatePtr = [[WebViewURLDelegate alloc] init];
	[localWebView setDelegate:(WebViewURLDelegate*)webViewURLDelegatePtr];

	[localWebView becomeFirstResponder];
}

WebViewControl::~WebViewControl()
{
	UIWebView* innerWebView = (UIWebView*)webViewPtr;

	[innerWebView removeFromSuperview];
	[innerWebView release];
	webViewPtr = nil;

	WebViewURLDelegate* w = (WebViewURLDelegate*)webViewURLDelegatePtr;
	[w release];
	webViewURLDelegatePtr = nil;

	RestoreSubviewImages();
}
	
void WebViewControl::SetDelegate(IUIWebViewDelegate *delegate, DAVAWebView* webView)
{
	WebViewURLDelegate* w = (WebViewURLDelegate*)webViewURLDelegatePtr;
	[w setDelegate:delegate andWebView:webView];
}

void WebViewControl::Initialize(const Rect& rect)
{
	SetRect(rect);
}

// Open the URL requested.
void WebViewControl::OpenURL(const String& urlToOpen)
{
	NSString* nsURLPathToOpen = [NSString stringWithUTF8String:urlToOpen.c_str()];
	NSURL* url = [NSURL URLWithString:[nsURLPathToOpen stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding]];
	
	NSURLRequest* requestObj = [NSURLRequest requestWithURL:url];
	[(UIWebView*)webViewPtr loadRequest:requestObj];
}

void WebViewControl::LoadHtmlString(const String& htlmString)
{
	NSString *htmlPageToLoad = [NSString stringWithUTF8String:htlmString.c_str()];
    [(UIWebView*)webViewPtr loadHTMLString:htmlPageToLoad baseURL:nil];
}

void WebViewControl::SetRect(const Rect& rect)
{
	CGRect webViewRect = [(UIWebView*)webViewPtr frame];

	Core::eScreenOrientation screenOrientation = Core::Instance()->GetScreenOrientation();
	switch (screenOrientation)
	{
		case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
		{
			// X and Y are swapped in this case.
			webViewRect.origin.y = (DAVA::Core::Instance()->GetVirtualScreenXMax() - rect.x - rect.dx) * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.origin.x = rect.y * DAVA::Core::GetVirtualToPhysicalFactor();
			
			webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().y;
			webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().x;

			// Height and width are swapped in this case,
			webViewRect.size.width = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.size.height = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
			
			((UIWebView*)webViewPtr).transform = CGAffineTransformMakeRotation(DAVA::DegToRad(-90.0f));
			break;
		}

		case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
		{
			// X and Y are swapped in this case.
			webViewRect.origin.y = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.origin.x = (DAVA::Core::Instance()->GetVirtualScreenYMax() - rect.y - rect.dy) * DAVA::Core::GetVirtualToPhysicalFactor();
			
			// Height and width are swapped in this case,
			webViewRect.size.width = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.size.height = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
			
			((UIWebView*)webViewPtr).transform = CGAffineTransformMakeRotation(DAVA::DegToRad(90.0f));
			break;
		}

		case Core::SCREEN_ORIENTATION_PORTRAIT:
		case Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
		{
			// Minimum recalculations are needed, no swapping, no rotation.
			webViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.origin.y = rect.y * DAVA::Core::GetVirtualToPhysicalFactor();
			
			webViewRect.size.width = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.size.height = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();

			webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
			webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;

			break;
		}

		default:
		{
			break;
		}
	}
	
	// Apply the Retina scale divider, if any.
	float scaleDivider = GetScaleDivider();
	webViewRect.origin.x /= scaleDivider;
	webViewRect.origin.y /= scaleDivider;
	webViewRect.size.height /= scaleDivider;
	webViewRect.size.width /= scaleDivider;

	[(UIWebView*)webViewPtr setFrame: webViewRect];
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
	[(UIWebView*)webViewPtr setHidden:!isVisible];
}

void WebViewControl::SetScalesPageToFit(bool isScalesToFit)
{
	[(UIWebView*)webViewPtr setScalesPageToFit:isScalesToFit];
}

float WebViewControl::GetScaleDivider()
{
    float scaleDivider = 1.f;
    if (DAVA::Core::IsAutodetectContentScaleFactor())
    {
        if ([UIScreen instancesRespondToSelector: @selector(scale) ]
            && [UIView instancesRespondToSelector: @selector(contentScaleFactor) ])
        {
            scaleDivider = [[UIScreen mainScreen] scale];
        }
	}

	return scaleDivider;
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
	UIWebView* webView = (UIWebView*)webViewPtr;
	[webView setOpaque: (enabled ? NO : YES)];

	UIColor* color = [webView backgroundColor];
	CGFloat r, g, b, a;
	[color getRed:&r green:&g blue:&b alpha:&a];

	if (enabled)
	{
		[webView setBackgroundColor:[UIColor colorWithRed:r green:g blue:b alpha:0.f]];
		HideSubviewImages(webView);
	}
	else
	{
		[webView setBackgroundColor:[UIColor colorWithRed:r green:g blue:b alpha:1.0f]];
		RestoreSubviewImages();
	}
}

void WebViewControl::HideSubviewImages(void* view)
{
	UIView* uiview = (UIView*)view;
	for (UIView* subview in [uiview subviews])
	{
		if ([subview isKindOfClass:[UIImageView class]])
		{
			subviewVisibilityMap[subview] = [subview isHidden];
			[subview setHidden:YES];
			[subview retain];
		}
		HideSubviewImages(subview);
	}
}

void WebViewControl::RestoreSubviewImages()
{
	Map<void*, bool>::iterator it;
	for (it = subviewVisibilityMap.begin(); it != subviewVisibilityMap.end(); ++it)
	{
		UIView* view = (UIView*)it->first;
		[view setHidden:it->second];
		[view release];
	}
	subviewVisibilityMap.clear();
}

bool WebViewControl::GetBounces() const
{
	if (!webViewPtr)
	{
		return false;
	}

	UIWebView* localWebView = (UIWebView*)webViewPtr;
	return (localWebView.scrollView.bounces == YES);
}
	
void WebViewControl::SetBounces(bool value)
{
	UIWebView* localWebView = (UIWebView*)webViewPtr;
	localWebView.scrollView.bounces = (value == true);
}
    
};