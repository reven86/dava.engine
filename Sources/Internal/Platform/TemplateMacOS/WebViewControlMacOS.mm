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



#include "Base/Platform.h"
#if defined __DAVAENGINE_MACOS__ && !defined __DISABLE_NATIVE_WEBVIEW__

#include "Platform/TemplateMacOS/WebViewControlMacOS.h"
#include "Platform/TemplateMacOS/MainWindowController.h"
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"

#import <WebKit/WebKit.h>
#import <AppKit/NSWorkspace.h>

using namespace DAVA;

// A delegate is needed to block the context menu. Note - this delegate
// is informal, so no inheritance from WebUIDelegate needed.
#if defined(__MAC_10_11)
@interface WebViewControlUIDelegate : NSObject<WebUIDelegate>
#else
@interface WebViewControlUIDelegate : NSObject
#endif
{
}

// Intercept the right-mouse-clicks/
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems;

@end

@implementation WebViewControlUIDelegate

- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems
{
	// No menu items needed.
	return nil;
}

@end

#if defined(__MAC_10_11)
@interface WebViewPolicyDelegate : NSObject<WebPolicyDelegate, WebFrameLoadDelegate>
#else
@interface WebViewPolicyDelegate : NSObject
#endif
{
	IUIWebViewDelegate* delegate;
    UIWebView* webView;
    WebViewControl* webViewControl;
}

- (id)init;

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener;

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame;
- (void)setDelegate:(IUIWebViewDelegate*)d andWebView:(UIWebView*)w;
- (void)onExecuteJScript:(NSString *)result;
- (void)setWebViewControl:(WebViewControl*) webControl;
- (void)setUiWebViewControl:(UIWebView*) uiWebControl;

@end

@implementation WebViewPolicyDelegate

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

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
	BOOL process = YES;
	
	if (delegate && self->webView)
	{
		NSString* url = [[request URL] absoluteString];
		
		if (url)
		{
            NSInteger navigationTypeKey = [[actionInformation objectForKey:@"WebActionNavigationTypeKey"] integerValue];
            bool isRedirecteByMouseClick = navigationTypeKey == WebNavigationTypeLinkClicked;
			IUIWebViewDelegate::eAction action = delegate->URLChanged(self->webView, [url UTF8String], isRedirecteByMouseClick);
			
			switch (action)
			{
				case IUIWebViewDelegate::PROCESS_IN_WEBVIEW:
					Logger::FrameworkDebug("PROCESS_IN_WEBVIEW");
					break;
					
				case IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					Logger::FrameworkDebug("PROCESS_IN_SYSTEM_BROWSER");
					process = NO;
					[[NSWorkspace sharedWorkspace] openURL:[request URL]];
					break;
					
				case IUIWebViewDelegate::NO_PROCESS:
					Logger::FrameworkDebug("NO_PROCESS");
					
				default:
					process = NO;
					break;
			}
		}
	}
	
	if (process)
	{
		[listener use];
	}
	else
	{
		[listener ignore];
	}
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    if (webView && webView->IsRenderToTexture())
    {
        webViewControl->RenderToTextureAndSetAsBackgroundSpriteToControl(*webView);
    }
    
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
    }
}

- (void)setDelegate:(IUIWebViewDelegate *)d
         andWebView:(UIWebView *)w
{
	if (d && w)
	{
		delegate = d;
		webView = w;
	}
}

- (void)onExecuteJScript:(NSString *)result
{
    if (delegate)
    {
        delegate->OnExecuteJScript(webView, String([result UTF8String]));
    }
}

- (void)setWebViewControl:(WebViewControl*) webControl
{
    DVASSERT(webControl);
    webViewControl = webControl;
}
                           
- (void)setUiWebViewControl:(UIWebView*) uiWebControl
{
    DVASSERT(uiWebControl);
    webView = uiWebControl;
}

@end

