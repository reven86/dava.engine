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


#include "WebViewControliOS.h"
#include "DAVAEngine.h"

#import <UIKit/UIKit.h>
#import <Platform/TemplateiOS/HelperAppDelegate.h>
#import "Platform/TemplateiOS/BackgroundView.h"

@interface WebViewURLDelegate : NSObject<UIWebViewDelegate>
{
	DAVA::IUIWebViewDelegate* delegate;
	DAVA::UIWebView* webView;
    DAVA::WebViewControl* webViewControl;
}

- (id)init;

- (void)setDelegate:(DAVA::IUIWebViewDelegate*)d andWebView:(DAVA::UIWebView*)w;

- (BOOL)webView: (UIWebView*)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType;

- (void)webViewDidFinishLoad:(UIWebView *)webView;
- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error;
- (void)leftGesture;
- (void)rightGesture;
- (UIImage *)renderUIViewToImage:(UIView *)view;
- (void)setDAVAUIWebView:(DAVA::UIWebView*) uiWebControl;
- (void)onExecuteJScript:(NSArray *)result;
- (void)setDAVAWebViewControl:(DAVA::WebViewControl*) webViewControl;
@end

@implementation WebViewURLDelegate

- (id)init
{
	self = [super init];
	if (self)
	{
		delegate = nullptr;
		webView = nullptr;
        webViewControl = nullptr;
	}
	return self;
}

- (void)leftGesture
{
    if (delegate)
    {
        delegate->SwipeGesture(true);
    }
}

- (void)rightGesture
{
    if (delegate)
    {
        delegate->SwipeGesture(false);
    }
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
					DAVA::Logger::FrameworkDebug("PROCESS_IN_WEBVIEW");
					process = YES;
					break;
					
				case DAVA::IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					DAVA::Logger::FrameworkDebug("PROCESS_IN_SYSTEM_BROWSER");
					[[UIApplication sharedApplication] openURL:[request URL]];
					process = NO;
					break;
					
				case DAVA::IUIWebViewDelegate::NO_PROCESS:
					DAVA::Logger::FrameworkDebug("NO_PROCESS");
					
				default:
					process = NO;
					break;
			}
		}
	}
	
	return process;
}


- (void)webViewDidFinishLoad:(UIWebView *)webViewParam
{
    DVASSERT(webViewControl);
    DVASSERT(webView);
    
    if (webViewControl->IsRenderToTexture())
    {
        webViewControl->RenderToTextureAndSetAsBackgroundSpriteToControl(
                                                                    *webView);
    }
    
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
    }
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
    DAVA::Logger::Instance()->Error("WebView error: %s", [[error description] UTF8String]);
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
    }
}

- (UIImage *)renderUIViewToImage:(UIView *)view
{
    void* image = DAVA::WebViewControl::RenderIOSUIViewToImage(view);
    DVASSERT(image);
    UIImage* uiImage = static_cast<UIImage*>(image);
    return uiImage;
}

- (void)onExecuteJScript:(NSString *)result
{
    if (delegate)
    {
        delegate->OnExecuteJScript(webView, DAVA::String([result UTF8String]));
    }
}

- (void)setDAVAUIWebView:(DAVA::UIWebView*) uiWebControl
{
    DVASSERT(uiWebControl);
    webView = uiWebControl;
}

- (void)setDAVAWebViewControl:(DAVA::WebViewControl*) control
{
    DVASSERT(control);
    webViewControl = control;
}

@end

