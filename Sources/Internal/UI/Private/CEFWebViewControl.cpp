#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_browser.h>

#include "Input/InputSystem.h"
#include "UI/UIEvent.h"
#include "UI/UIControlSystem.h"
#include "UI/UIWebView.h"
#include "UI/Private/CEFWebViewControl.h"
#include "Utils/Utils.h"

namespace DAVA
{
CEFWebViewControl::CEFWebViewControl(UIWebView& uiWebView)
    : webView(uiWebView)
{
}

void CEFWebViewControl::Initialize(const Rect& rect)
{
    webPageRender = new CEFWebPageRender(webView);

    CefWindowInfo windowInfo;
    windowInfo.windowless_rendering_enabled = 1;
    windowInfo.transparent_painting_enabled = 1;

    CefBrowserSettings settings;
    cefBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings, nullptr);
}

void CEFWebViewControl::Deinitialize()
{
    // Close browser and release object
    // If we don't release cefBrowser, dtor of CEFWebViewControl will never be invoked
    cefBrowser->GetHost()->CloseBrowser(true);
    webPageRender = nullptr;
    cefBrowser = nullptr;

    // Wait until CEF release this object
    while (!this->HasOneRef())
    {
        cefController.Update();
    }
}

void CEFWebViewControl::OpenURL(const String& url)
{
    requestedUrl = url;
    LoadURL(url, true);
}

void CEFWebViewControl::LoadHtmlString(const WideString& htmlString)
{
    StopLoading();
    LoadHtml(htmlString, "dava:/~res:/");
}

void CEFWebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    StopLoading();

    String fileUrl = "dava:/" + basePath.GetStringValue();
    LoadHtml(htmlString, fileUrl);
}

void CEFWebViewControl::ExecuteJScript(const String& scriptString)
{
    cefBrowser->GetMainFrame()->ExecuteJavaScript(scriptString, "", 0);
}

void CEFWebViewControl::SetRect(const Rect& rect)
{
    cefBrowser->GetHost()->WasResized();
}

void CEFWebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    cefBrowser->GetHost()->WasHidden(!isVisible);
}

void CEFWebViewControl::SetBackgroundTransparency(bool enabled)
{
}

void CEFWebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* /*webView*/)
{
    delegate = webViewDelegate;
}

void CEFWebViewControl::SetRenderToTexture(bool value)
{
    // Empty realization, always render to texture
}

bool CEFWebViewControl::IsRenderToTexture() const
{
    return true;
}

void CEFWebViewControl::Update()
{
    cefController.Update();

    if (pageLoaded)
    {
        if (delegate)
        {
            delegate->PageLoaded(&webView);
        }
        pageLoaded = false;
    }
}

CefRefPtr<CefRenderHandler> CEFWebViewControl::GetRenderHandler()
{
    return webPageRender;
}

CefRefPtr<CefLoadHandler> CEFWebViewControl::GetLoadHandler()
{
    return this;
}

CefRefPtr<CefRequestHandler> CEFWebViewControl::GetRequestHandler()
{
    return this;
}

CefRefPtr<CefLifeSpanHandler> CEFWebViewControl::GetLifeSpanHandler()
{
    return this;
}

void CEFWebViewControl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    pageLoaded = true;
}

bool CEFWebViewControl::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefRequest> request,
                                       bool isRedirect)
{
    String url = request->GetURL();

    // Always allow loading of URL from OpenURL method or if delegate is not set
    if (url == requestedUrl || delegate == nullptr)
    {
        return false;
    }

    IUIWebViewDelegate::eAction action;
    bool isRedirectedByMouseClick = !isRedirect && request->GetResourceType() == RT_MAIN_FRAME;
    action = delegate->URLChanged(&webView, url, isRedirectedByMouseClick);

    if (action == IUIWebViewDelegate::PROCESS_IN_WEBVIEW)
    {
        return false;
    }
    else if (action == IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER)
    {
        DAVA::OpenURL(url);
    }
    return true;
}

bool CEFWebViewControl::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      const CefString& targetUrl,
                                      const CefString& targetFrameName,
                                      WindowOpenDisposition targetDisposition,
                                      bool userGesture,
                                      const CefPopupFeatures& popupFeatures,
                                      CefWindowInfo& windowInfo,
                                      CefRefPtr<CefClient>& client,
                                      CefBrowserSettings& settings,
                                      bool* noJavascriptAccess)
{
    // Disallow popups
    LoadURL(targetUrl, false);
    return true;
}

void CEFWebViewControl::LoadURL(const String& url, bool clearSurface)
{
    StopLoading();
    if (clearSurface)
    {
        webPageRender->ClearRenderSurface();
    }
    cefBrowser->GetMainFrame()->LoadURL(url);
}

