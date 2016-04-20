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
#include "UI/Input/UIActionBindingComponent.h"

#include "Input/InputSystem.h"

namespace DAVA
{
const FastName UIKeyInputSystem::ACTION_FOCUS_LEFT("FocusLeft");
const FastName UIKeyInputSystem::ACTION_FOCUS_RIGHT("FocusRight");
const FastName UIKeyInputSystem::ACTION_FOCUS_UP("FocusUp");
const FastName UIKeyInputSystem::ACTION_FOCUS_DOWN("FocusDown");

const FastName UIKeyInputSystem::ACTION_FOCUS_NEXT("FocusNext");
const FastName UIKeyInputSystem::ACTION_FOCUS_PREV("FocusPrev");

const FastName UIKeyInputSystem::ACTION_PERFORM("PerformAction");
const FastName UIKeyInputSystem::ACTION_ESCAPE("Escape");

UIKeyInputSystem::UIKeyInputSystem(UIFocusSystem* focusSystem_)
    : focusSystem(focusSystem_)
{
    BindGlobalShortcut(KeyboardShortcut(Key::LEFT), ACTION_FOCUS_LEFT);
    BindGlobalShortcut(KeyboardShortcut(Key::RIGHT), ACTION_FOCUS_RIGHT);
    BindGlobalShortcut(KeyboardShortcut(Key::UP), ACTION_FOCUS_UP);
    BindGlobalShortcut(KeyboardShortcut(Key::DOWN), ACTION_FOCUS_DOWN);

    BindGlobalShortcut(KeyboardShortcut(Key::TAB), ACTION_FOCUS_NEXT);
    BindGlobalShortcut(KeyboardShortcut(Key::TAB, KeyboardShortcut::MODIFIER_SHIFT), ACTION_FOCUS_PREV);

    BindGlobalShortcut(KeyboardShortcut(Key::ENTER), ACTION_PERFORM);
    BindGlobalShortcut(KeyboardShortcut(Key::ESCAPE), ACTION_ESCAPE);

    BindGlobalAction(ACTION_FOCUS_LEFT, MakeFunction(focusSystem, &UIFocusSystem::MoveFocusLeft));
    BindGlobalAction(ACTION_FOCUS_RIGHT, MakeFunction(focusSystem, &UIFocusSystem::MoveFocusRight));
    BindGlobalAction(ACTION_FOCUS_UP, MakeFunction(focusSystem, &UIFocusSystem::MoveFocusUp));
    BindGlobalAction(ACTION_FOCUS_DOWN, MakeFunction(focusSystem, &UIFocusSystem::MoveFocusDown));

    BindGlobalAction(ACTION_FOCUS_NEXT, MakeFunction(focusSystem, &UIFocusSystem::MoveFocusForward));
    BindGlobalAction(ACTION_FOCUS_PREV, MakeFunction(focusSystem, &UIFocusSystem::MoveFocusBackward));

    BindGlobalAction(ACTION_PERFORM, MakeFunction(focusSystem, &UIFocusSystem::PerformAction));
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

    if (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        modifiers |= KeyboardShortcut::ConvertKeyToModifier(event->key);
    }
    else if (phase == UIEvent::Phase::KEY_UP)
    {
        modifiers &= ~KeyboardShortcut::ConvertKeyToModifier(event->key);
    }
    KeyboardShortcut shortcut(event->key, modifiers);

    UIControl* focusedControl = focusSystem->GetFocusedControl();
    UIControl* rootControl = focusSystem->GetRoot();

    if (focusedControl == nullptr && rootControl == nullptr)
    {
        return;
    }

    if (!processed && phase == UIEvent::Phase::KEY_DOWN) // try to process shortcuts
    {
        UIControl* c = focusedControl != nullptr ? focusedControl : rootControl;
        while (!processed && c != nullptr)
        {
            UIActionBindingComponent* actionBindingComponent = c->GetComponent<UIActionBindingComponent>();
            if (actionBindingComponent)
            {
                FastName action = actionBindingComponent->GetInputMap().FindAction(shortcut);
                if (action.IsValid())
                {
                    processed = actionBindingComponent->GetActionMap().Perform(action);
                }
            }

            c = (c == rootControl) ? nullptr : c->GetParent();
        }
    }

    if (!processed) // try to handle key events directly by control
    {
        UIControl* c = focusedControl != nullptr ? focusedControl : rootControl;
        while (!processed && c != nullptr)
        {
            processed = c->SystemProcessInput(event);
            c = (c == rootControl) ? nullptr : c->GetParent();
        }
    }

    if (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        FastName action = globalInputMap.FindAction(shortcut);
        if (action.IsValid())
        {
            globalActions.Perform(action);
        }
    }
}

void UIKeyInputSystem::BindGlobalShortcut(const KeyboardShortcut& shortcut, const FastName& actionName)
{
    globalInputMap.BindAction(shortcut, actionName);
}

void UIKeyInputSystem::BindGlobalAction(const FastName& actionName, const UIActionMap::Action& action)
{
    globalActions.Put(actionName, action);
}

}
