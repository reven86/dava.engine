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
#if defined (__DAVAENGINE_WINDOWS__)

#include "Debug/DVAssertMessage.h"
#include "FileSystem/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"

#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

namespace DAVA
{

#if defined (__DAVAENGINE_WIN32__)

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
        DAVA::Logger::Instance()->Error(
            "Return button id(%d) unknown! Error during handle assert message",
            buttonId);
        return true;
    }
}

#elif defined (__DAVAENGINE_WIN_UAP__)

bool DVAssertMessage::InnerShow(eModalType /*modalType*/, const char* content)
{
    using namespace Windows::UI::Popups;

    bool issueDebugBreak = false;
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    // Depending on what thread assertion has occured we should take different actions:
    //  - for UI thread
    //      MessageDialog always run asynchronously so breaking has no sense so dialog has only one button for continuation
    //      As we asserting on UI thread we can simply show dialog
    //  - for main and other threads
    //      MessageDialog must be run only on UI thread, so RunOnUIThread is used
    //      Also we block asserting thread to be able to retrieve user response: continue or break
    if (!core->IsUIThread())
    {
        Spinlock lock;  // TODO: maybe choose mutex to allow waiting thread sleep
        auto f = [content, &issueDebugBreak, &lock]() {
            using namespace Windows::UI::Popups;

            WideString contentStr = UTF8Utils::EncodeToWideString(content);
            Platform::String^ text = ref new Platform::String(contentStr.c_str());
            MessageDialog^ msg = ref new MessageDialog(text);

            UICommand^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler([&lock](IUICommand^) { lock.Unlock(); }));
            UICommand^ cancelCommand = ref new UICommand("Break", ref new UICommandInvokedHandler([&lock, &issueDebugBreak](IUICommand^) { issueDebugBreak = true; lock.Unlock(); }));
            msg->Commands->Append(continueCommand);
            msg->Commands->Append(cancelCommand);
            msg->DefaultCommandIndex = 0;
            msg->CancelCommandIndex = 0;

            msg->ShowAsync();   // This is always async call
        };

        lock.Lock();
        core->RunOnUIThread(f);
        lock.Lock();
    }
    else
    {
        WideString contentStr = UTF8Utils::EncodeToWideString(content);
        Platform::String^ text = ref new Platform::String(contentStr.c_str());
        MessageDialog^ msg = ref new MessageDialog(text);

        UICommand^ continueCommand = ref new UICommand("Continue", ref new UICommandInvokedHandler([](IUICommand^) {}));
        msg->Commands->Append(continueCommand);
        msg->DefaultCommandIndex = 0;

        msg->ShowAsync();   // This is always async call
    }
    return issueDebugBreak;
}

#endif // defined (__DAVAENGINE_WIN_UAP__)

} // namespace DAVA

#endif //#if defined (__DAVAENGINE_WINDOWS__)
