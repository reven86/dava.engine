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

#include <cassert>
#include <Shlwapi.h>

#include "Debug/DVAssertMessage.h"
#include "FileSystem/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"

#if defined (__DAVAENGINE_WIN32__)

namespace DAVA
{

#if defined (__DAVAENGINE_WINDOWS_DESKTOP__)

bool DVAssertMessage::InnerShow(eModalType /*modalType*/, const char* content)
{
	// Modal Type is ignored by Win32.
	int buttonId = MessageBoxA(HWND_DESKTOP, content, "Assert", MB_OKCANCEL | MB_ICONEXCLAMATION);
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

#elif defined (__DAVAENGINE_WINDOWS_STORE__)

bool DVAssertMessage::InnerShow(eModalType /*modalType*/, const char* content)
{
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__MARKER__
    using namespace Windows::UI::Popups;
    
    WideString contentStr = UTF8Utils::EncodeToWideString(content);
    Platform::String^ lbl = ref new Platform::String(contentStr.c_str());
    MessageDialog^ msg = ref new MessageDialog(lbl);

    //creating commands for message dialog
    UICommand^ continueCommand = ref new UICommand(
		"OK", 
        ref new UICommandInvokedHandler([&] (IUICommand^) {} ));
    UICommand^ cancelCommand = ref new UICommand(
		"Cancel", 
        ref new UICommandInvokedHandler([&] (IUICommand^) {} ));

    msg->Commands->Append(continueCommand);
    msg->Commands->Append(cancelCommand);

	//command options
	msg->DefaultCommandIndex = 0;
	msg->CancelCommandIndex = 1;

	//show message and blocking thread
	//msg->ShowAsync();
    //assert(false);

    return false;
}

#endif // defined (__DAVAENGINE_WINDOWS_STORE__)

} // namespace DAVA

#endif //#if defined (__DAVAENGINE_WIN32__)
