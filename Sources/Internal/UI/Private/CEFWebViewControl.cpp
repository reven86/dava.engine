#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_browser.h>
#include <cef/include/cef_scheme.h>
#include "UI/Private/CEFInterprocessMessages.h"
#include "UI/Private/CEFWebPageRender.h"

#include "UI/UIEvent.h"
#include "Input/InputSystem.h"
#include "UI/Private/CEFWebViewControl.h"
#include "UI/UIWebView.h"
#include "Utils/Utils.h"

namespace DAVA
{
WebViewControl::WebViewControl(UIWebView& uiWebView)
    : webView(uiWebView)
{
    // this object will be deleted manually, but ref counting is needed for work
    // new object in CEF system has 0 references,
    // so it will be destroyed when all owning CefRefPtr will be destroyed
    // to avoid premature destruction, increase reference count manually
    AddRef();
}

WebViewControl::~WebViewControl()
{
    cefBrowser->GetHost()->CloseBrowser(true);
    cefBrowser = nullptr;
    webPageRender = nullptr;
}

void WebViewControl::Initialize(const Rect& rect)
{
    webPageRender = new CEFWebPageRender(webView);

    CefWindowInfo windowInfo;
    windowInfo.windowless_rendering_enabled = 1;
    windowInfo.transparent_painting_enabled = 1;

    CefBrowserSettings settings;
    cefBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, this, "", settings, nullptr);
}

void WebViewControl::OpenURL(const String& url)
{
    requestedUrl = url;
    cefBrowser->GetMainFrame()->LoadURL(url);
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    LoadHtml(htmlString, "dava:/~res:/");
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    String fileUrl = "dava:/" + basePath.GetStringValue();
    LoadHtml(htmlString, fileUrl);
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    cefBrowser->GetMainFrame()->ExecuteJavaScript(scriptString, "", 0);
}

void WebViewControl::SetRect(const Rect& rect)
{
    if (rect.GetSize() != webView.GetSize())
    {
        cefBrowser->GetHost()->WasResized();
    }
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
    cefBrowser->GetHost()->WasHidden(!isVisible);
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* /*webView*/)
{
    delegate = webViewDelegate;
}

void WebViewControl::SetRenderToTexture(bool value)
{
    // Empty realization, always render to texture
}

bool WebViewControl::IsRenderToTexture() const
{
    return true;
}

void WebViewControl::Update()
{
    cefController.Update();
}

CefRefPtr<CefRenderHandler> WebViewControl::GetRenderHandler()
{
    return webPageRender;
}

CefRefPtr<CefLoadHandler> WebViewControl::GetLoadHandler()
{
    return this;
}

bool WebViewControl::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                              CefProcessId source_process,
                                              CefRefPtr<CefProcessMessage> message)
{
    // WebViewControl can process only URL loading request messages from render process
    if (source_process == PID_RENDERER &&
        cefBrowser->IsSame(browser) &&
        message->GetName() == urlLoadingRequestMessageName)
    {
        URLLoadingRequest request;
        if (!ParseUrlLoadingRequest(message, request))
        {
            return false;
        }

        OnURLLoadingRequst(request);
        return true;
    }

    return false;
}

void WebViewControl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (delegate && cefBrowser->IsSame(browser))
    {
        delegate->PageLoaded(&webView);
    }
}

void WebViewControl::LoadHtml(const CefString& html, const CefString& url)
{
    requestedUrl = "";
    CefRefPtr<CefFrame> frame = cefBrowser->GetMainFrame();

    // loading of "about:blank" is needed for loading string
    frame->LoadURL("about:blank");
    frame->LoadString(html, url);
}

void WebViewControl::OnURLLoadingRequst(const URLLoadingRequest& request)
{
    // Always allow loading of URL from OpenURL method or if delegate is not set
    if (request.url == requestedUrl || delegate == nullptr)
    {
        AllowURLLoading(request.url, request.frameID);
        return;
    }

    bool isLinkActivation = request.navigation_type == NAVIGATION_LINK_CLICKED;
    IUIWebViewDelegate::eAction action;
    action = delegate->URLChanged(&webView, request.url, isLinkActivation);

    if (action == IUIWebViewDelegate::PROCESS_IN_WEBVIEW)
    {
        AllowURLLoading(request.url, request.frameID);
    }
    else if (action == IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER)
    {
        DAVA::OpenURL(request.url);
    }
}

void WebViewControl::AllowURLLoading(const String& url, int64 frameID)
{
    URLLoadingPermit permit = { url, frameID };
    CefRefPtr<CefProcessMessage> msg = CreateUrlLoadingPermitMessage(permit);
    cefBrowser->SendProcessMessage(PID_RENDERER, msg);
}

typedef enum {
    EVENTFLAG_NONE = 0,
    EVENTFLAG_CAPS_LOCK_ON = 1 << 0,
    EVENTFLAG_SHIFT_DOWN = 1 << 1,
    EVENTFLAG_CONTROL_DOWN = 1 << 2,
    EVENTFLAG_ALT_DOWN = 1 << 3,
    EVENTFLAG_LEFT_MOUSE_BUTTON = 1 << 4,
    EVENTFLAG_MIDDLE_MOUSE_BUTTON = 1 << 5,
    EVENTFLAG_RIGHT_MOUSE_BUTTON = 1 << 6,
    // Mac OS-X command key.
    EVENTFLAG_COMMAND_DOWN = 1 << 7,
    EVENTFLAG_NUM_LOCK_ON = 1 << 8,
    EVENTFLAG_IS_KEY_PAD = 1 << 9,
    EVENTFLAG_IS_LEFT = 1 << 10,
    EVENTFLAG_IS_RIGHT = 1 << 11,
} cef_event_flags_t;

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

