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


#include "UIControlHelpers.h"
#include "UI/UIControl.h"
#include "UI/UIList.h"
#include "UI/UIScrollView.h"
#include "Utils/Utils.h"

namespace DAVA
{
const FastName UIControlHelpers::WILDCARD_PARENT("..");
const FastName UIControlHelpers::WILDCARD_CURRENT(".");
const FastName UIControlHelpers::WILDCARD_ROOT("^");
const FastName UIControlHelpers::WILDCARD_MATCHES_ONE_LEVEL("*");
const FastName UIControlHelpers::WILDCARD_MATCHES_ZERO_OR_MORE_LEVEL("**");

String UIControlHelpers::GetControlPath(const UIControl* control, const UIControl* rootControl /*= NULL*/)
{
    if (!control)
        return "";

    String controlPath = "";
    UIControl* controlIter = control->GetParent();
    do
    {
        if (!controlIter)
            return "";

        controlPath = String(controlIter->GetName().c_str()) + "/" + controlPath;

        controlIter = controlIter->GetParent();
    } while (controlIter != rootControl);

    return controlPath;
}

String UIControlHelpers::GetPathToOtherControl(const UIControl* src, const UIControl* dst)
{
    const UIControl* commonParent = src;

    const UIControl* p2 = dst;
    while (commonParent && commonParent != p2)
    {
        if (p2)
        {
            p2 = p2->GetParent();
        }
        else
        {
            p2 = dst;
            commonParent = commonParent->GetParent();
        }
    }

    if (commonParent)
    {
        const UIControl* p = src;
        String str;
        while (p != commonParent)
        {
            str += "../";
            p = p->GetParent();
        }

        p = dst;
        String str2;
        while (p != commonParent)
        {
            if (str2.empty())
            {
                str2 = p->GetName().c_str();
            }
            else
            {
                str2 = String(p->GetName().c_str()) + "/" + str2;
            }
            p = p->GetParent();
        }
        return str + str2;
    }
    else
    {
        return "";
    }
}

UIControl* UIControlHelpers::FindChildControlByName(const String& controlName, const UIControl* rootControl, bool recursive)
{
    return FindChildControlByName(FastName(controlName), rootControl, recursive);
}

UIControl* UIControlHelpers::FindChildControlByName(const FastName& controlName, const UIControl* rootControl, bool recursive)
{
    for (UIControl* c : rootControl->GetChildren())
    {
        if (c->GetName() == controlName)
            return c;

        if (recursive)
        {
            UIControl* res = FindChildControlByName(controlName, c, recursive);
            if (res)
            {
                return res;
            }
        }
    }
    return nullptr;
}

UIControl* UIControlHelpers::FindControlByPath(const String& controlPath, UIControl* rootControl)
{
    return FindControlByPathImpl(controlPath, rootControl);
}

const UIControl* UIControlHelpers::FindControlByPath(const String& controlPath, const UIControl* rootControl)
{
    return FindControlByPathImpl(controlPath, rootControl);
}

template <typename ControlType>
ControlType* UIControlHelpers::FindControlByPathImpl(const String& controlPath, ControlType* rootControl)
{
    Vector<String> strPath;
    Split(controlPath, "/", strPath, false, true);

    Vector<FastName> path;
    path.reserve(strPath.size());
    for (const String& str : strPath)
    {
        path.push_back(FastName(str));
    }

    return FindControlByPathImpl(path.begin(), path.end(), rootControl);
}

template <typename ControlType>
ControlType* UIControlHelpers::FindControlByPathImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, ControlType* rootControl)
{
    ControlType* control = rootControl;

    for (auto it = begin; it != end; ++it)
    {
        const FastName& name = *it;

        if (name == WILDCARD_PARENT)
        {
            control = control->GetParent(); // one step up
        }
        else if (name == WILDCARD_CURRENT)
        {
            // do nothing control will not changed
        }
        else if (name == WILDCARD_ROOT)
        {
            control = control->GetParentWithContext();
        }
        else if (name == WILDCARD_MATCHES_ONE_LEVEL)
        {
            auto nextIt = it + 1;
            for (UIControl* c : control->GetChildren())
            {
                UIControl* res = FindControlByPathImpl(nextIt, end, c);
                if (res)
                {
                    return res;
                }
            }
            return nullptr;
        }
        else if (name == WILDCARD_MATCHES_ZERO_OR_MORE_LEVEL)
        {
            auto nextIt = it + 1;
            return FindControlByPathRecursivelyImpl(nextIt, end, control);
        }
        else
        {
            control = UIControlHelpers::FindChildControlByName(name, control, false);
        }

        if (!control)
        {
            return nullptr;
        }
    }
    return control;
}

