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
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Focus/UIKeyInputSystem.h"

#include "Input/InputSystem.h"

namespace DAVA
{
UIInputSystem::UIInputSystem(UIFocusSystem* focusSystem_)
    : focusSystem(focusSystem_)
{
    keyInputSystem = new UIKeyInputSystem(focusSystem);
}

UIInputSystem::~UIInputSystem()
{
    SafeDelete(keyInputSystem);

    focusSystem = nullptr; // we are not owner
    currentScreen = nullptr; // we are not owner
    popupContainer = nullptr; // we are not owner
}

void UIInputSystem::SetCurrentScreen(UIScreen* screen)
{
    currentScreen = screen;
}

void UIInputSystem::SetPopupContainer(UIControl* container)
{
    popupContainer = container;
}

void UIInputSystem::OnControlVisible(UIControl* control)
{
    focusSystem->OnControlVisible(control);
}

void UIInputSystem::OnControlInvisible(UIControl* control)
{
    if (control->GetInputEnabled())
    {
        CancelInputs(control, false);
    }

    focusSystem->OnControlInvisible(control);
}

void UIInputSystem::HandleEvent(UIEvent* newEvent)
{
    UIEvent* eventToHandle = nullptr;

    if (newEvent->phase == UIEvent::Phase::BEGAN || newEvent->phase == UIEvent::Phase::DRAG || newEvent->phase == UIEvent::Phase::ENDED || newEvent->phase == UIEvent::Phase::CANCELLED)
    {
        auto it = std::find_if(begin(touchEvents), end(touchEvents), [newEvent](const UIEvent& ev) {
            return ev.touchId == newEvent->touchId;
        });
        if (it == end(touchEvents))
        {
            touchEvents.push_back(*newEvent);
            eventToHandle = &touchEvents.back();
        }
        else
        {
            it->timestamp = newEvent->timestamp;
            it->physPoint = newEvent->physPoint;
            it->point = newEvent->point;
            it->tapCount = newEvent->tapCount;
            it->phase = newEvent->phase;
            it->inputHandledType = newEvent->inputHandledType;

            eventToHandle = &(*it);
        }
    }
    else
    {
        eventToHandle = newEvent;
    }

    if (currentScreen)
    {
        UIEvent::Phase phase = eventToHandle->phase;

        if (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_UP || phase == UIEvent::Phase::KEY_DOWN_REPEAT || phase == UIEvent::Phase::CHAR || phase == UIEvent::Phase::CHAR_REPEAT)
        {
            keyInputSystem->HandleKeyEvent(eventToHandle);
        }
        else
        {
            if (phase == UIEvent::Phase::BEGAN)
            {
                focusedControlWhenTouchBegan = focusSystem->GetFocusedControl();
                positionOfTouchWhenTouchBegan = eventToHandle->point;
            }

            if (!popupContainer->SystemInput(eventToHandle))
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
}