void CEFWebViewControl::LoadHtml(const CefString& html, const CefString& url)
{
    requestedUrl = "";
    CefRefPtr<CefFrame> frame = cefBrowser->GetMainFrame();

    // loading of "about:blank" is needed for loading string
    frame->LoadURL("about:blank");
    frame->LoadString(html, url);
}

void CEFWebViewControl::StopLoading()
{
    if (cefBrowser->IsLoading())
    {
        cefBrowser->StopLoad();
    }
}

enum class eKeyModifiers : int32
{
    NONE = 0,
    SHIFT_DOWN = 1 << 0,
    CONTROL_DOWN = 1 << 1,
    ALT_DOWN = 1 << 2,
    LEFT_MOUSE_BUTTON = 1 << 3,
    MIDDLE_MOUSE_BUTTON = 1 << 4,
    RIGHT_MOUSE_BUTTON = 1 << 5,

    IS_LEFT = 1 << 24,
    IS_RIGHT = 1 << 25,
};

const Vector<int32> ModifiersDAVAToCef
{
  cef_event_flags_t::EVENTFLAG_NONE,
  cef_event_flags_t::EVENTFLAG_SHIFT_DOWN,
  cef_event_flags_t::EVENTFLAG_CONTROL_DOWN,
  cef_event_flags_t::EVENTFLAG_ALT_DOWN,
  cef_event_flags_t::EVENTFLAG_LEFT_MOUSE_BUTTON,
  cef_event_flags_t::EVENTFLAG_MIDDLE_MOUSE_BUTTON,
  cef_event_flags_t::EVENTFLAG_RIGHT_MOUSE_BUTTON,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  cef_event_flags_t::EVENTFLAG_IS_LEFT,
  cef_event_flags_t::EVENTFLAG_IS_RIGHT,
  0, 0, 0, 0, 0, 0
};

eKeyModifiers GetKeyModifier()
{
    int32 modifier = 0;
    KeyboardDevice& keyDevice = InputSystem::Instance()->GetKeyboard();
    for (Key keyIter = Key::UNKNOWN; keyIter != Key::TOTAL_KEYS_COUNT;)
    {
        if (keyDevice.IsKeyPressed(keyIter))
        {
            switch (keyIter)
            {
            case DAVA::Key::LSHIFT:
                //                 modifier |= static_cast<int32>(eKeyModifiers::IS_LEFT);
                modifier |= static_cast<int32>(eKeyModifiers::SHIFT_DOWN);
                break;
            case DAVA::Key::RSHIFT:
                //                 modifier |= static_cast<int32>(eKeyModifiers::IS_RIGHT);
                modifier |= static_cast<int32>(eKeyModifiers::SHIFT_DOWN);
                break;
            case DAVA::Key::LCTRL:
                //                 modifier |= static_cast<int32>(eKeyModifiers::IS_LEFT);
                modifier |= static_cast<int32>(eKeyModifiers::CONTROL_DOWN);
                break;
            case DAVA::Key::RCTRL:
                //                 modifier |= static_cast<int32>(eKeyModifiers::IS_RIGHT);
                modifier |= static_cast<int32>(eKeyModifiers::CONTROL_DOWN);
                break;
            case DAVA::Key::LALT:
                //                 modifier |= static_cast<int32>(eKeyModifiers::IS_LEFT);
                modifier |= static_cast<int32>(eKeyModifiers::ALT_DOWN);
                break;
            case DAVA::Key::RALT:
                //                 modifier |= static_cast<int32>(eKeyModifiers::IS_RIGHT);
                modifier |= static_cast<int32>(eKeyModifiers::ALT_DOWN);
                break;
            case DAVA::Key::CAPSLOCK:
                break;
            case DAVA::Key::NUMLOCK:
                break;
            }
        }
        keyIter = static_cast<Key>(static_cast<int32>(keyIter) + 1);
    }
    return static_cast<eKeyModifiers>(modifier);
}

int32 ConvertDAVAModifiersToCef(eKeyModifiers modifier)
{
    int32 cefModifier = 0;
    int32 davaModifier = static_cast<int32>(modifier);
    int32 davaIter = 0;
    if (0 == davaModifier)
    {
        return cefModifier;
    }

    int32 iter = 0, endIter = sizeof(eKeyModifiers) - 1;
    while (iter != endIter)
    {
        davaIter = 1 << iter;
        if (0 != (davaModifier & davaIter))
        {
            cefModifier |= ModifiersDAVAToCef[int32(eKeyModifiers(davaIter))];
        }
        iter++;
    }
    return cefModifier;
}

