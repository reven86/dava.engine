#ifndef __DAVAENGINE_PRIVATEWEBVIEWWINUAP_H__
#define __DAVAENGINE_PRIVATEWEBVIEWWINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
class Sprite;
class CorePlatformWinUAP;

class PrivateWebViewWinUAP : public std::enable_shared_from_this<PrivateWebViewWinUAP>
{
    struct WebViewProperties
    {
        enum
        {
            NAVIGATE_NONE = 0,
            NAVIGATE_OPEN_URL,
            NAVIGATE_LOAD_HTML,
            NAVIGATE_OPEN_BUFFER
        };

        WebViewProperties();
        void ClearChangedFlags();

        Rect rect;
        Rect rectInWindowSpace;
        bool visible = false;
        bool renderToTexture = false;
        bool backgroundTransparency = false;
        WideString htmlString; // for LoadHtmlString
        String urlOrHtml; // for OpenURL or OpenFromBuffer
        FilePath basePath; // for OpenFromBuffer
        String jsScript; // for ExecuteJScript

        bool createNew : 1;
        bool anyPropertyChanged : 1;
        bool rectChanged : 1;
        bool visibleChanged : 1;
        bool renderToTextureChanged : 1;
        bool backgroundTransparencyChanged : 1;
        bool execJavaScript : 1;
        uint32 navigateTo : 2;
    };

public:
    PrivateWebViewWinUAP(UIWebView* UIWebView);
    ~PrivateWebViewWinUAP();

    // WebViewControl should invoke it in its destructor to tell this class instance
    // to fly away on its own (finish pending jobs if any, and delete when all references are lost)
    void OwnerAtPremortem();

    void Initialize(const Rect& rect);

    void OpenURL(const String& urlToOpen);
    void LoadHtmlString(const WideString& htmlString);
    void OpenFromBuffer(const String& string, const FilePath& basePath);
    void ExecuteJScript(const String& scriptString);

    void SetRect(const Rect& rect);
    void SetVisible(bool isVisible);
    void SetBackgroundTransparency(bool enabled);

    void SetDelegate(IUIWebViewDelegate* webViewDelegate);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    void Update();

private:
    void CreateNativeControl();
    void InstallEventHandlers();

    void ProcessProperties(const WebViewProperties& props);
    void ApplyChangedProperties(const WebViewProperties& props);

    void SetNativePositionAndSize(const Rect& rect, bool offScreen);
    void SetNativeBackgroundTransparency(bool enabled);
    void NativeNavigateTo(const WebViewProperties& props);
    void NativeExecuteJavaScript(const String& jsScript);

    Rect VirtualToWindow(const Rect& srcRect) const;

    void RenderToTexture();
    Sprite* CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const;

private: // WebView event handlers
    void OnNavigationStarting(Windows::UI::Xaml::Controls::WebView ^ sender, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs ^ args);
    void OnNavigationCompleted(Windows::UI::Xaml::Controls::WebView ^ sender, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs ^ args);

private:
    // clang-format off
    CorePlatformWinUAP* core;
    UIWebView* uiWebView = nullptr;
    IUIWebViewDelegate* webViewDelegate = nullptr;
    Windows::UI::Xaml::Controls::WebView^ nativeWebView = nullptr;
    Windows::UI::Color defaultBkgndColor;   // Original WebView's background color used on transparency turned off

    bool renderToTexture = false;
    bool visible = false;                   // Native control initially is invisible
    Rect rectInWindowSpace;

    WebViewProperties properties;
    // clang-format on
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_PRIVATEWEBVIEWWINUAP_H__
