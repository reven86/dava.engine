#include "UI/Private/OSX/WebViewControlMacOS.h"

#if defined __DAVAENGINE_MACOS__ && !defined DISABLE_NATIVE_WEBVIEW

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/EngineModule.h"
#include "Engine/WindowNativeService.h"
#else
#include "Platform/TemplateMacOS/MainWindowController.h"
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#endif

#include "Render/Image/Image.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "UI/UIWebView.h"

#import <WebKit/WebKit.h>
#import <AppKit/NSWorkspace.h>

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
- (NSArray*)webView:(WebView*)sender contextMenuItemsForElement:(NSDictionary*)element defaultMenuItems:(NSArray*)defaultMenuItems;

@end

@implementation WebViewControlUIDelegate

- (NSArray*)webView:(WebView*)sender contextMenuItemsForElement:(NSDictionary*)element defaultMenuItems:(NSArray*)defaultMenuItems
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
    DAVA::IUIWebViewDelegate* delegate;
    DAVA::UIWebView* webView;
    DAVA::WebViewControl* webViewControl;
}

- (id)init;

- (void)webView:(WebView*)inWebView decidePolicyForNewWindowAction:(NSDictionary*)actionInformation request:(NSURLRequest*)inRequest newFrameName:(WebFrame*)inFrame decisionListener:(id<WebPolicyDecisionListener>)listener;

- (void)webView:(WebView*)webView decidePolicyForNavigationAction:(NSDictionary*)actionInformation request:(NSURLRequest*)request frame:(WebFrame*)frame decisionListener:(id<WebPolicyDecisionListener>)listener;

- (void)webView:(WebView*)sender didFinishLoadForFrame:(WebFrame*)frame;
- (void)setDelegate:(DAVA::IUIWebViewDelegate*)d andWebView:(DAVA::UIWebView*)w;
- (void)onExecuteJScript:(NSString*)result;
- (void)setWebViewControl:(DAVA::WebViewControl*)webControl;
- (void)setUiWebViewControl:(DAVA::UIWebView*)uiWebControl;

@end

@implementation WebViewPolicyDelegate

- (id)init
{
    self = [super init];
    if (self)
    {
        delegate = nullptr;
        webView = nullptr;
    }
    return self;
}

- (void)webView:(WebView*)inWebView decidePolicyForNewWindowAction:(NSDictionary*)actionInformation request:(NSURLRequest*)inRequest newFrameName:(WebFrame*)inFrame decisionListener:(id<WebPolicyDecisionListener>)listener
{
    [self webView:inWebView decidePolicyForNavigationAction:actionInformation request:inRequest frame:inFrame decisionListener:listener];
}

- (void)webView:(WebView*)webView decidePolicyForNavigationAction:(NSDictionary*)actionInformation request:(NSURLRequest*)request frame:(WebFrame*)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
    using namespace DAVA;

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

- (void)webView:(WebView*)sender didFinishLoadForFrame:(WebFrame*)frame
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

- (void)setDelegate:(DAVA::IUIWebViewDelegate*)d
         andWebView:(DAVA::UIWebView*)w
{
    delegate = d;
    webView = w;
}

- (void)onExecuteJScript:(NSString*)result
{
    if (delegate)
    {
        delegate->OnExecuteJScript(webView, DAVA::String([result UTF8String]));
    }
}

- (void)setWebViewControl:(DAVA::WebViewControl*)webControl
{
    webViewControl = webControl;
}

- (void)setUiWebViewControl:(DAVA::UIWebView*)uiWebControl
{
    webView = uiWebControl;
}

@end

