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

#include "UIKeyInputSystem.h"

#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"

#include "UI/Focus/UIFocusSystem.h"

#include "Input/InputSystem.h"

namespace DAVA
{
UIKeyInputSystem::UIKeyInputSystem(UIFocusSystem* focusSystem_)
    : focusSystem(focusSystem_)
{
}

UIKeyInputSystem::~UIKeyInputSystem()
{
    focusSystem = nullptr;
}

void UIKeyInputSystem::HandleKeyEvent(UIEvent* event)
{
    bool processed = false;

    UIEvent::Phase phase = event->phase;
    DVASSERT(phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_UP || phase == UIEvent::Phase::KEY_DOWN_REPEAT || phase == UIEvent::Phase::CHAR || phase == UIEvent::Phase::CHAR_REPEAT);

    UIControl* focusedControl = focusSystem->GetFocusedControl();
    UIControl* rootControl = focusSystem->GetRoot();

    if (!processed && focusedControl)
    {
        UIControl* c = focusedControl;
        while (c != nullptr && c != rootControl && !processed)
        {
            UIControl* tmp = SafeRetain(c);
            processed = c->SystemProcessInput(event);
            c = c->GetParent();
            SafeRelease(tmp);
        }
    }

    rootControl = focusSystem->GetRoot();
    if (!processed && rootControl)
    {
        processed = rootControl->SystemProcessInput(event);
    }

    if (!processed && (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_DOWN_REPEAT))
    {
        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
        switch (event->key)
        {
        case Key::LEFT:
            focusSystem->MoveFocusLeft();
            break;

        case Key::RIGHT:
            focusSystem->MoveFocusRight();
            break;

        case Key::UP:
            focusSystem->MoveFocusUp();
            break;

        case Key::DOWN:
            focusSystem->MoveFocusDown();
            break;

        case Key::TAB:
            if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT))
            {
                focusSystem->MoveFocusBackward();
            }
            else
            {
                focusSystem->MoveFocusForward();
            }
            break;

        default:
            // do nothing
            break;
        }
    }
}
}
