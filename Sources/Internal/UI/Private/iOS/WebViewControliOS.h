#ifndef __DAVAENGINE_WEBVIEWCONTROL_IOS_H__
#define __DAVAENGINE_WEBVIEWCONTROL_IOS_H__

#include "Base/Platform.h"
#if defined(__DAVAENGINE_IPHONE__) && !defined(DISABLE_NATIVE_WEBVIEW)

#include "UI/IWebViewControl.h"

namespace DAVA
{
class UIControl;
class FilePath;
class UIWebView;

// Web View Control - iOS version.
class WebViewControl : public IWebViewControl
{
public:
    explicit WebViewControl(UIWebView& uiWebView);
    virtual ~WebViewControl();

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Open the URL requested.
    void OpenURL(const String& urlToOpen) override;
    // Load html page from string
    void LoadHtmlString(const WideString& htmlString) override;
    // Delete all cookies associated with target URL
    void DeleteCookies(const String& targetUrl) override;
    // Get cookie for specific domain and name
    String GetCookie(const String& targetUrl, const String& name) const override;
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& targetUrl) const override;
    // Perfrom Java script
    void ExecuteJScript(const String& scriptString) override;

    void OpenFromBuffer(const String& string, const FilePath& basePath) override;

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;

    void SetScalesPageToFit(bool isScalesToFit) override;

    void SetDelegate(IUIWebViewDelegate* delegate,
                     UIWebView* webView) override;
    void SetBackgroundTransparency(bool enabled) override;

    // Bounces control.
    bool GetBounces() const override;
    void SetBounces(bool value) override;
    void SetGestures(bool value) override;

    // Data detector types.
    void SetDataDetectorTypes(int32 value) override;
    int32 GetDataDetectorTypes() const override;

    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override
    {
        return pendingRenderToTexture;
    }

    void WillDraw() override;

    // common ios part to render any UIView* to UIImage*
    // workaroundKeyboardBug - if call during show/hide keyboard - false
    static void* RenderIOSUIViewToImage(void* uiviewPtr);
    // common ios part to copy from ios ::UIImage* to DAVA::Sprite*
    static void SetImageAsSpriteToControl(void* imagePtr, UIControl& control);

    void RenderToTextureAndSetAsBackgroundSpriteToControl(UIWebView& control);

private:
    //A pointer to iOS WebView.
    void* webViewPtr;

    // A pointer to the WebView delegate.

    void* webViewURLDelegatePtr;

    void* rightSwipeGesturePtr;
    void* leftSwipeGesturePtr;

    Map<void*, bool> subviewVisibilityMap;

    void HideSubviewImages(void* view);
    void RestoreSubviewImages();

    bool gesturesEnabled;
    bool isRenderToTexture;
    bool pendingRenderToTexture;
    bool isVisible;
    bool pendingVisible;

    UIWebView& uiWebView;
};
};

#endif //defined(__DAVAENGINE_IPHONE__) && !defined(DISABLE_NATIVE_WEBVIEW)

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_IOS_H__) */