const Vector<int32> ModifiersDAVAToCef = {
    EVENTFLAG_NONE,
    EVENTFLAG_SHIFT_DOWN,
    EVENTFLAG_CONTROL_DOWN,
    EVENTFLAG_ALT_DOWN,
    EVENTFLAG_LEFT_MOUSE_BUTTON,
    EVENTFLAG_MIDDLE_MOUSE_BUTTON,
    EVENTFLAG_RIGHT_MOUSE_BUTTON,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    EVENTFLAG_IS_LEFT,
    EVENTFLAG_IS_RIGHT,
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
                // 				modifier |= static_cast<int32>(eKeyModifiers::IS_LEFT);
                modifier |= static_cast<int32>(eKeyModifiers::SHIFT_DOWN);
                break;
            case DAVA::Key::RSHIFT:
                // 				modifier |= static_cast<int32>(eKeyModifiers::IS_RIGHT);
                modifier |= static_cast<int32>(eKeyModifiers::SHIFT_DOWN);
                break;
            case DAVA::Key::LCTRL:
                // 				modifier |= static_cast<int32>(eKeyModifiers::IS_LEFT);
                modifier |= static_cast<int32>(eKeyModifiers::CONTROL_DOWN);
                break;
            case DAVA::Key::RCTRL:
                // 				modifier |= static_cast<int32>(eKeyModifiers::IS_RIGHT);
                modifier |= static_cast<int32>(eKeyModifiers::CONTROL_DOWN);
                break;
            case DAVA::Key::LALT:
                // 				modifier |= static_cast<int32>(eKeyModifiers::IS_LEFT);
                modifier |= static_cast<int32>(eKeyModifiers::ALT_DOWN);
                break;
            case DAVA::Key::RALT:
                // 				modifier |= static_cast<int32>(eKeyModifiers::IS_RIGHT);
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
    typedef enum {
        MBT_LEFT = 0,
        MBT_MIDDLE,
        MBT_RIGHT,
    } cef_mouse_button_type_t;

    int32 mouseType = 0;
    if (input->mouseButton == UIEvent::MouseButton::LEFT)
    {
        mouseType = 0;
    }
    else if (input->mouseButton == UIEvent::MouseButton::MIDDLE)
    {
        mouseType = 1;
    }
    else if (input->mouseButton == UIEvent::MouseButton::RIGHT)
    {
        mouseType = 2;
    }
    return mouseType;
}

void WebViewControl::Input(UIEvent* currentInput)
{
    switch (currentInput->device)
    {
    case DAVA::UIEvent::Device::MOUSE:
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
        break;
    case DAVA::UIEvent::Device::TOUCH_SURFACE:
        break;
    case DAVA::UIEvent::Device::TOUCH_PAD:
        break;
    default:
        break;
    }
}

void WebViewControl::OnMouseClick(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
    clickEvent.x = static_cast<int>(input->physPoint.dx);
    clickEvent.y = static_cast<int>(input->physPoint.dy);
    clickEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    CefBrowserHost::MouseButtonType type = static_cast<CefBrowserHost::MouseButtonType>(ConvertMouseTypeDavaToCef(input));
    bool mouseUp = (input->phase == UIEvent::Phase::ENDED);
    int clickCount = input->tapCount;
    host->SendMouseClickEvent(clickEvent, type, mouseUp, clickCount);
}

void WebViewControl::OnMouseMove(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
    clickEvent.x = static_cast<int>(input->physPoint.dx);
    clickEvent.y = static_cast<int>(input->physPoint.dy);
    clickEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    bool mouseLeave = true;
    host->SendMouseMoveEvent(clickEvent, mouseLeave);
}

void WebViewControl::OnMouseWheel(UIEvent* input)
{
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    CefMouseEvent clickEvent;
    clickEvent.x = static_cast<int>(input->physPoint.dx);
    clickEvent.y = static_cast<int>(input->physPoint.dy);
    clickEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    int deltaX = static_cast<int>(input->wheelDelta.x);
    int deltaY = static_cast<int>(input->wheelDelta.y);
    host->SendMouseWheelEvent(clickEvent, deltaX, deltaY);
}

int32 GetCefKeyType(UIEvent* input)
{
    // TODO: cef_key_event_type_t::KEYEVENT_RAWKEYDOWN
    int32 keyType = 0;
    switch (input->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
        keyType = cef_key_event_type_t::KEYEVENT_KEYDOWN;
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

int32 GetCefWinKeyCode(UIEvent* input)
{
    return 0;
}

int32 GetCefNativeKeyCode(UIEvent* input)
{
    return 0;
}

void WebViewControl::OnKey(UIEvent* input)
{
    CefKeyEvent keyEvent;
    keyEvent.type = static_cast<cef_key_event_type_t>(GetCefKeyType(input));
    keyEvent.modifiers = ConvertDAVAModifiersToCef(GetKeyModifier());
    keyEvent.windows_key_code = static_cast<int>(GetCefWinKeyCode(input));
    keyEvent.native_key_code = static_cast<int>(input->keyChar);
    keyEvent.is_system_key = 0;
    keyEvent.character = input->keyChar;
    keyEvent.unmodified_character;
    keyEvent.focus_on_editable_field;
    CefRefPtr<CefBrowserHost> host = cefBrowser->GetHost();
    host->SendKeyEvent(keyEvent);
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