WebViewControl::WebViewControl(UIWebView& ptr) :
    webImageCachePtr(0),
    uiWebViewControl(ptr),
    isRenderToTexture(false),
    isVisible(true)
{
	NSRect emptyRect = NSMakeRect(0.0f, 0.0f, 0.0f, 0.0f);	
	webViewPtr = [[WebView alloc] initWithFrame:emptyRect frameName:nil
                                      groupName:nil];

	WebView* localWebView = (WebView*)webViewPtr;
	[localWebView setWantsLayer:YES];
	
	webViewDelegatePtr = [[WebViewControlUIDelegate alloc] init];
	[localWebView setUIDelegate:(WebViewControlUIDelegate*)webViewDelegatePtr];

	webViewPolicyDelegatePtr = [[WebViewPolicyDelegate alloc] init];
	[localWebView setPolicyDelegate:
                    (WebViewPolicyDelegate*)webViewPolicyDelegatePtr];
    
    [localWebView setFrameLoadDelegate:
                    (WebViewPolicyDelegate*)webViewPolicyDelegatePtr];

    [(WebViewPolicyDelegate*)webViewPolicyDelegatePtr setWebViewControl:this];
    [(WebViewPolicyDelegate*)webViewPolicyDelegatePtr setUiWebViewControl:
                                                            &uiWebViewControl];

    NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
    [openGLView addSubview:localWebView];

    // if switch to renderToTexture mode
    [localWebView setShouldUpdateWhileOffscreen:YES];
    
    CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
    appMinimizedRestoredConnectionId = xcore->signalAppMinimizedRestored.Connect(this, &WebViewControl::OnAppMinimizedRestored);
}

WebViewControl::~WebViewControl()
{
    CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Disconnect(appMinimizedRestoredConnectionId);
    
    NSBitmapImageRep* imageRep = (NSBitmapImageRep*)webImageCachePtr;
   [imageRep release];
    webImageCachePtr = 0;
    
	WebView* innerWebView = (WebView*)webViewPtr;

	[innerWebView setUIDelegate:nil];

	[innerWebView removeFromSuperview];
	[innerWebView close];
	[innerWebView release];
	webViewPtr = 0;

	WebViewPolicyDelegate* w = (WebViewPolicyDelegate*)webViewPolicyDelegatePtr;
	[w release];
	webViewPolicyDelegatePtr = 0;
    
    WebViewControlUIDelegate* c = (WebViewControlUIDelegate*)webViewDelegatePtr;
    [c release];
    webViewDelegatePtr = 0;
}

void WebViewControl::SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView)
{
	WebViewPolicyDelegate* w = (WebViewPolicyDelegate*)webViewPolicyDelegatePtr;
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
	[(WebView*)webViewPtr setMainFrameURL:nsURLPathToOpen];
}

void WebViewControl::LoadHtmlString(const WideString& htlmString)
{
	NSString* htmlPageToLoad = [[[NSString alloc]
                                 initWithBytes: htlmString.data()
                                        length: htlmString.size() * sizeof(wchar_t)
                                      encoding: NSUTF32LittleEndianStringEncoding] autorelease];
    [[(WebView*)webViewPtr mainFrame]
        loadHTMLString:htmlPageToLoad
        baseURL:[[NSBundle mainBundle] bundleURL]];
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

void WebViewControl::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    NSString* dataToOpen = [NSString stringWithUTF8String:string.c_str()];
    NSString* baseUrl = [NSString stringWithUTF8String:basePath.AsURL().c_str()];
    [[(WebView*)webViewPtr mainFrame] loadHTMLString:dataToOpen baseURL:[NSURL URLWithString:baseUrl]];
}

void WebViewControl::SetRect(const Rect& srcRect)
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();
    
    // 1. map virtual to physical
    Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
    rect += coordSystem->GetPhysicalDrawOffset();
    rect.y = coordSystem->GetPhysicalScreenSize().dy - (rect.y + rect.dy);
    
    // 2. map physical to window
    NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
    NSRect controlRect = [openGLView convertRectFromBacking:NSMakeRect(rect.x, rect.y, rect.dx, rect.dy)];
    [(WebView*)webViewPtr setFrame:controlRect];
    
    if (isRenderToTexture)
    {
        RenderToTextureAndSetAsBackgroundSpriteToControl(uiWebViewControl);
    }
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
    this->isVisible = isVisible;
    
    if (!isRenderToTexture)
    {
        SetNativeVisible(isVisible);
    }
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
	WebView* webView = (WebView*)webViewPtr;
	[webView setDrawsBackground:(enabled ? NO : YES)];
}

void WebViewControl::SetRenderToTexture(bool value)
{
    isRenderToTexture = value;
    if (isRenderToTexture)
    {
        RenderToTextureAndSetAsBackgroundSpriteToControl(uiWebViewControl);
        SetNativeVisible(false);
    }
    else
    {
        // remove sprite from UIControl and show native window
        uiWebViewControl.SetSprite(0, 0);
        if (isVisible)
        {
            SetNativeVisible(true);
        }
    }
}

