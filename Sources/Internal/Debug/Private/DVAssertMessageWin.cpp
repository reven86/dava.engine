#include "Base/Platform.h"

#if !defined(__DAVAENGINE_COREV2__)

#include "Utils/UTF8Utils.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Debug/DVAssertMessage.h"
#include "Logger/Logger.h"

namespace DAVA
{
bool DVAssertMessage::InnerShow(eModalType modalType, const char* content)
{
    // Modal Type is ignored by Win32.
    const int flags = MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST | (modalType == TRY_NONMODAL ? MB_APPLMODAL : MB_TASKMODAL);
    WideString wContent;
    UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(content), static_cast<DAVA::int32>(strlen(content)), wContent);
    int buttonId = ::MessageBoxW(HWND_DESKTOP, wContent.c_str(), L"Assert", flags);
    switch (buttonId)
    {
    case IDABORT:
        return true; // break executions
    case IDIGNORE:
        return false; // continue execution
    case IDRETRY:
        return InnerShow(modalType, content);
    default:
        // should never happen!
        Logger::Error(
        "Return button id(%d) unknown! Error during handle assert message",
        buttonId);
        return true;
    }
}

} // namespace DAVA

#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssertMessage.h"

#include "Concurrency/Mutex.h"
#include "Concurrency/ConditionVariable.h"
#include "Concurrency/LockGuard.h"
#include "Engine/Engine.h"
#include "Utils/UTF8Utils.h"

#if !defined(__DAVAENGINE_COREV2__)
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/DispatcherWinUAP.h"
#endif

namespace DAVA
{
bool DVAssertMessage::InnerShow(eModalType /*modalType*/, const char* content)
{
#if defined(__DAVAENGINE_COREV2__)
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::UI::Popups;

    enum eUserChoice
    {
        USER_HASNT_CHOOSE_YET,
        USER_CHOOSE_CONTINUE,
        USER_CHOOSE_BREAK
    } userChoice = USER_HASNT_CHOOSE_YET;

    Mutex mutex;
    ConditionVariable cv;

    bool inUiThread = CoreWindow::GetForCurrentThread() != nullptr;
    Platform::String ^ text = ref new ::Platform::String(UTF8Utils::EncodeToWideString(content).c_str());
    auto f = [&userChoice, &mutex, &cv, text, inUiThread]() {
        auto cmdHandler = [&userChoice, &mutex, &cv, inUiThread](IUICommand ^ uiCmd) {
            if (inUiThread)
            {
                userChoice = (0 == ::Platform::String::CompareOrdinal(uiCmd->Label, L"break")) ? USER_CHOOSE_BREAK : USER_CHOOSE_CONTINUE;
            }
            else
            {
                {
                    LockGuard<Mutex> lock(mutex);
                    userChoice = (0 == ::Platform::String::CompareOrdinal(uiCmd->Label, L"break")) ? USER_CHOOSE_BREAK : USER_CHOOSE_CONTINUE;
                }
                cv.NotifyOne();
            }
        };

        MessageDialog ^ msg = ref new MessageDialog(text);

        UICommand ^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler(cmdHandler));
        msg->Commands->Append(continueCommand);

        if (!inUiThread)
        {
            UICommand ^ breakCommand = ref new UICommand("Break", ref new UICommandInvokedHandler(cmdHandler));
            breakCommand->Label = "break";
            msg->Commands->Append(breakCommand);
        }

        msg->DefaultCommandIndex = 0;
        msg->CancelCommandIndex = 0;

        msg->ShowAsync(); // This is always async call
    };

    UniqueLock<Mutex> lock(mutex);
    RunOnUIThread(f);
    if (!inUiThread)
    {
        cv.Wait(lock, [&userChoice]() { return userChoice != USER_HASNT_CHOOSE_YET; });
    }

    return userChoice == USER_CHOOSE_BREAK;
#else
    using namespace Windows::UI::Popups;

    enum eUserChoice
    {
        USER_HASNT_CHOOSE_YET,
        USER_CHOOSE_CONTINUE,
        USER_CHOOSE_BREAK
    } userChoice = USER_HASNT_CHOOSE_YET;

    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    // Depending on what thread assertion has occured we should take different actions:
    //  - for UI thread
    //      MessageDialog always run asynchronously so breaking has no sense so dialog has only one button for continuation
    //      As we asserting on UI thread we can simply show dialog
    //  - for main and other threads
    //      MessageDialog must be run only on UI thread, so RunOnUIThread is used
    //      Also we block asserting thread to be able to retrieve user response: continue or break
    WideString wContent;
    UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(content), static_cast<DAVA::int32>(strlen(content)), wContent);
    Platform::String ^ text = ref new Platform::String(wContent.c_str());
    if (!core->IsUIThread())
    {
        // If MainThreadDispatcher is in blocking call to UI thread we cannot show dialog box
        // performing this action can lead to deadlock or system simply discards dialog box without showing it
        //
        // So we simply tell caller to always debug break on DVASSERT to notify programmer about problems
        if (core->XamlApplication()->MainThreadDispatcher()->InBlockingCall())
        {
            return true;
        }

        Mutex mutex;
        ConditionVariable cv;

        auto f = [text, &userChoice, &cv, &mutex]()
        {
            auto cmdHandler = [&cv, &mutex, &userChoice](IUICommand ^ uiCmd)
            {
                {
                    LockGuard<Mutex> lock(mutex);
                    userChoice = (0 == Platform::String::CompareOrdinal(uiCmd->Label, L"break")) ? USER_CHOOSE_BREAK : USER_CHOOSE_CONTINUE;
                }
                cv.NotifyOne();
            };

            UICommand ^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler(cmdHandler));
            UICommand ^ breakCommand = ref new UICommand("Break", ref new UICommandInvokedHandler(cmdHandler));
            breakCommand->Label = "break";

            MessageDialog ^ msg = ref new MessageDialog(text);
            msg->Commands->Append(continueCommand);
            msg->Commands->Append(breakCommand);
            msg->DefaultCommandIndex = 0;
            msg->CancelCommandIndex = 0;

            msg->ShowAsync(); // This is always async call
        };

        UniqueLock<Mutex> lock(mutex);
        core->RunOnUIThread(f);
        cv.Wait(lock, [&userChoice]() { return userChoice != USER_HASNT_CHOOSE_YET; });
    }
    else
    {
        UICommand ^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler([](IUICommand ^ ) {}));

        MessageDialog ^ msg = ref new MessageDialog(text);
        msg->Commands->Append(continueCommand);
        msg->DefaultCommandIndex = 0;

        userChoice = USER_CHOOSE_CONTINUE;
        msg->ShowAsync(); // This is always async call
    }
    return USER_CHOOSE_BREAK == userChoice;
#endif
}

} // namespace DAVA

#endif // defined (__DAVAENGINE_WIN_UAP__)
#endif // !defined(__DAVAENGINE_COREV2__)