int32 ConvertMouseTypeDavaToCef(UIEvent* input)
{
    int32 mouseType = 0;
    if (input->mouseButton == UIEvent::MouseButton::LEFT)
    {
        mouseType = cef_mouse_button_type_t::MBT_LEFT;
    }
    else if (input->mouseButton == UIEvent::MouseButton::MIDDLE)
    {
        mouseType = cef_mouse_button_type_t::MBT_MIDDLE;
    }
    else if (input->mouseButton == UIEvent::MouseButton::RIGHT)
    {
        mouseType = cef_mouse_button_type_t::MBT_RIGHT;
    }
    return mouseType;
}

void CEFWebViewControl::Input(UIEvent* currentInput)
{
    switch (currentInput->device)
    {
    case DAVA::UIEvent::Device::MOUSE:
        webViewOffSet = webView.GetAbsolutePosition();
        webViewOffSet.dx = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(webViewOffSet.dx);
        webViewOffSet.dy = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(webViewOffSet.dy);
        switch (currentInput->phase)
        {
        case DAVA::UIEvent::Phase::BEGAN:
        case DAVA::UIEvent::Phase::ENDED:
            OnMouseClick(currentInput);
            break;
        case DAVA::UIEvent::Phase::MOVE:
        case DAVA::UIEvent::Phase::DRAG:
            OnMouseMove(currentInput);
            break;
        case DAVA::UIEvent::Phase::WHEEL:
            OnMouseWheel(currentInput);
            break;
        default:
            break;
        }
        break;
    case DAVA::UIEvent::Device::KEYBOARD:
        OnKey(currentInput);
        break;
    case DAVA::UIEvent::Device::TOUCH_SURFACE:
        break;
    case DAVA::UIEvent::Device::TOUCH_PAD:
        break;
    default:
        break;
    }
}

void CEFWebViewControl::OnMouseClick(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
    clickEvent.x = static_cast<int>(input->physPoint.dx - webViewOffSet.dx);
    clickEvent.y = static_cast<int>(input->physPoint.dy - webViewOffSet.dy);
    clickEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    CefBrowserHost::MouseButtonType type = static_cast<CefBrowserHost::MouseButtonType>(ConvertMouseTypeDavaToCef(input));
    bool mouseUp = (input->phase == UIEvent::Phase::ENDED);
    int clickCount = input->tapCount;
    host->SendFocusEvent(true);
    host->SendMouseClickEvent(clickEvent, type, mouseUp, clickCount);
}

void CEFWebViewControl::OnMouseMove(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
    clickEvent.x = static_cast<int>(input->physPoint.dx - webViewOffSet.dx);
    clickEvent.y = static_cast<int>(input->physPoint.dy - webViewOffSet.dy);
    clickEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    bool mouseLeave = false;
    host->SendMouseMoveEvent(clickEvent, mouseLeave);
}

void CEFWebViewControl::OnMouseWheel(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
    clickEvent.x = static_cast<int>(webViewOffSet.dx);
    clickEvent.y = static_cast<int>(webViewOffSet.dy);
    clickEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    int deltaX = static_cast<int>(input->wheelDelta.x * WHEEL_DELTA);
    int deltaY = static_cast<int>(input->wheelDelta.y * WHEEL_DELTA);
    host->SendMouseWheelEvent(clickEvent, deltaX, deltaY);
}

int32 GetCefKeyType(UIEvent* input)
{
    int32 keyType = 0;
    switch (input->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
        keyType = cef_key_event_type_t::KEYEVENT_RAWKEYDOWN;
        //         keyType = cef_key_event_type_t::KEYEVENT_KEYDOWN;
        break;
    case UIEvent::Phase::KEY_DOWN_REPEAT:
        break;
    case UIEvent::Phase::KEY_UP:
        keyType = cef_key_event_type_t::KEYEVENT_KEYUP;
        break;
    case UIEvent::Phase::CHAR:
        keyType = cef_key_event_type_t::KEYEVENT_CHAR;
        break;
    case UIEvent::Phase::CHAR_REPEAT:
        keyType = cef_key_event_type_t::KEYEVENT_CHAR;
        break;
    default:
        break;
    }
    return keyType;
}

void CEFWebViewControl::OnKey(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefKeyEvent keyEvent;
    keyEvent.type = static_cast<cef_key_event_type_t>(GetCefKeyType(input));
    keyEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    if (UIEvent::Phase::CHAR == input->phase || UIEvent::Phase::CHAR == input->phase)
    {
        keyEvent.windows_key_code = input->keyChar;
    }
    else if (UIEvent::Phase::KEY_DOWN == input->phase || UIEvent::Phase::KEY_UP == input->phase)
    {
        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
        keyEvent.windows_key_code = keyboard.GetSystemKeyForDavaKey(input->key);

// TODO: remove this conversion from CorePlatformWin32
#ifdef __DAVAENGINE_WIN32__
        keyEvent.windows_key_code ^= 0x100;
#endif
    }
    else
    {
        return;
    }
    host->SendKeyEvent(keyEvent);
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
