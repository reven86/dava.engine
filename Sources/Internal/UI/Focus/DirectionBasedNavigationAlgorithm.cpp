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

#include "DirectionBasedNavigationAlgorithm.h"

#include "UIFocusComponent.h"
#include "UIFocusGroupComponent.h"
#include "UINavigationComponent.h"
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
    if (focusedControl != nullptr)
    {
        UIControl* next = FindNextControl(focusedControl, dir);
        if (next != nullptr && next != focusedControl)
        {
            return next;
        }
    }
    return nullptr;
}

UIControl* DirectionBasedNavigationAlgorithm::FindNextControl(UIControl* focusedControl, FocusHelpers::Direction dir) const
{
    UIControl* next = FindNextSpecifiedControl(focusedControl, dir);
    if (next != nullptr)
    {
        return next;
    }

    UIControl* parent = focusedControl;
    while (parent != nullptr && parent != root.Get())
    {
        UIFocusGroupComponent* focusGroup = parent->GetComponent<UIFocusGroupComponent>();
        if (focusGroup != nullptr)
        {
            UIControl* c = FindNearestControl(focusedControl, parent, dir);
            if (c != nullptr)
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
    UINavigationComponent* navigation = focusedControl->GetComponent<UINavigationComponent>();
    if (navigation != nullptr)
    {
        const String& controlInDirection = navigation->GetNextControlPathInDirection(dir);
        if (!controlInDirection.empty())
        {
            UIControl* next = UIControlHelpers::FindControlByPath(controlInDirection, focusedControl);
            if (next != nullptr && FocusHelpers::CanFocusControl(next) && next != focusedControl)
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

        if (res != nullptr)
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

    switch (dir)
    {
    case FocusHelpers::Direction::UP:
        return dy < 0 && Abs(dy) > Abs(dx);

    case FocusHelpers::Direction::DOWN:
        return dy > 0 && Abs(dy) > Abs(dx);

    case FocusHelpers::Direction::LEFT:
        return dx < 0 && Abs(dx) > Abs(dy);

    case FocusHelpers::Direction::RIGHT:
        return dx > 0 && Abs(dx) > Abs(dy);

    default:
        DVASSERT(false);
        break;
    }
    return false;
}
}
