#include "UIFocusSystem.h"

#include "UI/Focus/UIFocusComponent.h"

#include "UI/Focus/FocusHelpers.h"
#include "UI/Focus/DirectionBasedNavigationAlgorithm.h"

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

    UIControl* focusedControl = FindFirstControl(control);
    ClearFocusState(root.Get());

    SetFocusedControl(focusedControl);
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

void UIFocusSystem::MoveFocus(FocusHelpers::Direction dir)
{
    if (root.Valid() && focusedControl.Valid())
    {
        DirectionBasedNavigationAlgorithm alg(root.Get());
        UIControl* next = alg.GetNextControl(focusedControl.Get(), dir);
        if (next && next != focusedControl)
        {
            SetFocusedControl(next);
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