DAVA::WebViewControl::WebViewControl(DAVA::UIWebView& uiWeb)
    : webViewPtr(0)
    , webViewURLDelegatePtr(0)
    , rightSwipeGesturePtr(0)
    , leftSwipeGesturePtr(0)
    , gesturesEnabled(false)
    , isRenderToTexture(false)
    , pendingRenderToTexture(false)
    , isVisible(true)
    , uiWebView(uiWeb)
{
    HelperAppDelegate* appDelegate = [[UIApplication sharedApplication]
                                                                    delegate];
    BackgroundView* backgroundView = [appDelegate glController].backgroundView;
    
    ::UIWebView* localWebView = [backgroundView CreateWebView];
    webViewPtr = localWebView;
    
    CGRect emptyRect = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
    [localWebView setFrame:emptyRect];
    
    SetBounces(false);
    
    webViewURLDelegatePtr = [[WebViewURLDelegate alloc] init];
    WebViewURLDelegate* viewURLDelegate =
                            (WebViewURLDelegate*)webViewURLDelegatePtr;
    [localWebView setDelegate:viewURLDelegate];
    [viewURLDelegate setDAVAUIWebView:&uiWebView];
    [viewURLDelegate setDAVAWebViewControl:this];
    
    [localWebView becomeFirstResponder];
}

void* DAVA::WebViewControl::RenderIOSUIViewToImage(void* uiviewPtr)
{
    ::UIView* view = static_cast<::UIView*>(uiviewPtr);
    DVASSERT(view);
    
    size_t w = view.frame.size.width;
    size_t h = view.frame.size.height;
    
    if (w == 0 || h == 0)
    {
        return nullptr; // empty rect on start, just skip it
    }
    
    // Workaround! render text view directly without scrolling
    if ([::UITextView class] == [view class])
    {
        ::UITextView* textView = (::UITextView*)view;
        view = textView.textInputView;
    }
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(w, h), NO, 0);
    // Workaround! iOS bug see http://stackoverflow.com/questions/23157653/drawviewhierarchyinrectafterscreenupdates-delays-other-animations
    [view.layer renderInContext:UIGraphicsGetCurrentContext()];
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    DVASSERT(image);
    return image;
}

void DAVA::WebViewControl::SetImageAsSpriteToControl(void* imagePtr, UIControl& control)
{
    ::UIImage* image = static_cast<::UIImage*>(imagePtr);
    DVASSERT(image);
    // copy image into our buffer with bitmap context
    // TODO create fucntion static void copyImageToTexture(UIControl& control, ::UIImage* image
    CGImageRef imageRef = [image CGImage];
    DAVA::int32 width = static_cast<DAVA::int32>(CGImageGetWidth(imageRef));
    DAVA::int32 height = static_cast<DAVA::int32>(CGImageGetHeight(imageRef));
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    {
        DAVA::Image* imageRGB = DAVA::Image::Create(width, height,
                                                    DAVA::FORMAT_RGBA8888);
        DVASSERT(imageRGB);
        
        DAVA::uint8 *rawData = imageRGB->GetData();
        
        NSUInteger bytesPerPixel = 4;
        NSUInteger bytesPerRow = bytesPerPixel * width;
        NSUInteger bitsPerComponent = 8;
        
        // this way we can copy image from system memory into our buffer
        Memset(rawData, 0, width * height * bytesPerPixel);
        
        CGContextRef context = CGBitmapContextCreate(rawData, width, height,
                                                     bitsPerComponent, bytesPerRow, colorSpace,
                                                     kCGImageAlphaPremultipliedLast
                                                     | kCGBitmapByteOrder32Big);
        CGColorSpaceRelease(colorSpace);
        
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
        CGContextRelease(context);
        
        {
            DAVA::Texture* tex = DAVA::Texture::CreateFromData(imageRGB, false);
            DVASSERT(tex);
            
            DAVA::Rect rect = control.GetRect();
            {
                DAVA::Sprite* spr = DAVA::Sprite::CreateFromTexture(tex, 0, 0, width, height, rect.dx, rect.dy);
                DVASSERT(spr);
                
                control.GetBackground()->SetSprite(spr, 0);
                DAVA::SafeRelease(spr);
            }
            DAVA::SafeRelease(tex);
        }
        DAVA::SafeRelease(imageRGB);
    }
}