void WebViewControl::RenderToTextureAndSetAsBackgroundSpriteToControl(
                                        UIWebView& uiWebViewControl)
{
    bool recreateImageRep = true;
    NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
    WebView* nativeWebView = static_cast<WebView*>(webViewPtr);
    NSBitmapImageRep* imageRep = static_cast<NSBitmapImageRep*>(webImageCachePtr);
    if (imageRep != nullptr)
    {
        NSSize imageRepSize = [imageRep size];
        NSSize webViewSize = [nativeWebView frame].size;
        recreateImageRep = imageRepSize.width != webViewSize.width || imageRepSize.height != webViewSize.height;
    }
    if (recreateImageRep)
    {
        [imageRep release];
        imageRep = [openGLView bitmapImageRepForCachingDisplayInRect:[nativeWebView frame]];
        if (imageRep != nullptr)
        {
            webImageCachePtr = imageRep;
            [imageRep retain];
        }
        else
        {
            webImageCachePtr = nullptr;
        }
    }

    if (nullptr == imageRep)
    {
        uiWebViewControl.SetSprite(nullptr, 0);
        return;
    }

    NSSize imageRepSize = [imageRep size];
    NSRect imageRect = NSMakeRect(0.f, 0.f, imageRepSize.width, imageRepSize.height);
    
    // render web view into bitmap image
    [nativeWebView cacheDisplayInRect:imageRect toBitmapImageRep:imageRep];
    
    const uint8* rawData = [imageRep bitmapData];
    const int w = [imageRep pixelsWide];
    const int h = [imageRep pixelsHigh];
    const int BPP = [imageRep bitsPerPixel];
    const int pitch = [imageRep bytesPerRow];
    
    PixelFormat format = FORMAT_INVALID;
    if (24 == BPP)
    {
        format = FORMAT_RGB888;
    }
    else if (32 == BPP)
    {
        DVASSERT(!([imageRep bitmapFormat] & NSAlphaFirstBitmapFormat));
        format = FORMAT_RGBA8888;
    }
    else
    {
        DVASSERT(false && "[WebView] Unexpected bits per pixel value");
        uiWebViewControl.SetSprite(nullptr, 0);
        return;
    }
    
    {
        RefPtr<Image> imageRGB;
        int bytesPerLine = w * (BPP / 8);
        
        if (pitch == bytesPerLine)
        {
            imageRGB = Image::CreateFromData(w, h, format, rawData);
        }
        else
        {
            imageRGB = Image::Create(w, h, format);
            uint8* pixels = imageRGB->GetData();
            
            // copy line by line image
            for(int y = 0; y < h; ++y)
            {
                uint8* dstLineStart = &pixels[y * bytesPerLine];
                const uint8* srcLineStart = &rawData[y * pitch];
                Memcpy(dstLineStart, srcLineStart, bytesPerLine);
            }
        }
        
        DVASSERT(imageRGB);
        {
            RefPtr<Texture> tex(Texture::CreateFromData(imageRGB.Get(), false));
            const Rect& rect = uiWebViewControl.GetRect();
            {
                RefPtr<Sprite> sprite(Sprite::CreateFromTexture(tex.Get(), 0, 0, w, h, rect.dx, rect.dy));
                uiWebViewControl.SetSprite(sprite.Get(), 0);
            }
        }
    }
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    NSString *jScriptString = [NSString stringWithUTF8String:scriptString.c_str()];
    NSString *resultString = [(WebView*)webViewPtr stringByEvaluatingJavaScriptFromString:jScriptString];

    WebViewPolicyDelegate* w = (WebViewPolicyDelegate*) webViewPolicyDelegatePtr;
    if (w)
    {
        [w performSelector:@selector(onExecuteJScript:) withObject:resultString afterDelay:0.0];
    }
}

void WebViewControl::SetImageCache(void* ptr)
{
    DVASSERT(ptr);
    webImageCachePtr = ptr;
}

void* WebViewControl::GetImageCache() const
{
    return webImageCachePtr;
}

void WebViewControl::SetNativeVisible(bool visible)
{
    if (visible)
    {
        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [openGLView addSubview:(WebView*)webViewPtr];
    }
    else
    {
        [(WebView*)webViewPtr removeFromSuperview];
    }
}

void WebViewControl::OnAppMinimizedRestored(bool minimized)
{
    if (isVisible)
    {
        SetNativeVisible(!minimized);
    }
}

#endif //defined __DAVAENGINE_MACOS__ && !defined __DISABLE_NATIVE_WEBVIEW__