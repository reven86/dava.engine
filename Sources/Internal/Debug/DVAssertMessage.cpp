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


#include "Debug/DVAssertMessage.h"
#include "Core/Core.h"

using namespace DAVA;

namespace
{
DAVA::Atomic<bool> messageDisplayed(false);
}

#if defined(ENABLE_ASSERT_MESSAGE)

bool DVAssertMessage::ShowMessage(eModalType modalType, const char8 * text, ...)
{
    bool userClickBreak = false;
	// we don't need to show assert window for console mode
	if(Core::Instance()->IsConsoleMode()) return userClickBreak; // TODO what to do here? is loging only in console mode?

	va_list vl;
	va_start(vl, text);
	
	char tmp[4096] = {0};
	// sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer. 
	vsnprintf(tmp, sizeof(tmp)-2, text, vl);
	strcat(tmp, "\n");
    messageDisplayed = true;
    userClickBreak = InnerShow(modalType, tmp);
    messageDisplayed = false;
    va_end(vl);

    return userClickBreak;
}


#else

bool DVAssertMessage::ShowMessage(eModalType /*modalType*/, const char8 * /*text*/, ...)
{
	// Do nothing here.
    return false;
}

#endif	// ENABLE_ASSERT_MESSAGE

bool DVAssertMessage::IsMessageDisplayed()
{
    return messageDisplayed;
}
