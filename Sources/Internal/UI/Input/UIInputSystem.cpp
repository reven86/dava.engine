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

#include "UIInputSystem.h"

#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"

#include "UI/Focus/UIFocusSystem.h"
#include "UI/Input/UIModalInputComponent.h"
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Input/UIActionComponent.h"

#include "Input/InputSystem.h"

namespace DAVA
{
const FastName UIInputSystem::ACTION_FOCUS_LEFT("FocusLeft");
const FastName UIInputSystem::ACTION_FOCUS_RIGHT("FocusRight");
const FastName UIInputSystem::ACTION_FOCUS_UP("FocusUp");
const FastName UIInputSystem::ACTION_FOCUS_DOWN("FocusDown");

const FastName UIInputSystem::ACTION_FOCUS_NEXT("FocusNext");
const FastName UIInputSystem::ACTION_FOCUS_PREV("FocusPrev");

const FastName UIInputSystem::ACTION_PERFORM("PerformAction");
const FastName UIInputSystem::ACTION_ESCAPE("Escape");

UIInputSystem::UIInputSystem()
{
    focusSystem = new UIFocusSystem();

    BindGlobalAction(ACTION_FOCUS_LEFT, MakeFunction(this, &UIInputSystem::MoveFocusLeft));
    BindGlobalAction(ACTION_FOCUS_RIGHT, MakeFunction(this, &UIInputSystem::MoveFocusRight));
    BindGlobalAction(ACTION_FOCUS_UP, MakeFunction(this, &UIInputSystem::MoveFocusUp));
    BindGlobalAction(ACTION_FOCUS_DOWN, MakeFunction(this, &UIInputSystem::MoveFocusDown));

    BindGlobalAction(ACTION_FOCUS_NEXT, MakeFunction(this, &UIInputSystem::MoveFocusForward));
    BindGlobalAction(ACTION_FOCUS_PREV, MakeFunction(this, &UIInputSystem::MoveFocusBackward));

    BindGlobalAction(ACTION_PERFORM, MakeFunction(this, &UIInputSystem::PerformActionOnFocusedControl));
}

UIInputSystem::~UIInputSystem()
{
    SafeDelete(focusSystem);

    currentScreen = nullptr; // we are not owner
    popupContainer = nullptr; // we are not owner
}

void UIInputSystem::SetCurrentScreen(UIScreen* screen)
{
    currentScreen = screen;
    UpdateModalControl();
}

void UIInputSystem::SetPopupContainer(UIControl* container)
{
    popupContainer = container;
}

void UIInputSystem::OnControlVisible(UIControl* control)
{
    if (control->GetComponent<UIModalInputComponent>() != nullptr)
    {
        UpdateModalControl();
    }
    focusSystem->OnControlVisible(control);
}

void UIInputSystem::OnControlInvisible(UIControl* control)
{
    if (control->GetHover())
    {
        SetHoveredControl(nullptr);
    }

    if (control->GetInputEnabled())
    {
        CancelInputs(control, false);
    }

    if (control->GetComponent<UIModalInputComponent>() != nullptr)
    {
        UpdateModalControl();
    }

    focusSystem->OnControlInvisible(control);
}

void UIInputSystem::HandleEvent(UIEvent* event)
{
    if (currentScreen)
    {
        UIEvent::Phase phase = event->phase;

        if (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_UP || phase == UIEvent::Phase::KEY_DOWN_REPEAT || phase == UIEvent::Phase::CHAR || phase == UIEvent::Phase::CHAR_REPEAT)
        {
            HandleKeyEvent(event);
        }
        else if (phase == UIEvent::Phase::BEGAN || phase == UIEvent::Phase::DRAG || phase == UIEvent::Phase::ENDED || phase == UIEvent::Phase::CANCELLED)
        {
            HandleTouchEvent(event);
        }
        else
        {
            HandleOtherEvent(event); // joypad, geasture, mouse wheel
        }
    }

    auto startRemoveIt = std::remove_if(begin(touchEvents), end(touchEvents), [this](UIEvent& ev) {
        bool shouldRemove = (ev.phase == UIEvent::Phase::ENDED || ev.phase == UIEvent::Phase::CANCELLED);
        if (shouldRemove)
        {
            CancelInput(&ev);
        }
        return shouldRemove;
    });
    touchEvents.erase(startRemoveIt, end(touchEvents));
}

void UIInputSystem::CancelInput(UIEvent* touch)
{
    if (touch->touchLocker)
    {
        touch->touchLocker->SystemInputCancelled(touch);
    }
    if (touch->touchLocker != currentScreen)
    {
        currentScreen->SystemInputCancelled(touch);
    }
}

void UIInputSystem::CancelAllInputs()
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        CancelInput(&(*it));
    }
    touchEvents.clear();
}

