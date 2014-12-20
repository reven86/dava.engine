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



#include "WebViewControlMacOS.h"
#include "MainWindowController.h"

#import <WebKit/WebKit.h>
#import <AppKit/NSWorkspace.h>

using namespace DAVA;

// A delegate is needed to block the context menu. Note - this delegate
// is informal, so no inheritance from WebUIDelegate needed.
@interface WebViewControlUIDelegate : NSObject
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


@interface WebViewPolicyDelegate : NSObject
{
	IUIWebViewDelegate* delegate;
	UIWebView* webView;
    WebViewControl* webViewControl;
}

- (id)init;

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener;

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame;

- (void)setDelegate:(IUIWebViewDelegate*)d andWebView:(UIWebView*)w;
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
    // TODO get image from loaded frame
    NSRect testRect = NSMakeRect(0.0f, 0.0f, 700.0f, 500.0f);
    
    NSBitmapImageRep* imageRep = [sender bitmapImageRepForCachingDisplayInRect:testRect];
    DVASSERT(imageRep);
    [sender cacheDisplayInRect:testRect toBitmapImageRep:imageRep];
    if (webView)
    {
        const unsigned char* rawData = [imageRep bitmapData];
        int w = [imageRep size].width;
        int h = [imageRep size].height;
        const int BPP = [imageRep bitsPerPixel];
        const int pitch = [imageRep bytesPerRow];
        {
            PixelFormat format = FORMAT_INVALID;
            if (24 == BPP)
            {
                format = FORMAT_RGB888;
                DVASSERT(pitch == w * 3);
            } else if (32 == BPP){
                format = FORMAT_RGBA8888;
                DVASSERT(pitch == w * 4);
            } else
            {
                DVASSERT(false);
            }
            Image* imageRGB = Image::CreateFromData(w, h, format, rawData);
            DVASSERT(imageRGB);
            Sprite* spr = Sprite::CreateFromImage(imageRGB);
            webView->SetSprite(spr, 0);
            imageRGB->Release();
        }
    }
    if (webViewControl)
    {
        webViewControl->SetImage(imageRep);
        NSData *pngData = [imageRep representationUsingType:NSPNGFileType properties:nil];
        [pngData writeToFile:@"test_output.png" atomically:YES];
        // should I delete pngData?
    }
    
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
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

- (void)setWebViewControl:(WebViewControl*) webControl
{
    if (webControl)
    {
        webViewControl = webControl;
    }
}
                           
- (void)setUiWebViewControl:(UIWebView*) uiWebControl
{
   if (uiWebControl)
   {
       webView = uiWebControl;
   }
}

@end


WebViewControl::WebViewControl(UIWebView* ptr) :
    isWebViewVisible(true),
    webImageCachePtr(0),
    uiWebView(ptr)
{
	NSRect emptyRect = NSMakeRect(0.0f, 0.0f, 0.0f, 0.0f);	
	webViewPtr = [[WebView alloc] initWithFrame:emptyRect frameName:nil groupName:nil];

	WebView* localWebView = (WebView*)webViewPtr;
	[localWebView setWantsLayer:YES];
	
	webViewDelegatePtr = [[WebViewControlUIDelegate alloc] init];
	[localWebView setUIDelegate:(WebViewControlUIDelegate*)webViewDelegatePtr];

	webViewPolicyDelegatePtr = [[WebViewPolicyDelegate alloc] init];
	[localWebView setPolicyDelegate:(WebViewPolicyDelegate*)webViewPolicyDelegatePtr];
    
    [localWebView setFrameLoadDelegate:(WebViewPolicyDelegate*)webViewPolicyDelegatePtr];

    [(WebViewPolicyDelegate*)webViewPolicyDelegatePtr setWebViewControl:this];
    [(WebViewPolicyDelegate*)webViewPolicyDelegatePtr setUiWebViewControl:uiWebView];
    
	NSView* openGLView = (NSView*)Core::Instance()->GetOpenGLView();
	[openGLView addSubview:localWebView];
    
    // TODO try hide windows and render it's content to image(texture)
    
    [localWebView setShouldUpdateWhileOffscreen:YES];
    
    //[localWebView setHidden:YES];
    
    NSRect testRect = NSMakeRect(0.0f, 0.0f, 700.0f, 500.0f);
    
    NSBitmapImageRep* imageRep = [openGLView bitmapImageRepForCachingDisplayInRect:testRect];
    DVASSERT(imageRep);
    webImageCachePtr = imageRep;
}

WebViewControl::~WebViewControl()
{
	WebView* innerWebView = (WebView*)webViewPtr;

	[innerWebView setUIDelegate:nil];

	[innerWebView removeFromSuperview];
	[innerWebView close];
	[innerWebView release];
	webViewPtr = nil;

	WebViewPolicyDelegate* w = (WebViewPolicyDelegate*)webViewPolicyDelegatePtr;
	[w release];
	webViewPolicyDelegatePtr = nil;
    
    WebViewControlUIDelegate* c = (WebViewControlUIDelegate*)webViewDelegatePtr;
    [c release];
    webViewDelegatePtr = nil;
}

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView)
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

void WebViewControl::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    NSString* dataToOpen = [NSString stringWithUTF8String:string.c_str()];
    NSString* baseUrl = [NSString stringWithUTF8String:basePath.AsURL().c_str()];
    [[(WebView*)webViewPtr mainFrame] loadHTMLString:dataToOpen baseURL:[NSURL URLWithString:baseUrl]];
}

void WebViewControl::SetRect(const Rect& rect)
{
	NSRect webViewRect = [(WebView*)webViewPtr frame];

	webViewRect.size.width = rect.dx * Core::GetVirtualToPhysicalFactor();
	webViewRect.size.height = rect.dy * Core::GetVirtualToPhysicalFactor();
	
	webViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
	webViewRect.origin.y = Core::Instance()->GetPhysicalScreenHeight() - (rect.y + rect.dy) * DAVA::Core::GetVirtualToPhysicalFactor();
	
	webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
	webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;
	
	[(WebView*)webViewPtr setFrame: webViewRect];
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
    if (!isWebViewVisible && isVisible)
    {
        // TODO delow is comented render to image works, continue investigate
//        NSView* openGLView = (NSView*)Core::Instance()->GetOpenGLView();
//        [openGLView addSubview:(WebView*)webViewPtr];
    }
    else if (isWebViewVisible && !isVisible)
    {
        [(WebView*)webViewPtr removeFromSuperview];
    }
    
    isWebViewVisible = isVisible;
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
	WebView* webView = (WebView*)webViewPtr;
	[webView setDrawsBackground:(enabled ? NO : YES)];
}

void WebViewControl::SetRenderToTexture(bool value)
{
    // TODO
}

bool WebViewControl::IsRenderToTexture() const
{
    // TODO
    return true;
}

void WebViewControl::SetImage(void* ptr)
{
    DVASSERT(ptr);
    this->webImageCachePtr = ptr;
}
