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

#include "Base/Platform.h"

#if defined (__DAVAENGINE_WIN32__)

#include "Debug/DVAssertMessage.h"
#include "FileSystem/Logger.h"

namespace DAVA
{

bool DVAssertMessage::InnerShow(eModalType modalType, const char* content)
{
    // Modal Type is ignored by Win32.
    const int flags = MB_OKCANCEL | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST | (modalType == TRY_NONMODAL ? MB_APPLMODAL : MB_TASKMODAL);
    int buttonId = ::MessageBoxA(HWND_DESKTOP, content, "Assert", flags);
    switch (buttonId)
    {
    case IDCANCEL:
        return true; // break executions
    case IDOK:
        return false; // continue execution
    default:
        // should never happen!
        Logger::Instance()->Error(
            "Return button id(%d) unknown! Error during handle assert message",
            buttonId);
        return true;
    }
}

}   // namespace DAVA

#elif defined (__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssertMessage.h"
#include "Utils/Utils.h"

#include "Concurrency/Mutex.h"
#include "Concurrency/ConditionVariable.h"
#include "Concurrency/LockGuard.h"

#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/DispatcherWinUAP.h"

namespace DAVA
{

bool DVAssertMessage::InnerShow(eModalType /*modalType*/, const char* content)
{
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
    Platform::String^ text = ref new Platform::String(StringToWString(content).c_str());
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
            auto cmdHandler = [&cv, &mutex, &userChoice](IUICommand^ uiCmd)
            {
                {
                    LockGuard<Mutex> lock(mutex);
                    userChoice = (0 == Platform::String::CompareOrdinal(uiCmd->Label, L"break")) ? USER_CHOOSE_BREAK : USER_CHOOSE_CONTINUE;
                }
                cv.NotifyOne();
            };

            UICommand^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler(cmdHandler));
            UICommand^ breakCommand = ref new UICommand("Break", ref new UICommandInvokedHandler(cmdHandler));
            breakCommand->Label = "break";

            MessageDialog^ msg = ref new MessageDialog(text);
            msg->Commands->Append(continueCommand);
            msg->Commands->Append(breakCommand);
            msg->DefaultCommandIndex = 0;
            msg->CancelCommandIndex = 0;

            msg->ShowAsync();   // This is always async call
        };

        UniqueLock<Mutex> lock(mutex);
        core->RunOnUIThread(f);
        cv.Wait(lock, [&userChoice]() { return userChoice != USER_HASNT_CHOOSE_YET; });
    }
    else
    {
        UICommand^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler([](IUICommand^) {}));

        MessageDialog^ msg = ref new MessageDialog(text);
        msg->Commands->Append(continueCommand);
        msg->DefaultCommandIndex = 0;

        userChoice = USER_CHOOSE_CONTINUE;
        msg->ShowAsync();   // This is always async call
    }
    return USER_CHOOSE_BREAK == userChoice;
}

}   // namespace DAVA

#endif // defined (__DAVAENGINE_WIN_UAP__)
