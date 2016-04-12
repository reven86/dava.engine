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

#include "UIFocusSystem.h"

#include "UI/Focus/UIFocusComponent.h"

#include "UI/Focus/FocusHelpers.h"
#include "UI/Focus/DirectionBasedNavigationAlgorithm.h"
#include "UI/Focus/TabTraversalAlgorithm.h"
#include "UI/Input/UIActionBindingComponent.h" // TODO: Fix dependencies
#include "UI/Input/UIActionComponent.h" // TODO: Fix dependencies

#include "UI/UIControl.h"
#include "UI/UIList.h"
#include "UI/UIEvent.h"
#include "UI/UIControlHelpers.h"
#include "UI/UITextField.h"

#include "Input/KeyboardDevice.h"

namespace DAVA
{
UIFocusSystem::UIFocusSystem()
{
}

UIFocusSystem::~UIFocusSystem()
{
}

UIControl* UIFocusSystem::GetRoot() const
{
    return root.Get();
}

void UIFocusSystem::SetRoot(UIControl* newRoot)
{
    root = newRoot;

    if (root)
    {
        UIControl* focusedControl = FindFirstControl(newRoot);
        ClearFocusState(newRoot);
        SetFocusedControl(nullptr);

        SetFocusedControl(focusedControl);
    }
    else
    {
        SetFocusedControl(nullptr);
    }
}

UIControl* UIFocusSystem::GetFocusedControl() const
{
    return focusedControl.Get();
}

void UIFocusSystem::SetFocusedControl(UIControl* control)
{
    bool textFieldWasEditing = false;
    if (control != focusedControl.Get())
    {
        if (focusedControl.Valid())
        {
            UITextField* textField = dynamic_cast<UITextField*>(focusedControl.Get());
            if (textField)
            {
                textFieldWasEditing = textField->IsEditing();
            }

            focusedControl->SystemOnFocusLost();
            focusedControl = nullptr;
        }

        if (control != nullptr)
        {
            if (FocusHelpers::CanFocusControl(control))
            {
                focusedControl = control;
                focusedControl->SystemOnFocused();
                UIControlHelpers::ScrollToControl(focusedControl.Get());

                if (textFieldWasEditing)
                {
                    UITextField* textField = dynamic_cast<UITextField*>(focusedControl.Get());
                    if (textField)
                    {
                        textField->StartEdit();
                    }
                }
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
}

void UIFocusSystem::OnControlVisible(UIControl* control)
{
    if (!focusedControl.Valid() && FocusHelpers::CanFocusControl(control) && root.Valid())
    {
        UIControl* c = control;
        while (c != nullptr && c != root.Get())
        {
            c = c->GetParent();
        }

        if (c != nullptr) // control in current hierarchy
        {
            SetFocusedControl(control);
        }
    }
}

void UIFocusSystem::OnControlInvisible(UIControl* control)
{
    if (focusedControl == control)
    {
        if (root.Valid())
        {
            UIControl* focusedControl = FindFirstControl(root.Get());
            ClearFocusState(root.Get());
            SetFocusedControl(focusedControl);
        }
        else
        {
            SetFocusedControl(nullptr);
        }
    }
}

void UIFocusSystem::MoveFocusLeft()
{
    MoveFocus(FocusHelpers::Direction::LEFT);
}

void UIFocusSystem::MoveFocusRight()
{
    MoveFocus(FocusHelpers::Direction::RIGHT);
}

void UIFocusSystem::MoveFocusUp()
{
    MoveFocus(FocusHelpers::Direction::UP);
}

void UIFocusSystem::MoveFocusDown()
{
    MoveFocus(FocusHelpers::Direction::DOWN);
}

void UIFocusSystem::MoveFocusForward()
{
    MoveFocus(FocusHelpers::TabDirection::FORWARD);
}

void UIFocusSystem::MoveFocusBackward()
{
    MoveFocus(FocusHelpers::TabDirection::BACKWARD);
}

bool UIFocusSystem::MoveFocus(FocusHelpers::Direction dir)
{
    if (root.Valid() && focusedControl.Valid())
    {
        DirectionBasedNavigationAlgorithm alg(root.Get());
        UIControl* next = alg.GetNextControl(focusedControl.Get(), dir);
        if (next != nullptr && next != focusedControl)
        {
            SetFocusedControl(next);
            return true;
        }
    }
    return false;
}

bool UIFocusSystem::MoveFocus(FocusHelpers::TabDirection dir)
{
    if (root.Valid() && focusedControl.Valid())
    {
        TabTraversalAlgorithm alg(root.Get());
        UIControl* next = alg.GetNextControl(focusedControl.Get(), dir);
        if (next != nullptr && next != focusedControl)
        {
            SetFocusedControl(next);
            return true;
        }
    }
    return false;
}

void UIFocusSystem::PerformAction()
{
    if (focusedControl != nullptr)
    {
        UIActionComponent* actionComponent = focusedControl->GetComponent<UIActionComponent>();
        if (actionComponent != nullptr && actionComponent->GetAction().IsValid())
        {
            UIControl* c = focusedControl.Get();
            bool processed = false;
            while (!processed && c != nullptr)
            {
                UIActionBindingComponent* actionBindingComponent = c->GetComponent<UIActionBindingComponent>();
                if (actionBindingComponent)
                {
                    processed = actionBindingComponent->GetActionMap().Perform(actionComponent->GetAction());
                }

                c = (c == root.Get()) ? nullptr : c->GetParent();
            }
        }
    }
}

void UIFocusSystem::ClearFocusState(UIControl* control)
{
    control->SetState(control->GetState() & (~UIControl::STATE_FOCUSED));
    for (UIControl* c : control->GetChildren())
    {
        ClearFocusState(c);
    }
}

UIControl* UIFocusSystem::FindFirstControl(UIControl* control) const
{
    UIControl* candidate = nullptr;
    for (UIControl* c : control->GetChildren())
    {
        UIControl* res = FindFirstControl(c);
        if (res != nullptr && IsControlBetterForFocusThanCandidate(res, candidate))
        {
            candidate = res;
        }
    }

    if (candidate == nullptr && FocusHelpers::CanFocusControl(control))
    {
        return control;
    }

    return candidate;
}

bool UIFocusSystem::IsControlBetterForFocusThanCandidate(UIControl* c1, UIControl* c2) const
{
    DVASSERT(c1 != nullptr);
    if (c2 == nullptr)
    {
        return true;
    }

    UIFocusComponent* f1 = c1->GetComponent<UIFocusComponent>();
    DVASSERT(f1 != nullptr);
    UIFocusComponent* f2 = c2->GetComponent<UIFocusComponent>();
    DVASSERT(f2 != nullptr);

    if ((c1->GetState() & UIControl::STATE_FOCUSED) != 0 && (c2->GetState() & UIControl::STATE_FOCUSED) == 0)
    {
        return true;
    }

    if (f1->IsRequestFocus() && !f2->IsRequestFocus())
    {
        return true;
    }

    return false;
}
}
