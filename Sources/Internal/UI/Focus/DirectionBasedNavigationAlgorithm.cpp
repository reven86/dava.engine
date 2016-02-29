#include "DirectionBasedNavigationAlgorithm.h"

#include "UIFocusComponent.h"
#include "FocusHelpers.h"

#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIList.h"
#include "UI/UIEvent.h"
#include "UI/UIControlHelpers.h"
#include "Input/KeyboardDevice.h"

namespace DAVA
{
DirectionBasedNavigationAlgorithm::DirectionBasedNavigationAlgorithm(UIControl* root_)
{
    root = root_;
}

DirectionBasedNavigationAlgorithm::~DirectionBasedNavigationAlgorithm()
{
}

UIControl* DirectionBasedNavigationAlgorithm::GetNextControl(UIControl* focusedControl, FocusHelpers::Direction dir)
{
    if (focusedControl)
    {
        UIControl* next = FindNextControl(focusedControl, dir);
        if (next && next != focusedControl)
        {
            return next;
        }
    }
    return nullptr;
}

UIControl* DirectionBasedNavigationAlgorithm::FindNextControl(UIControl* focusedControl, FocusHelpers::Direction dir) const
{
    UIControl* next = FindNextSpecifiedControl(focusedControl, dir);
    if (next)
    {
        return next;
    }

    UIControl* parent = focusedControl;
    while (parent && parent != root.Get())
    {
        UIFocusComponent* focus = parent->GetComponent<UIFocusComponent>();
        if (focus && (focus->GetPolicy() == UIFocusComponent::FOCUSABLE_GROUP))
        {
            UIControl* c = FindNearestControl(focusedControl, parent, dir);
            if (c)
            {
                return c;
            }
        }

        parent = parent->GetParent();
    }

    if (root.Valid())
    {
        return FindNearestControl(focusedControl, root.Get(), dir);
    }

    return nullptr;
}

UIControl* DirectionBasedNavigationAlgorithm::FindNextSpecifiedControl(UIControl* focusedControl, FocusHelpers::Direction dir) const
{
    UIFocusComponent* focus = focusedControl->GetComponent<UIFocusComponent>();
    if (focus != nullptr)
    {
        const String& controlInDirection = focus->GetControlInDirection(dir);
        if (!controlInDirection.empty())
        {
            UIControl* next = UIControlHelpers::FindControlByPath(controlInDirection, focusedControl);
            if (next && FocusHelpers::CanFocusControl(next) && next != focusedControl)
            {
                return next;
            }
        }
    }
    return nullptr;
}

UIControl* DirectionBasedNavigationAlgorithm::FindNearestControl(UIControl* focusedControl, UIControl* control, FocusHelpers::Direction dir) const
{
    UIControl* bestControl = nullptr;
    float32 bestDistSq = 0;

    Rect rect = focusedControl->GetAbsoluteRect();
    Vector2 pos = rect.GetCenter();

    switch (dir)
    {
    case FocusHelpers::Direction::UP:
        pos.y = rect.y;
        break;

    case FocusHelpers::Direction::DOWN:
        pos.y = rect.y + rect.dy;
        break;

    case FocusHelpers::Direction::LEFT:
        pos.x = rect.x;
        break;

    case FocusHelpers::Direction::RIGHT:
        pos.x = rect.x + rect.dx;
        break;

    default:
        DVASSERT(false);
        break;
    }

    if (CanNavigateToControl(focusedControl, control, dir))
    {
        return control;
    }

    for (UIControl* c : control->GetChildren())
    {
        UIControl* res = FindNearestControl(focusedControl, c, dir);

        if (res)
        {
            Vector2 p = CalcNearestPos(pos, res, dir);
            float32 distSq = (p - pos).SquareLength();
            if (bestControl == nullptr || distSq < bestDistSq)
            {
                bestControl = res;
                bestDistSq = distSq;
            }
        }
    }

    return bestControl;
}

Vector2 DirectionBasedNavigationAlgorithm::CalcNearestPos(const Vector2& pos, UIControl* testControl, FocusHelpers::Direction dir) const
{
    Rect r = testControl->GetAbsoluteRect();
    Vector2 res = r.GetCenter();
    if (dir == FocusHelpers::Direction::UP || dir == FocusHelpers::Direction::DOWN)
    {
        if (pos.x > r.x + r.dx)
        {
            res.x = r.x + r.dx;
        }
        else if (pos.x < r.x)
        {
            res.x = r.x;
        }
        else
        {
            res.x = pos.x;
        }
    }
    else
    {
        if (pos.y > r.y + r.dy)
        {
            res.y = r.y + r.dy;
        }
        else if (pos.y < r.y)
        {
            res.y = r.y;
        }
        else
        {
            res.y = pos.y;
        }
    }

    if (dir == FocusHelpers::Direction::UP)
    {
        res.y = Min(r.y + r.dy, pos.y);
    }
    else if (dir == FocusHelpers::Direction::DOWN)
    {
        res.y = Max(r.y, pos.y);
    }
    else if (dir == FocusHelpers::Direction::LEFT)
    {
        res.x = Min(r.x + r.dx, pos.x);
    }
    else if (dir == FocusHelpers::Direction::RIGHT)
    {
        res.x = Max(r.x, pos.x);
    }

    return res;
}

bool DirectionBasedNavigationAlgorithm::CanNavigateToControl(UIControl* focusedControl, UIControl* control, FocusHelpers::Direction dir) const
{
    if (control == focusedControl || !FocusHelpers::CanFocusControl(control))
    {
        return false;
    }
    Rect rect = focusedControl->GetAbsoluteRect();
    Vector2 pos = rect.GetCenter();
    Vector2 srcPos = rect.GetCenter();

    Vector2 cPos = CalcNearestPos(pos, control, dir);

    float32 dx = cPos.x - srcPos.x;
    float32 dy = cPos.y - srcPos.y;

    bool ok = false;
    switch (dir)
    {
    case FocusHelpers::Direction::UP:
        ok = dy < 0 && Abs(dy) > Abs(dx);
        break;

    case FocusHelpers::Direction::DOWN:
        ok = dy > 0 && Abs(dy) > Abs(dx);
        break;

    case FocusHelpers::Direction::LEFT:
        ok = dx < 0 && Abs(dx) > Abs(dy);
        break;

    case FocusHelpers::Direction::RIGHT:
        ok = dx > 0 && Abs(dx) > Abs(dy);
        break;

    default:
        DVASSERT(false);
        break;
    }

    return ok ? control : nullptr;
}
}