void UIInputSystem::CancelInputs(UIControl* control, bool hierarchical)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if (!hierarchical)
        {
            if (it->touchLocker == control)
            {
                CancelInput(&(*it));
                break;
            }
            continue;
        }
        UIControl* parentLockerControl = it->touchLocker;
        while (parentLockerControl)
        {
            if (control == parentLockerControl)
            {
                CancelInput(&(*it));
                break;
            }
            parentLockerControl = parentLockerControl->GetParent();
        }
    }
}

void UIInputSystem::SwitchInputToControl(uint32 eventID, UIControl* targetControl)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if ((*it).touchId == eventID)
        {
            CancelInput(&(*it));

            if (targetControl->IsPointInside((*it).point))
            {
                (*it).controlState = UIEvent::CONTROL_STATE_INSIDE;
                targetControl->touchesInside++;
            }
            else
            {
                (*it).controlState = UIEvent::CONTROL_STATE_OUTSIDE;
            }
            (*it).touchLocker = targetControl;
            targetControl->currentInputID = eventID;
            if (targetControl->GetExclusiveInput())
            {
                SetExclusiveInputLocker(targetControl, eventID);
            }
            else
            {
                SetExclusiveInputLocker(NULL, -1);
            }

            targetControl->totalTouches++;
        }
    }
}

const Vector<UIEvent>& UIInputSystem::GetAllInputs() const
{
    return touchEvents;
}

void UIInputSystem::SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId)
{
    SafeRelease(exclusiveInputLocker);
    if (locker != NULL)
    {
        for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
        {
            if (it->touchId != lockEventId && it->touchLocker != locker)
            { //cancel all inputs excepts current input and inputs what allready handles by this locker.
                CancelInput(&(*it));
            }
        }
    }

    exclusiveInputLocker = SafeRetain(locker);
}

UIControl* UIInputSystem::GetExclusiveInputLocker() const
{
    return exclusiveInputLocker;
}

void UIInputSystem::SetHoveredControl(UIControl* newHovered)
{
    if (hovered != newHovered)
    {
        if (hovered)
        {
            hovered->SystemDidRemoveHovered();
            hovered->Release();
        }
        hovered = SafeRetain(newHovered);
        if (hovered)
        {
            hovered->SystemDidSetHovered();
        }
    }
}

UIControl* UIInputSystem::GetHoveredControl() const
{
    return hovered;
}

UIFocusSystem* UIInputSystem::GetFocusSystem() const
{
    return focusSystem;
}

void UIInputSystem::BindGlobalShortcut(const KeyboardShortcut& shortcut, const FastName& actionName)
{
    globalInputMap.BindAction(shortcut, actionName);
}

void UIInputSystem::BindGlobalAction(const FastName& actionName, const UIActionMap::Action& action)
{
    globalActions.Put(actionName, action);
}

void UIInputSystem::MoveFocusLeft()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::LEFT);
}

void UIInputSystem::MoveFocusRight()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::RIGHT);
}

void UIInputSystem::MoveFocusUp()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::UP);
}

void UIInputSystem::MoveFocusDown()
{
    focusSystem->MoveFocus(UINavigationComponent::Direction::DOWN);
}

void UIInputSystem::MoveFocusForward()
{
    focusSystem->MoveFocus(UITabOrderComponent::Direction::FORWARD, true);
}

void UIInputSystem::MoveFocusBackward()
{
    focusSystem->MoveFocus(UITabOrderComponent::Direction::BACKWARD, true);
}

void UIInputSystem::PerformActionOnControl(UIControl* control)
{
    if (control != nullptr)
    {
        UIActionComponent* actionComponent = control->GetComponent<UIActionComponent>();
        if (actionComponent != nullptr && actionComponent->GetAction().IsValid())
        {
            UIControl* c = control;
            bool processed = false;
            while (!processed && c != nullptr)
            {
                UIActionBindingComponent* actionBindingComponent = c->GetComponent<UIActionBindingComponent>();
                if (actionBindingComponent)
                {
                    processed = actionBindingComponent->GetActionMap().Perform(actionComponent->GetAction());
                }

                c = (c == modalControl.Get()) ? nullptr : c->GetParent();
            }
        }
    }
}

void UIInputSystem::PerformActionOnFocusedControl()
{
    PerformActionOnControl(focusSystem->GetFocusedControl());
}