void DAVA::WebViewControl::RenderToTextureAndSetAsBackgroundSpriteToControl(
                                            DAVA::UIWebView& control)
{
    WebViewURLDelegate* webURLDelegate =
                (WebViewURLDelegate*)webViewURLDelegatePtr;
    DVASSERT(webURLDelegate);
    
    ::UIWebView* iosWebView = (::UIWebView*)webViewPtr;
    DVASSERT(iosWebView);
    
    
    UIImage* image = [webURLDelegate renderUIViewToImage:iosWebView];
    DVASSERT(image);
    
    WebViewControl::SetImageAsSpriteToControl(image, control);
}

namespace DAVA
{
	typedef DAVA::UIWebView DAVAWebView;

	//Use unqualified UIWebView and UIScreen from global namespace, i.e. from UIKit
	using ::UIWebView;
	using ::UIScreen;

    static const struct
    {
        DAVAWebView::eDataDetectorType davaDetectorType;
        NSUInteger systemDetectorType;
    }
    detectorsMap[] =
    {
        {DAVAWebView::DATA_DETECTOR_ALL, UIDataDetectorTypeAll},
        {DAVAWebView::DATA_DETECTOR_NONE, UIDataDetectorTypeNone},
        {DAVAWebView::DATA_DETECTOR_PHONE_NUMBERS, UIDataDetectorTypePhoneNumber},
        {DAVAWebView::DATA_DETECTOR_LINKS, UIDataDetectorTypeLink},
        {DAVAWebView::DATA_DETECTOR_ADDRESSES, UIDataDetectorTypeAddress},
        {DAVAWebView::DATA_DETECTOR_CALENDAR_EVENTS, UIDataDetectorTypeCalendarEvent}
    };



WebViewControl::~WebViewControl()
{
    SetGestures(NO);
	UIWebView* innerWebView = (UIWebView*)webViewPtr;

    
    [innerWebView setDelegate:nil];
    [innerWebView stopLoading];
    [innerWebView loadHTMLString:@"" baseURL:nil];
    
    [innerWebView resignFirstResponder];

    
    HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
    BackgroundView* backgroundView = [appDelegate glController].backgroundView;
    [backgroundView ReleaseWebView:innerWebView];
    
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
    UIWebView* innerWebView = (UIWebView*)webViewPtr;
    [innerWebView stopLoading];
    [innerWebView loadRequest:requestObj];
}
    
void WebViewControl::OpenFromBuffer(const String& string, const FilePath& basePath)
{
	NSString* dataToOpen = [NSString stringWithUTF8String:string.c_str()];
    NSString* baseUrl = [NSString stringWithUTF8String:basePath.AsURL().c_str()];
    
    UIWebView* innerWebView = (UIWebView*)webViewPtr;
    [innerWebView stopLoading];
    
    [innerWebView loadHTMLString:dataToOpen baseURL:[NSURL URLWithString:baseUrl]];
}

void WebViewControl::LoadHtmlString(const WideString& htlmString)
{
	NSString* htmlPageToLoad = [[[NSString alloc] initWithBytes: htlmString.data()
												   length: htlmString.size() * sizeof(wchar_t)
												 encoding:NSUTF32LittleEndianStringEncoding] autorelease];

    [(UIWebView*)webViewPtr loadHTMLString:htmlPageToLoad baseURL:nil];
}

void WebViewControl::DeleteCookies(const String& targetUrl)
{
	NSString *targetUrlString = [NSString stringWithUTF8String:targetUrl.c_str()];
	NSHTTPCookieStorage* cookies = [NSHTTPCookieStorage sharedHTTPCookieStorage];
	// Delete all cookies for specified URL
	for(NSHTTPCookie *cookie in [cookies cookies])
	{
		if([[cookie domain] rangeOfString:targetUrlString].location != NSNotFound)
	  	{
       		[cookies deleteCookie:cookie];
   	 	}
	}
	// Syncronized all changes with file system
	[[NSUserDefaults standardUserDefaults] synchronize];
}

String WebViewControl::GetCookie(const String& targetUrl, const String& name) const
{
	Map<String, String> cookiesMap = GetCookies(targetUrl);
	Map<String, String>::iterator cIter = cookiesMap.find(name);
	
	if (cIter != cookiesMap.end())
	{
		return cIter->second;
	}

	return String();
}

Map<String, String> WebViewControl::GetCookies(const String& targetUrl) const
{
	Map<String, String> resultMap;
	
	NSString *targetUrlString = [NSString stringWithUTF8String:targetUrl.c_str()];
    NSArray  *cookiesArray = [[NSHTTPCookieStorage sharedHTTPCookieStorage] cookiesForURL: [NSURL URLWithString:targetUrlString]];

	for(NSHTTPCookie * cookie in cookiesArray)
	{
		String cookieName = [[cookie name] UTF8String];
		resultMap[cookieName] = [[cookie value] UTF8String];
    }
	
	return resultMap;
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
	NSString *jScriptString = [NSString stringWithUTF8String:
                                                scriptString.c_str()];
    
	NSString *resultString = [(UIWebView*)webViewPtr
                        stringByEvaluatingJavaScriptFromString:jScriptString];

    WebViewURLDelegate* w = (WebViewURLDelegate*)webViewURLDelegatePtr;
    if (w)
    {
        [w performSelector:@selector(onExecuteJScript:)
                withObject:resultString afterDelay:0.0];
    }
}

void WebViewControl::SetRect(const Rect& rect)
{
	CGRect webViewRect = [(UIWebView*)webViewPtr frame];

    VirtualCoordinatesSystem& VCS = *VirtualCoordinatesSystem::Instance();
    
    Rect physicalRect = VCS.ConvertVirtualToPhysical(rect);
    
    webViewRect.origin.x = physicalRect.x + VCS.GetPhysicalDrawOffset().x;
    webViewRect.origin.y = physicalRect.y + VCS.GetPhysicalDrawOffset().y;
    
    if (isRenderToTexture)
    {
        const int32 offScreenPos = -10000;
        // on iOS just move window away and we can render it into our texture
        // if we will add/remove to view hierarchy - bug with memory
        // if we hide windows render to texture - always blank texture
        webViewRect.origin.x = offScreenPos;
    }

    webViewRect.size.width = physicalRect.dx;
    webViewRect.size.height = physicalRect.dy;
	
	// Apply the Retina scale divider, if any.
    DAVA::float32 scaleDivider = Core::Instance()->GetScreenScaleFactor();
	webViewRect.origin.x /= scaleDivider;
	webViewRect.origin.y /= scaleDivider;
	webViewRect.size.height /= scaleDivider;
	webViewRect.size.width /= scaleDivider;

	[(UIWebView*)webViewPtr setFrame: webViewRect];
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
    pendingVisible = isVisible;
    
    // Workaround: call WillDraw instantly because it will not be called on SystemDraw
    if(!isVisible)
    {
        WillDraw();
    }
}

void WebViewControl::SetScalesPageToFit(bool isScalesToFit)
{
	[(UIWebView*)webViewPtr setScalesPageToFit:isScalesToFit];
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
		[webView setBackgroundColor:[UIColor colorWithRed:r green:g blue:b
                                                    alpha:0.f]];
		HideSubviewImages(webView);
	}
	else
	{
		[webView setBackgroundColor:[UIColor colorWithRed:r green:g blue:b
                                                    alpha:1.0f]];
		RestoreSubviewImages();
	}
}

