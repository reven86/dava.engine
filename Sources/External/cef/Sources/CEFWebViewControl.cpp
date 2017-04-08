#include <cef/include/cef_browser.h>
#include <regex>

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "UI/UIEvent.h"
#include "UI/UIControlSystem.h"
#include "UI/UIWebView.h"
#include "CEFWebViewControl.h"
#include "CEFDavaResourceHandler.h"
#include "Utils/Utils.h"

namespace DAVA
{
struct CookieTerminator : public CefCookieVisitor
{
    IMPLEMENT_REFCOUNTING(CookieTerminator);

    bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) override
    {
        deleteCookie = true;
        return true;
    }
};

class CookieHarvester : public CefCookieVisitor
{
    IMPLEMENT_REFCOUNTING(CookieHarvester);

public:
    CookieHarvester(const String& name = "")
        : specificName(name)
    {
    }

    bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) override
    {
        deleteCookie = false;

        String name = CefString(&cookie.name).ToString();
        if (specificName.empty() || specificName == name)
        {
            cookies[name] = CefString(&cookie.value).ToString();
        }

        return true;
    }

    const Map<String, String>& GetCookies() const
    {
        return cookies;
    }

private:
    String specificName;
    Map<String, String> cookies;
};

#if defined(__DAVAENGINE_COREV2__)
CEFWebViewControl::CEFWebViewControl(Window* w, UIWebView& uiWebView)
    : window(w)
    , webView(uiWebView)
{
}
#else
CEFWebViewControl::CEFWebViewControl(UIWebView& uiWebView)
    : webView(uiWebView)
{
}
#endif

void CEFWebViewControl::Initialize(const Rect& rect)
{
#if defined(__DAVAENGINE_COREV2__)
    Engine::Instance()->PrimaryWindow()->sizeChanged.Connect(this, &CEFWebViewControl::OnWindowSizeChanged);
    scale = window->GetDPI() / defaultDpi;
    webPageRender = new CEFWebPageRender(window, scale);
#else
    webPageRender = new CEFWebPageRender;
#endif

    CefWindowInfo windowInfo;
    windowInfo.windowless_rendering_enabled = 1;
    windowInfo.transparent_painting_enabled = 1;

    CefBrowserSettings settings;
    cefBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings, nullptr);
}

void CEFWebViewControl::Deinitialize()
{
#if defined(__DAVAENGINE_COREV2__)
    // TODO: Deinitialize is called when UIScreen with webview is destroyed. Singletons are deleted at the end of life and if app is closing when UIScreen with webview active, window is null
    Window* primaryWindow = Engine::Instance()->PrimaryWindow();
    if (primaryWindow != nullptr)
    {
        primaryWindow->sizeChanged.Disconnect(this);
    }
#endif

    // Close browser and release object
    // If we don't release cefBrowser, dtor of CEFWebViewControl will never be invoked
    cefBrowser->GetHost()->CloseBrowser(true);
    cefBrowser = nullptr;
    webPageRender->ShutDown();
    webPageRender = nullptr;
}

void CEFWebViewControl::OpenURL(const String& url)
{
    requestedUrl = url;
    LoadURL(url, true);
}

void CEFWebViewControl::LoadHtmlString(const WideString& htmlString)
{
    StopLoading();
    LoadHtml(htmlString, CEFDavaResourceHandler::FilePathToDavaUrl("~res:/"));
}

void CEFWebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    StopLoading();

    String fileUrl = CEFDavaResourceHandler::FilePathToDavaUrl(basePath);
    LoadHtml(htmlString, fileUrl);
}

void CEFWebViewControl::ExecuteJScript(const String& scriptString)
{
    cefBrowser->GetMainFrame()->ExecuteJavaScript(scriptString, "", 0);
}

void CEFWebViewControl::DeleteCookies(const String& url)
{
    CefRefPtr<CefCookieManager> cookieMan = CefCookieManager::GetGlobalManager(nullptr);
    cookieMan->VisitUrlCookies(url, false, new CookieTerminator);
}

String CEFWebViewControl::GetCookie(const String& url, const String& name) const
{
    CefRefPtr<CefCookieManager> cookieMan = CefCookieManager::GetGlobalManager(nullptr);
    CefRefPtr<CookieHarvester> harvester = new CookieHarvester(name);
    cookieMan->VisitUrlCookies(url, false, harvester);

    const Map<String, String>& cookies = harvester->GetCookies();
    auto iter = cookies.find(name);
    return iter != cookies.end() ? iter->second : "";
}

Map<String, String> CEFWebViewControl::GetCookies(const String& url) const
{
    CefRefPtr<CefCookieManager> cookieMan = CefCookieManager::GetGlobalManager(nullptr);
    CefRefPtr<CookieHarvester> harvester = new CookieHarvester;
    cookieMan->VisitUrlCookies(url, false, harvester);

    return harvester->GetCookies();
}