void UIInputSystem::HandleTouchEvent(UIEvent* event)
{
    DVASSERT(event->phase == UIEvent::Phase::BEGAN || event->phase == UIEvent::Phase::DRAG || event->phase == UIEvent::Phase::ENDED || event->phase == UIEvent::Phase::CANCELLED);

    UIEvent* eventToHandle = nullptr;
    auto it = std::find_if(begin(touchEvents), end(touchEvents), [event](const UIEvent& ev) {
        return ev.touchId == event->touchId;
    });
    if (it == end(touchEvents))
    {
        touchEvents.push_back(*event);
        eventToHandle = &touchEvents.back();
    }
    else
    {
        it->timestamp = event->timestamp;
        it->physPoint = event->physPoint;
        it->point = event->point;
        it->tapCount = event->tapCount;
        it->phase = event->phase;
        it->inputHandledType = event->inputHandledType;

        eventToHandle = &(*it);
    }

    UIEvent::Phase phase = eventToHandle->phase;
    if (phase == UIEvent::Phase::BEGAN)
    {
        focusedControlWhenTouchBegan = focusSystem->GetFocusedControl();
        positionOfTouchWhenTouchBegan = eventToHandle->point;
    }

    if (modalControl.Valid())
    {
        RefPtr<UIControl> control = modalControl;
        control->SystemInput(eventToHandle);
    }
    else if (!popupContainer->SystemInput(eventToHandle))
    {
        currentScreen->SystemInput(eventToHandle);
    }

    if (phase == UIEvent::Phase::ENDED)
    {
        UIControl* focusedControl = focusSystem->GetFocusedControl();
        if (focusedControl != nullptr)
        {
            static const float32 draggingThresholdSq = 20.0f * 20.0f;
            bool focusWasntChanged = focusedControl == focusedControlWhenTouchBegan;

            bool touchWasntDragged = (positionOfTouchWhenTouchBegan - eventToHandle->point).SquareLength() < draggingThresholdSq;
            bool touchOutsideControl = !focusedControl->IsPointInside(eventToHandle->point);
            if (focusWasntChanged && touchWasntDragged && touchOutsideControl)
            {
                focusedControl->OnTouchOutsideFocus();
            }
        }
        focusedControlWhenTouchBegan = nullptr;
    }
}

void UIInputSystem::HandleKeyEvent(UIEvent* event)
{
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
    UIControl* rootControl = modalControl.Valid() ? modalControl.Get() : currentScreen;

    if (focusedControl != nullptr || rootControl != nullptr)
    {
        UIControl* c = focusedControl != nullptr ? focusedControl : rootControl;
        while (c != nullptr)
        {
            if (c->SystemProcessInput(event))
            {
                break;
            }

            if (phase == UIEvent::Phase::KEY_DOWN)
            {
                UIActionBindingComponent* actionBindingComponent = c->GetComponent<UIActionBindingComponent>();
                if (actionBindingComponent)
                {
                    FastName action = actionBindingComponent->GetInputMap().FindAction(shortcut);
                    if (action.IsValid() && actionBindingComponent->GetActionMap().Perform(action))
                    {
                        break;
                    }
                }
            }

            if (c == rootControl)
            {
                break;
            }
            c = c->GetParent();
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

void UIInputSystem::HandleOtherEvent(UIEvent* event)
{
    if (modalControl.Valid())
    {
        RefPtr<UIControl> control = modalControl;
        control->SystemInput(event);
    }
    else if (!popupContainer->SystemInput(event))
    {
        currentScreen->SystemInput(event);
    }
}

void UIInputSystem::UpdateModalControl()
{
    if (currentScreen != nullptr)
    {
        UIControl* root = FindNearestToUserModalControl();
        focusSystem->SetRoot(root == nullptr ? currentScreen : root);
        modalControl = root;

        if (root != nullptr)
        {
            CancelInputForAllOutsideChildren(root);
        }
    }
    else
    {
        focusSystem->SetRoot(nullptr);
        modalControl = nullptr;
    }
}

void UIInputSystem::CancelInputForAllOutsideChildren(UIControl* root)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        UIControl* control = it->touchLocker;
        if (control != nullptr)
        {
            bool isInHierarchy = false;
            while (control != nullptr)
            {
                if (control == modalControl)
                {
                    isInHierarchy = true;
                    break;
                }
                control = control->GetParent();
            }

            if (!isInHierarchy)
            {
                CancelInput(&(*it));
            }
        }
    }
}

UIControl* UIInputSystem::FindNearestToUserModalControl() const
{
    UIControl* control = FindNearestToUserModalControlImpl(popupContainer);
    if (control != nullptr)
    {
        return control;
    }
    return FindNearestToUserModalControlImpl(currentScreen);
}

UIControl* UIInputSystem::FindNearestToUserModalControlImpl(UIControl* current) const
{
    const List<UIControl*>& children = current->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
    {
        UIControl* result = FindNearestToUserModalControlImpl(*it);
        if (result != nullptr)
        {
            return result;
        }
    }

    if (current->GetComponent<UIModalInputComponent>() != nullptr && current->IsVisible())
    {
        return current;
    }
    return nullptr;
}
}