namespace DAVA
{
// Struct to hold Objective-C classes and get rid of void* in WebViewControl
struct WebViewControl::WebViewObjCBridge final
{
    WebView* webView = nullptr;
    WebViewPolicyDelegate* policyDelegate = nullptr;
    WebViewControlUIDelegate* controlUIDelegate = nullptr;
    NSBitmapImageRep* bitmapImageRep = nullptr;
};

#if defined(__DAVAENGINE_COREV2__)
WebViewControl::WebViewControl(Window* w, UIWebView* uiWebView)
    : uiWebViewControl(*uiWebView)
    , window(w)
    , bridge(new WebViewObjCBridge)
#else
WebViewControl::WebViewControl(UIWebView* uiWebView)
    : uiWebViewControl(*uiWebView)
    , bridge(new WebViewObjCBridge)
#endif
{
    bridge->controlUIDelegate = [[WebViewControlUIDelegate alloc] init];
    bridge->policyDelegate = [[WebViewPolicyDelegate alloc] init];
    bridge->webView = [[WebView alloc] initWithFrame:NSMakeRect(0.0f, 0.0f, 0.0f, 0.0f)
                                           frameName:nil
                                           groupName:nil];
    [bridge->webView setWantsLayer:YES];
    [bridge->webView setShouldUpdateWhileOffscreen:YES]; // for rendering to texture

    [bridge->webView setUIDelegate:bridge->controlUIDelegate];
    [bridge->webView setPolicyDelegate:bridge->policyDelegate];
    [bridge->webView setFrameLoadDelegate:bridge->policyDelegate];

    [bridge->policyDelegate setWebViewControl:this];
    [bridge->policyDelegate setUiWebViewControl:&uiWebViewControl];

#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->AddNSView(bridge->webView);

    windowVisibilityChangedConnection = window->visibilityChanged.Connect(this, &WebViewControl::OnWindowVisibilityChanged);
#else
    NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
    [openGLView addSubview:bridge->webView];

    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    appMinimizedRestoredConnectionId = xcore->signalAppMinimizedRestored.Connect(this, &WebViewControl::OnAppMinimizedRestored);
#endif

    SetBackgroundTransparency(true);
}

WebViewControl::~WebViewControl()
{
#if defined(__DAVAENGINE_COREV2__)
    window->visibilityChanged.Disconnect(windowVisibilityChangedConnection);
#else
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Disconnect(appMinimizedRestoredConnectionId);
#endif

    [bridge->bitmapImageRep release];
    bridge->bitmapImageRep = nullptr;

#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->RemoveNSView(bridge->webView);
#else
    [bridge->webView removeFromSuperview];
#endif

    // It is very important to set WebView's delegates to nil before calling [WebView close] as
    //  - WebView can invoke didFinishLoadForFrame for unfinished request
    //  - also WebView can invoke release for delegate
    // Both cases can lean=d to crash
    [bridge->webView setUIDelegate:nil];
    [bridge->webView setPolicyDelegate:nil];
    [bridge->webView setFrameLoadDelegate:nil];

    [bridge->policyDelegate setWebViewControl:nullptr];
    [bridge->policyDelegate setUiWebViewControl:nullptr];
    [bridge->policyDelegate setDelegate:nullptr andWebView:nullptr];

    [bridge->policyDelegate release];
    bridge->policyDelegate = nullptr;

    [bridge->controlUIDelegate release];
    bridge->controlUIDelegate = nullptr;

    [bridge->webView close];
    [bridge->webView release];
    bridge->webView = nullptr;

    delete bridge;
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* delegate, UIWebView* webView)
{
    [bridge->policyDelegate setDelegate:delegate andWebView:webView];
}

void WebViewControl::Initialize(const Rect& rect)
{
    SetRect(rect);
}

// Open the URL requested.
void WebViewControl::OpenURL(const String& urlToOpen)
{
    NSString* nsURLPathToOpen = [NSString stringWithUTF8String:urlToOpen.c_str()];
    [bridge->webView setMainFrameURL:nsURLPathToOpen];
}

void WebViewControl::LoadHtmlString(const WideString& htlmString)
{
    NSString* htmlPageToLoad = [[[NSString alloc]
    initWithBytes:htlmString.data()
           length:htlmString.size() * sizeof(wchar_t)
         encoding:NSUTF32LittleEndianStringEncoding] autorelease];
    [[bridge->webView mainFrame] loadHTMLString:htmlPageToLoad
                                        baseURL:[[NSBundle mainBundle] bundleURL]];
}

void WebViewControl::DeleteCookies(const String& targetUrl)
{
    NSString* targetUrlString = [NSString stringWithUTF8String:targetUrl.c_str()];
    NSHTTPCookieStorage* cookies = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    // Delete all cookies for specified URL
    for (NSHTTPCookie* cookie in [cookies cookies])
    {
        if ([[cookie domain] rangeOfString:targetUrlString].location != NSNotFound)
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

    // First escape base path to allow spaces and other similar symbols
    NSString* basePathAsUrl = [NSString stringWithUTF8String:basePath.AsURL().c_str()];
    NSString* escapedBasePath = [basePathAsUrl stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];

    NSURL* baseUrl = [NSURL URLWithString:escapedBasePath];

    [[bridge->webView mainFrame] loadHTMLString:dataToOpen baseURL:baseUrl];
}

void WebViewControl::SetRect(const Rect& srcRect)
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
    rect += coordSystem->GetPhysicalDrawOffset();
    rect.y = coordSystem->GetPhysicalScreenSize().dy - (rect.y + rect.dy);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

#if defined(__DAVAENGINE_COREV2__)
    // 2. map physical to window
    NSRect controlRect = [[bridge->webView superview] convertRectFromBacking:NSMakeRect(rect.x, rect.y, rect.dx, rect.dy)];
#else
    // 2. map physical to window
    NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
    NSRect controlRect = [openGLView convertRectFromBacking:NSMakeRect(rect.x, rect.y, rect.dx, rect.dy)];
#endif
    [bridge->webView setFrame:controlRect];

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
    [bridge->webView setDrawsBackground:enabled ? NO : YES];
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

void WebViewControl::RenderToTextureAndSetAsBackgroundSpriteToControl(UIWebView& uiWebViewControl)
{
    bool recreateImageRep = true;
#if !defined(__DAVAENGINE_COREV2__)
    NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
#endif
    if (bridge->bitmapImageRep != nullptr)
    {
        NSSize imageRepSize = [bridge->bitmapImageRep size];
        NSSize webViewSize = [bridge->webView frame].size;

        recreateImageRep = imageRepSize.width != webViewSize.width || imageRepSize.height != webViewSize.height;
    }
    if (recreateImageRep)
    {
        [bridge->bitmapImageRep release];
#if defined(__DAVAENGINE_COREV2__)
        bridge->bitmapImageRep = [[bridge->webView superview] bitmapImageRepForCachingDisplayInRect:[bridge->webView frame]];
#else
        bridge->bitmapImageRep = [openGLView bitmapImageRepForCachingDisplayInRect:[bridge->webView frame]];
#endif
        if (bridge->bitmapImageRep != nullptr)
        {
            [bridge->bitmapImageRep retain];
        }
    }

    if (bridge->bitmapImageRep == nullptr)
    {
        uiWebViewControl.SetSprite(nullptr, 0);
        return;
    }

    NSSize imageRepSize = [bridge->bitmapImageRep size];
    NSRect imageRect = NSMakeRect(0.f, 0.f, imageRepSize.width, imageRepSize.height);

    // render web view into bitmap image
    [bridge->webView cacheDisplayInRect:imageRect toBitmapImageRep:bridge->bitmapImageRep];

    const uint8* rawData = [bridge->bitmapImageRep bitmapData];
    const int w = [bridge->bitmapImageRep pixelsWide];
    const int h = [bridge->bitmapImageRep pixelsHigh];
    const int BPP = [bridge->bitmapImageRep bitsPerPixel];
    const int pitch = [bridge->bitmapImageRep bytesPerRow];

    PixelFormat format = FORMAT_INVALID;
    if (24 == BPP)
    {
        format = FORMAT_RGB888;
    }
    else if (32 == BPP)
    {
        DVASSERT(!([bridge->bitmapImageRep bitmapFormat] & NSAlphaFirstBitmapFormat));
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
            for (int y = 0; y < h; ++y)
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
    NSString* jScriptString = [NSString stringWithUTF8String:scriptString.c_str()];
    NSString* resultString = [bridge->webView stringByEvaluatingJavaScriptFromString:jScriptString];

    if (bridge->policyDelegate != nullptr)
    {
        [bridge->policyDelegate performSelector:@selector(onExecuteJScript:) withObject:resultString afterDelay:0.0];
    }
}

void WebViewControl::SetNativeVisible(bool visible)
{
    [bridge->webView setHidden:!visible];
}

#if defined(__DAVAENGINE_COREV2__)
void WebViewControl::OnWindowVisibilityChanged(Window& w, bool visible)
{
    if (visible && isVisible)
    {
        // Force WebView repaint after restoring application window
        // Repaint is done by hiding and showing control as people in internet say
        SetNativeVisible(false);
        SetNativeVisible(true);
    }
}
#else
void WebViewControl::OnAppMinimizedRestored(bool minimized)
{
    if (!minimized && isVisible)
    {
        // Force WebView repaint after restoring application window
        // Repaint is done by hiding and showing control as people in internet say
        SetNativeVisible(false);
        SetNativeVisible(true);
    }
}
#endif
} // namespace DAVA

#endif //defined __DAVAENGINE_MACOS__ && !defined DISABLE_NATIVE_WEBVIEW
