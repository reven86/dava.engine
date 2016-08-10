#ifndef __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__
#define __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__

#include "Base/BaseTypes.h"
#if defined __DAVAENGINE_MACOS__ && !defined DISABLE_NATIVE_WEBVIEW

#include "UI/IWebViewControl.h"
#include "Functional/SignalBase.h"

namespace DAVA
{
class Window;
class UIWebView;

// Web View Control - MacOS version.
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
    void LoadHtmlString(const WideString& htlmString) override;
    // Delete all cookies associated with target URL
    void DeleteCookies(const String& targetUrl) override;
    // Execute javascript command, return request ID
    void ExecuteJScript(const String& scriptString) override;

    void OpenFromBuffer(const String& string, const FilePath& basePath) override;

    // Size/pos/visibility changes.
    void SetRect(const Rect& rect) override;
    void SetVisible(bool isVisible, bool hierarchic) override;

    void SetDelegate(IUIWebViewDelegate* delegate, UIWebView* webView) override;
    void SetBackgroundTransparency(bool enabled) override;

    void SetRenderToTexture(bool value) override;
    bool IsRenderToTexture() const override;

    void SetImageCache(void* ptr);
    void* GetImageCache() const;

    void RenderToTextureAndSetAsBackgroundSpriteToControl(UIWebView& uiWebViewControl);

private:
    void SetNativeVisible(bool visible);

#if defined(__DAVAENGINE_COREV2__)
    void OnWindowVisibilityChanged(Window& w, bool visible);
    size_t windowVisibilityChangedConnection = 0;
#else
    void OnAppMinimizedRestored(bool minimized);
    SigConnectionID appMinimizedRestoredConnectionId;
#endif

private:
    UIWebView& uiWebViewControl;
    
#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
#endif
    struct WebViewObjCBridge;
    WebViewObjCBridge* bridge = nullptr;

    bool isRenderToTexture = false;
    bool isVisible = true;
};

inline bool WebViewControl::IsRenderToTexture() const
{
    return isRenderToTexture;
}
}

#endif //defined __DAVAENGINE_MACOS__ && !defined DISABLE_NATIVE_WEBVIEW

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */
