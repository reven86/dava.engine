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

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/MouseCaptureWinUAP.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

using namespace ::Windows::UI::Core;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Controls;

namespace DAVA
{
void MouseDeviceUWP::SetMode(eCaptureMode newMode)
{
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    SwapChainPanel ^ swapchain = reinterpret_cast<SwapChainPanel ^>(DAVA::Core::Instance()->GetNativeView());
    DVASSERT(swapchain);

    if (eCaptureMode::PINING == newMode)
    {
        core->RunOnUIThread([]()
                            {
                                Window::Current->CoreWindow->PointerCursor = nullptr;
                            });
    }
    else
    {
        core->RunOnUIThread([]()
                            {
                                Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
                            });
    }
}

void MouseDeviceUWP::SetCursorInCenter()
{
}

bool MouseDeviceUWP::SkipEvents()
{
    return false;
}

} // namespace DAVA

#endif //  __DAVAENGINE_WIN_UAP__