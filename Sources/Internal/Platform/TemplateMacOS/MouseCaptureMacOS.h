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

#ifndef __FRAMEWORK__MOUSECAPTUREMACOS__
#define __FRAMEWORK__MOUSECAPTUREMACOS__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_MACOS__)

#include "Input/MouseCapture.h"

namespace DAVA
{
class MouseCapturePrivate : public MouseCaptureInterface
{
public:
    void SetNativePining(eMouseCaptureMode newMode) override;
    void SetCursorInCenter() override;
    bool SkipEvents() override;

private:
    bool cursorVisible = true;
    // If mouse pointer was outside window rectangle when enabling pinning mode then
    // mouse clicks are forwarded to other windows and our application loses focus.
    // So move mouse pointer to window center before enabling pinning mode.
    // Secondly, after using CGWarpMouseCursorPosition function to center mouse pointer
    // mouse move events arrive with big delta which causes mouse hopping.
    // The best solution I have investigated is to skip first N mouse move events after enabling
    // pinning mode: global variable mouseMoveSkipCounter is set to some reasonable value
    // and is checked in OpenGLView's process method to skip mouse move events
    uint32 skipMouseMoveEvents = 0;
    const uint32 SKIP_N_MOUSE_MOVE_EVENTS = 4;

    void MovePointerToWindowCenter();
    void OSXShowCursor();
    void OSXHideCursor();
};

} //  namespace DAVA

#endif //  __DAVAENGINE_MACOS__

#endif //  __FRAMEWORK__MOUSECAPTUREMACOS__