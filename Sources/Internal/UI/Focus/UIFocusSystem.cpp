#include "UIFocusSystem.h"

#include "UI/Focus/UIFocusComponent.h"

#include "UI/Focus/FocusHelpers.h"
#include "UI/Focus/DirectionBasedNavigationAlgorithm.h"
#include "UI/Focus/TabTraversalAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/UIList.h"
#include "UI/UIEvent.h"
#include "UI/UIControlHelpers.h"

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

void UIFocusSystem::SetRoot(UIControl* control)
{
    root = control;

    if (root)
    {
        UIControl* focusedControl = FindFirstControl(control);
        ClearFocusState(root.Get());

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
    if (focusedControl)
    {
        focusedControl->SystemOnFocusLost();
        focusedControl = nullptr;
    }

    if (!focusedControl.Valid())
    {
        focusedControl = control;
        if (focusedControl)
        {
            focusedControl->SystemOnFocused();
            UIControlHelpers::ScrollToControl(focusedControl.Get());
        }
    }
}

void UIFocusSystem::ControlBecomInvisible(UIControl* control)
{
    if (focusedControl == control)
    {
        SetFocusedControl(nullptr);
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
        if (next && next != focusedControl)
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
        if (next && next != focusedControl)
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
    return FindFirstControlImpl(control, nullptr);
}

UIControl* UIFocusSystem::FindFirstControlImpl(UIControl* control, UIControl* candidate) const
{
    if (FocusHelpers::CanFocusControl(control))
    {
        UIFocusComponent* focus = control->GetComponent<UIFocusComponent>();
        if (candidate == nullptr)
        {
            return control;
        }
        else if ((control->GetState() & UIControl::STATE_FOCUSED) != 0)
        {
            return control;
        }
        else if (focus->IsRequestFocus() && (candidate == nullptr || ((candidate->GetState() & UIControl::STATE_FOCUSED) == 0)))
        {
            return control;
        }
    }

    UIControl* test = candidate;
    for (UIControl* c : control->GetChildren())
    {
        UIControl* res = FindFirstControlImpl(c, candidate);
        if (res)
        {
            if (test == nullptr || ((res->GetState() & UIControl::STATE_FOCUSED) == 0))
            {
                test = res;
                if ((test->GetState() & UIControl::STATE_FOCUSED) != 0)
                    return test;
            }
        }
    }

    return test;
}
}