template <typename ControlType>
ControlType* UIControlHelpers::FindControlByPathRecursivelyImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, ControlType* rootControl)
{
    ControlType* control = rootControl;

    ControlType* res = FindControlByPathImpl(begin, end, rootControl);
    if (res)
    {
        return res;
    }

    for (ControlType* c : control->GetChildren())
    {
        res = FindControlByPathRecursivelyImpl(begin, end, c);
        if (res)
        {
            return res;
        }
    }

    return nullptr;
}

void UIControlHelpers::ScrollToControl(DAVA::UIControl* control, bool withAnimation)
{
    UIControl* parent = control->GetParent();
    if (parent)
    {
        ScrollToRect(parent, control->GetRect(), withAnimation);
    }
}

void UIControlHelpers::ScrollToRect(DAVA::UIControl* control, const Rect& rect, bool withAnimation)
{
    UIList* list = dynamic_cast<UIList*>(control);
    if (list)
    {
        ScrollListToRect(list, rect, withAnimation);
    }
    else
    {
        UIScrollView* scrollView = dynamic_cast<UIScrollView*>(control);
        if (scrollView)
        {
            ScrollUIScrollViewToRect(scrollView, rect, withAnimation);
        }
    }

    UIControl* parent = control->GetParent();
    if (parent)
    {
        Rect r = rect;
        r += control->GetPosition();
        ScrollToRect(parent, r, withAnimation);
    }
}

float32 UIControlHelpers::GetScrollPositionToShowControl(float32 controlPos, float32 controlSize, float32 scrollSize)
{
    if (controlPos < 0)
    {
        return -controlPos;
    }
    else if (controlPos + controlSize > scrollSize)
    {
        return scrollSize - (controlPos + controlSize);
    }
    else
    {
        return 0;
    }
}

void UIControlHelpers::ScrollListToRect(UIList* list, const DAVA::Rect& rect, bool withAnimation)
{
    Vector2 scrollSize = list->GetSize();

    float32 scrollPos = list->GetScrollPosition();
    if (list->GetOrientation() == UIList::ORIENTATION_HORIZONTAL)
    {
        scrollPos += GetScrollPositionToShowControl(rect.x, rect.dx, scrollSize.dx);
    }
    else
    {
        scrollPos += GetScrollPositionToShowControl(rect.y, rect.dy, scrollSize.dy);
    }

    if (withAnimation)
    {
        list->ScrollToPosition(-scrollPos);
    }
    else
    {
        list->SetScrollPosition(-scrollPos);
    }
}

void UIControlHelpers::ScrollUIScrollViewToRect(UIScrollView* scrollView, const DAVA::Rect& rect, bool withAnimation)
{
    Vector2 scrollSize = scrollView->GetSize();
    Vector2 scrollPos = scrollView->GetScrollPosition();

    scrollPos.x += GetScrollPositionToShowControl(rect.x, rect.dx, scrollSize.dx);
    scrollPos.y += GetScrollPositionToShowControl(rect.y, rect.dy, scrollSize.dy);

    if (withAnimation)
    {
        scrollView->ScrollToPosition(scrollPos);
    }
    else
    {
        scrollView->SetScrollPosition(scrollPos);
    }
}
}