void CEFWebViewControl::SetRect(const Rect& rect)
{
    // WORKAROUND PART 1 BEGIN -->
    // In off-screen rednering mode, when resizing webView on devices with
    // screen scaling other than 1.0f it can be repaint with bad size (without taking into
    // account that scale). Possible it is a CEF bug... or we don't understand how to
    // use off-screen rendering right.
    // But when we switch visibility off/on this cause CEF to invoke OnPaint with right sizes,
    // so we can workaround this problem.
    if (webPageRender->IsVisible())
    {
        cefBrowser->GetHost()->WasHidden(true);
    }
    // <--- WORKAROUND PART 1 END

    webPageRender->SetViewRect(rect);
    cefBrowser->GetHost()->WasResized();

    // WORKAROUND LAST PART 2 BEGIN -->
    // See PART 1 description higher
    if (webPageRender->IsVisible())
    {
        cefBrowser->GetHost()->WasHidden(false);
    }
    // <-- WORKAROUND PART 2 END
}

void CEFWebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    webPageRender->SetVisible(isVisible);
    cefBrowser->GetHost()->WasHidden(!isVisible);
}

void CEFWebViewControl::SetBackgroundTransparency(bool enabled)
{
    webPageRender->SetBackgroundTransparency(enabled);
    cefBrowser->GetHost()->Invalidate(PET_VIEW);
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

void CEFWebViewControl::Draw(const UIGeometricData& geometricData)
{
    webPageRender->GetContentBackground()->Draw(geometricData);
}

void CEFWebViewControl::Update()
{
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

    // Disallow email processing
    if (IsEmail(url))
    {
        return true;
    }

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
    // Ops, chromium crashes on empty url
    if (!url.empty())
    {
        StopLoading();
        if (clearSurface)
        {
            webPageRender->ClearRenderSurface();
        }
        cefBrowser->GetMainFrame()->LoadURL(url);
    }
    else
    {
        Logger::Error("CEFWebViewControl::LoadURL empty URL has come");
    }
}

void CEFWebViewControl::LoadHtml(const CefString& html, const CefString& url)
{
    if (!html.empty())
    {
        requestedUrl = "";
        CefRefPtr<CefFrame> frame = cefBrowser->GetMainFrame();

        // loading of "about:blank" is needed for loading string
        frame->LoadURL("about:blank");
        frame->LoadString(html, url);
    }
    else
    {
        Logger::Error("CEFWebViewControl::LoadHtml empty HTML has come");
    }
}

void CEFWebViewControl::StopLoading()
{
    if (cefBrowser->IsLoading())
    {
        cefBrowser->StopLoad();
    }
}

bool CEFWebViewControl::IsEmail(const String& url)
{
    const char* emailRegex = "^[-a-z0-9!#$%&'*+/=?^_`{|}~]+(\\.[-a-z0-9!#$%&'*+/=?^_`{|}~]+)*@"
                             "([a-z0-9]([-a-z0-9]{0,61}[a-z0-9])?\\.)*(aero|arpa|asia|biz|cat|"
                             "com|coop|edu|gov|info|int|jobs|mil|mobi|museum|name|net|org|pro|"
                             "tel|travel|[a-z][a-z])$";

    bool result = url.find("mailto:") == 0 || std::regex_match(url, std::regex(emailRegex));
    return result;
}

namespace CEFDetails
{
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
                modifier |= static_cast<int32>(eKeyModifiers::SHIFT_DOWN);
                break;
            case DAVA::Key::RSHIFT:
                modifier |= static_cast<int32>(eKeyModifiers::SHIFT_DOWN);
                break;
            case DAVA::Key::LCTRL:
                modifier |= static_cast<int32>(eKeyModifiers::CONTROL_DOWN);
                break;
            case DAVA::Key::RCTRL:
                modifier |= static_cast<int32>(eKeyModifiers::CONTROL_DOWN);
                break;
            case DAVA::Key::LALT:
                modifier |= static_cast<int32>(eKeyModifiers::ALT_DOWN);
                break;
            case DAVA::Key::RALT:
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
    if (input->mouseButton == eMouseButtons::LEFT)
    {
        mouseType = cef_mouse_button_type_t::MBT_LEFT;
    }
    else if (input->mouseButton == eMouseButtons::MIDDLE)
    {
        mouseType = cef_mouse_button_type_t::MBT_MIDDLE;
    }
    else if (input->mouseButton == eMouseButtons::RIGHT)
    {
        mouseType = cef_mouse_button_type_t::MBT_RIGHT;
    }
    return mouseType;
}

int32 GetCefKeyType(UIEvent* input)
{
    int32 keyType = 0;
    switch (input->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
        keyType = cef_key_event_type_t::KEYEVENT_RAWKEYDOWN;
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
} // namespace CEFDetails

void CEFWebViewControl::Input(UIEvent* currentInput)
{
    switch (currentInput->device)
    {
    case eInputDevices::MOUSE:
        webViewPos = webView.GetAbsolutePosition();
#if defined(__DAVAENGINE_COREV2__)
        webViewPos = window->GetUIControlSystem()->vcs->ConvertVirtualToInput(webViewPos);
#endif
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
    case eInputDevices::KEYBOARD:
        OnKey(currentInput);
        break;
    case eInputDevices::TOUCH_SURFACE:
        break;
    case eInputDevices::TOUCH_PAD:
        break;
    default:
        break;
    }
}

void CEFWebViewControl::OnWindowSizeChanged(Window* window, Size2f, Size2f)
{
    if (webPageRender->IsVisible())
    {
        cefBrowser->GetHost()->WasHidden(true);
    }
    // <--- WORKAROUND PART 1 END

#if defined(__DAVAENGINE_COREV2__)
    scale = window->GetDPI() / defaultDpi;
    webPageRender->SetScale(scale);
    cefBrowser->GetHost()->WasResized();
#endif
    cefBrowser->GetHost()->NotifyScreenInfoChanged();

    // WORKAROUND LAST PART 2 BEGIN -->
    // See PART 1 description higher
    if (webPageRender->IsVisible())
    {
        cefBrowser->GetHost()->WasHidden(false);
    }
}

void CEFWebViewControl::OnMouseClick(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
#if defined(__DAVAENGINE_COREV2__)
    clickEvent.x = static_cast<int>(input->physPoint.x - webViewPos.x);
    clickEvent.y = static_cast<int>(input->physPoint.y - webViewPos.y);
#else
    clickEvent.x = static_cast<int>(input->point.dx - webViewPos.dx);
    clickEvent.y = static_cast<int>(input->point.dy - webViewPos.dy);
#endif
    clickEvent.modifiers = ConvertDAVAModifiersToCef(CEFDetails::GetKeyModifier());
    int32 mouseType = CEFDetails::ConvertMouseTypeDavaToCef(input);
    CefBrowserHost::MouseButtonType type = static_cast<CefBrowserHost::MouseButtonType>(mouseType);
    bool mouseUp = (input->phase == UIEvent::Phase::ENDED);
    int clickCount = input->tapCount;
    host->SendFocusEvent(true);
    host->SendMouseClickEvent(clickEvent, type, mouseUp, clickCount);
}

void CEFWebViewControl::OnMouseMove(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
#if defined(__DAVAENGINE_COREV2__)
    clickEvent.x = static_cast<int>(input->physPoint.x - webViewPos.x);
    clickEvent.y = static_cast<int>(input->physPoint.y - webViewPos.y);
#else
    clickEvent.x = static_cast<int>(input->point.dx - webViewPos.dx);
    clickEvent.y = static_cast<int>(input->point.dy - webViewPos.dy);
#endif
    clickEvent.modifiers = ConvertDAVAModifiersToCef(CEFDetails::GetKeyModifier());
    bool mouseLeave = false;
    host->SendMouseMoveEvent(clickEvent, mouseLeave);
}

void CEFWebViewControl::OnMouseWheel(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
#if defined(__DAVAENGINE_COREV2__)
    clickEvent.x = static_cast<int>(input->physPoint.x - webViewPos.x);
    clickEvent.y = static_cast<int>(input->physPoint.y - webViewPos.y);
#else
    clickEvent.x = static_cast<int>(input->point.dx - webViewPos.dx);
    clickEvent.y = static_cast<int>(input->point.dy - webViewPos.dy);
#endif
    clickEvent.modifiers = ConvertDAVAModifiersToCef(CEFDetails::GetKeyModifier());
    int deltaX = static_cast<int>(input->wheelDelta.x * WHEEL_DELTA);
    int deltaY = static_cast<int>(input->wheelDelta.y * WHEEL_DELTA);
    host->SendMouseWheelEvent(clickEvent, deltaX, deltaY);
}

void CEFWebViewControl::OnKey(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefKeyEvent keyEvent;
    keyEvent.type = static_cast<cef_key_event_type_t>(CEFDetails::GetCefKeyType(input));
    keyEvent.modifiers = ConvertDAVAModifiersToCef(CEFDetails::GetKeyModifier());
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
        keyEvent.windows_key_code &= ~0x100;
#endif
    }
    else
    {
        return;
    }
    host->SendKeyEvent(keyEvent);
}

} // namespace DAVA