void WebViewControl::HideSubviewImages(void* view)
{
    UIWebView* webView = (UIWebView*)webViewPtr;
    ::UIScrollView *scrollView = webView.scrollView;
    
	UIView* uiview = (UIView*)view;
	for (UIView* subview in [uiview subviews])
	{
        if(uiview == scrollView)
            continue;
        
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

//for iOS we need use techique like http://stackoverflow.com/questions/12578895/how-to-detect-a-swipe-gesture-on-webview
void WebViewControl::SetGestures(bool value)
{
    HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
    UIView * backView = appDelegate.glController.backgroundView;

    if (value && !gesturesEnabled)
    {
        WebViewURLDelegate * urlDelegate = (WebViewURLDelegate *)webViewURLDelegatePtr;
        
        UISwipeGestureRecognizer * rightSwipeGesture = [[UISwipeGestureRecognizer alloc] initWithTarget:urlDelegate action:@selector(rightGesture)];
        UISwipeGestureRecognizer * leftSwipeGesture = [[UISwipeGestureRecognizer alloc] initWithTarget:urlDelegate action:@selector(leftGesture)];
        rightSwipeGesture.direction = UISwipeGestureRecognizerDirectionRight;
        leftSwipeGesture.direction = UISwipeGestureRecognizerDirectionLeft;
        
        [backView addGestureRecognizer:rightSwipeGesture];
        [backView addGestureRecognizer:leftSwipeGesture];
        
        UIWebView* localWebView = (UIWebView*)webViewPtr;
        [localWebView.scrollView.panGestureRecognizer requireGestureRecognizerToFail:rightSwipeGesture];
        [localWebView.scrollView.panGestureRecognizer requireGestureRecognizerToFail:leftSwipeGesture];
        rightSwipeGesturePtr = rightSwipeGesture;
        leftSwipeGesturePtr = leftSwipeGesture;
    }
    else if (!value && gesturesEnabled)
    {
        UISwipeGestureRecognizer *rightSwipeGesture = (UISwipeGestureRecognizer *)rightSwipeGesturePtr;
        UISwipeGestureRecognizer *leftSwipeGesture = (UISwipeGestureRecognizer *)leftSwipeGesturePtr;
        
        [backView removeGestureRecognizer:rightSwipeGesture];
        [backView removeGestureRecognizer:leftSwipeGesture];
        [rightSwipeGesture release];
        [leftSwipeGesture release];
        rightSwipeGesturePtr = nil;
        leftSwipeGesturePtr = nil;
    }
    gesturesEnabled = value;
}

void WebViewControl::SetDataDetectorTypes(int32 value)
{
    NSUInteger systemDetectorTypes = 0;

    int detectorsCount = COUNT_OF(detectorsMap);
    for (int i = 0; i < detectorsCount; i ++)
    {
        if ((value & detectorsMap[i].davaDetectorType) == detectorsMap[i].davaDetectorType)
        {
            systemDetectorTypes |= detectorsMap[i].systemDetectorType;
        }
    }

    UIWebView* localWebView = (UIWebView*)webViewPtr;
    localWebView.dataDetectorTypes = systemDetectorTypes;
}

int32 WebViewControl::GetDataDetectorTypes() const
{
    UIWebView* localWebView = (UIWebView*)webViewPtr;
    NSUInteger systemDetectorTypes = localWebView.dataDetectorTypes;

    int32 davaDetectorTypes = 0;
    
    int detectorsCount = COUNT_OF(detectorsMap);
    for (int i = 0; i < detectorsCount; i ++)
    {
        if ((systemDetectorTypes & detectorsMap[i].systemDetectorType) == detectorsMap[i].systemDetectorType)
        {
            davaDetectorTypes |= detectorsMap[i].davaDetectorType;
        }
    }

    return davaDetectorTypes;
}
    
void WebViewControl::SetRenderToTexture(bool value)
{
    pendingRenderToTexture = value;
}
    
void WebViewControl::WillDraw()
{
    if(isVisible != pendingVisible)
    {
        isVisible = pendingVisible;
        [(UIWebView *) webViewPtr setHidden:(isVisible ? NO : YES)];
    }
    
    if (isRenderToTexture != pendingRenderToTexture)
    {
        isRenderToTexture = pendingVisible;
        
        // hide windows - move to offScreenPos position
        // so it still can render WebView into
        DAVA::Rect r = uiWebView.GetRect();
        SetRect(r);
        
        if(isRenderToTexture)
        {
            // we have to show window or we can't render web view into texture
            if(!isVisible)
            {
                [(UIWebView*)webViewPtr setHidden:NO];
            }
            RenderToTextureAndSetAsBackgroundSpriteToControl(uiWebView);
            if(!isVisible)
            {
                [(UIWebView *) webViewPtr setHidden:YES];
            }
            
        }
    }
}
    
} // end namespace DAVA
