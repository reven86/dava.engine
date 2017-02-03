#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN32__)

#include "Base/BaseTypes.h"
#include "Concurrency/Thread.h"
#include "Engine/Engine.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Win32/Window/WindowBackendWin32.h"
#include "Debug/DVAssert.h"
#include "Utils/UTF8Utils.h"

#if defined(__DAVAENGINE_QT__)
#include "Engine/PlatformApi.h"
#endif

namespace DAVA
{
namespace Debug
{
class MessageBoxHook
{
public:
    MessageBoxHook();
    ~MessageBoxHook();

    int Show(HWND hwndParent, WideString caption, WideString message, Vector<WideString> buttonNames, int defaultButton);

private:
    struct DialogButtonInfo
    {
        int id;
        HWND hwnd;
    };
    struct IdToIndexMap
    {
        int id;
        int index;
    };

    void PrepareButtons(HWND hwnd);

    static LRESULT CALLBACK HookInstaller(int code, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK HookWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static BOOL CALLBACK EnumChildProcStart(HWND hwnd, LPARAM lparam);

    HHOOK hhook = nullptr;
    WNDPROC oldWndProc = nullptr;

    Vector<WideString> buttons;
    size_t buttonCount = 0;

    static MessageBoxHook* pthis;
    static const int buttonTypes[3];
    static const int defaultButtons[3];
    static const IdToIndexMap map[6];
};

MessageBoxHook* MessageBoxHook::pthis = nullptr;

const int MessageBoxHook::buttonTypes[3] = {
    MB_OK,
    MB_YESNO,
    MB_ABORTRETRYIGNORE,
};

const int MessageBoxHook::defaultButtons[3] = {
    MB_DEFBUTTON1,
    MB_DEFBUTTON2,
    MB_DEFBUTTON3,
};

const MessageBoxHook::IdToIndexMap MessageBoxHook::map[6] = {
    { IDABORT, 0 },
    { IDRETRY, 1 },
    { IDIGNORE, 2 },
    { IDYES, 0 },
    { IDNO, 1 },
    { IDCANCEL, 0 },
};

MessageBoxHook::MessageBoxHook()
{
    DVASSERT(pthis == nullptr);
    pthis = this;
}

MessageBoxHook::~MessageBoxHook()
{
    pthis = nullptr;
}

int MessageBoxHook::Show(HWND hwndParent, WideString caption, WideString message, Vector<WideString> buttonNames, int defaultButton)
{
    buttons = std::move(buttonNames);
    buttonCount = std::min<size_t>(3, buttons.size());
    defaultButton = std::max(0, std::min(2, defaultButton));

    // Install hook procedure to replace buttons names for standrd MessageBox function
    hhook = ::SetWindowsHookExW(WH_CALLWNDPROC, &MessageBoxHook::HookInstaller, nullptr, ::GetCurrentThreadId());
    const UINT style = MB_ICONEXCLAMATION | defaultButtons[defaultButton] | buttonTypes[buttonCount - 1];
    int choice = ::MessageBoxW(hwndParent, message.c_str(), caption.c_str(), style);
    ::UnhookWindowsHookEx(hhook);

    for (const IdToIndexMap& i : map)
    {
        if (choice == i.id)
            return i.index;
    }
    return 0;
}

void MessageBoxHook::PrepareButtons(HWND hwnd)
{
    Vector<DialogButtonInfo> dialogButtons;
    ::EnumChildWindows(hwnd, &MessageBoxHook::EnumChildProcStart, reinterpret_cast<LPARAM>(&dialogButtons));

    size_t n = std::min(dialogButtons.size(), buttonCount);
    for (size_t i = 0; i < n; ++i)
    {
        ::SetWindowTextW(dialogButtons[i].hwnd, buttons[i].c_str());
    }
}

LRESULT CALLBACK MessageBoxHook::HookInstaller(int code, WPARAM wparam, LPARAM lparam)
{
    if (code == HC_ACTION)
    {
        CWPSTRUCT* cwp = reinterpret_cast<CWPSTRUCT*>(lparam);
        if (cwp->message == WM_INITDIALOG)
        {
            LONG_PTR newWndProc = reinterpret_cast<LONG_PTR>(&MessageBoxHook::HookWndProc);
            pthis->oldWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtrW(cwp->hwnd, GWLP_WNDPROC, newWndProc));
        }
    }
    return ::CallNextHookEx(pthis->hhook, code, wparam, lparam);
}

LRESULT CALLBACK MessageBoxHook::HookWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT lresult = ::CallWindowProcW(pthis->oldWndProc, hwnd, message, wparam, lparam);
    if (message == WM_INITDIALOG)
    {
        pthis->PrepareButtons(hwnd);
    }
    return lresult;
}

BOOL CALLBACK MessageBoxHook::EnumChildProcStart(HWND hwnd, LPARAM lparam)
{
    Vector<DialogButtonInfo>& buttons = *reinterpret_cast<Vector<DialogButtonInfo>*>(lparam);
    int id = ::GetDlgCtrlID(hwnd);
    if (id == IDABORT || id == IDRETRY || id == IDIGNORE || id == IDYES || id == IDNO || id == IDCANCEL)
    {
        buttons.push_back({ id, hwnd });
    }
    return TRUE;
}

int MessageBox(const String& title, const String& message, const Vector<String>& buttons, int defaultButton)
{
    using namespace DAVA::Private;

    DVASSERT(0 < buttons.size() && buttons.size() <= 3);
    DVASSERT(0 <= defaultButton && defaultButton < static_cast<int>(buttons.size()));

    int result = -1;
    auto showMessageBox = [&title, &message, &buttons, defaultButton, &result]()
    {
        if (!EngineBackend::showingModalMessageBox)
        {
            Vector<WideString> wideButtons;
            wideButtons.reserve(buttons.size());
            for (const String& s : buttons)
            {
                wideButtons.push_back(UTF8Utils::EncodeToWideString(s));
            }

            EngineBackend::showingModalMessageBox = true;

#if defined(__DAVAENGINE_QT__)
            Window* primaryWindow = GetPrimaryWindow();
            if (primaryWindow != nullptr && primaryWindow->IsAlive())
                PlatformApi::Qt::AcquireWindowContext(primaryWindow);
#endif

            MessageBoxHook msgBox;
            result = msgBox.Show(::GetActiveWindow(), UTF8Utils::EncodeToWideString(title), UTF8Utils::EncodeToWideString(message), std::move(wideButtons), defaultButton);

#if defined(__DAVAENGINE_QT__)
            if (primaryWindow != nullptr && primaryWindow->IsAlive())
                PlatformApi::Qt::ReleaseWindowContext(primaryWindow);
#endif

            EngineBackend::showingModalMessageBox = false;
        }
    };

    Window* primaryWindow = GetPrimaryWindow();
    if (Thread::IsMainThread())
    {
        showMessageBox();
    }
    else if (primaryWindow != nullptr && primaryWindow->IsAlive())
    {
        primaryWindow->RunOnUIThread(showMessageBox);
    }
    return result;
}

} // namespace Debug
} // namespace DAVA

#endif // defined(__DAVAENGINE_WIN32__)
#endif // defined(__DAVAENGINE_COREV2__)
