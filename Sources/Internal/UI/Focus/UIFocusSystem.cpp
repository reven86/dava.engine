#include "UIFocusSystem.h"

#include "UI/Focus/UIFocusComponent.h"

#include "UI/Focus/FocusHelpers.h"
#include "UI/Focus/DirectionBasedNavigationAlgorithm.h"
#include "UI/Focus/TabTraversalAlgorithm.h"

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

void UIFocusSystem::ControlBecomInvisible(UIControl* control)
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

bool UIFocusSystem::MoveFocusLeft()
{
    return MoveFocus(FocusHelpers::Direction::LEFT);
}

bool UIFocusSystem::MoveFocusRight()
{
    return MoveFocus(FocusHelpers::Direction::RIGHT);
}

bool UIFocusSystem::MoveFocusUp()
{
    return MoveFocus(FocusHelpers::Direction::UP);
}

bool UIFocusSystem::MoveFocusDown()
{
    return MoveFocus(FocusHelpers::Direction::DOWN);
}

bool UIFocusSystem::MoveFocusForward()
{
    return MoveFocus(FocusHelpers::TabDirection::FORWARD);
}

bool UIFocusSystem::MoveFocusBackward()
{
    return MoveFocus(FocusHelpers::TabDirection::BACKWARD);
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
