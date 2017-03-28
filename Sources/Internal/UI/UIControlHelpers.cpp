#include "UIControlHelpers.h"
#include "UI/UIControl.h"
#include "UI/UIList.h"
#include "UI/UIScrollView.h"
#include "Utils/Utils.h"

namespace UIControlHelpersDetails
{
static const DAVA::String PATH_SEPARATOR("/");
static const DAVA::FastName WILDCARD_PARENT("..");
static const DAVA::FastName WILDCARD_CURRENT(".");
static const DAVA::FastName WILDCARD_ROOT("^");
static const DAVA::FastName WILDCARD_MATCHES_ONE_LEVEL("*");
static const DAVA::FastName WILDCARD_MATCHES_ZERO_OR_MORE_LEVEL("**");
static const DAVA::Set<DAVA::FastName> RESERVED_NAMES = {
    WILDCARD_PARENT,
    WILDCARD_CURRENT,
    WILDCARD_ROOT,
    WILDCARD_MATCHES_ONE_LEVEL,
    WILDCARD_MATCHES_ZERO_OR_MORE_LEVEL
};
}

namespace DAVA
{
String UIControlHelpers::GetControlPath(const UIControl* control, const UIControl* rootControl /*= NULL*/)
{
    using namespace UIControlHelpersDetails;
    if (!control)
        return "";

    String controlPath = "";
    UIControl* controlIter = control->GetParent();
    do
    {
        if (!controlIter)
            return "";

        controlPath = String(controlIter->GetName().c_str()) + PATH_SEPARATOR + controlPath;

        controlIter = controlIter->GetParent();
    } while (controlIter != rootControl);

    return controlPath;
}

String UIControlHelpers::GetPathToOtherControl(const UIControl* src, const UIControl* dst)
{
    using namespace UIControlHelpersDetails;
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
            if (!str.empty())
            {
                str += PATH_SEPARATOR;
            }
            str += WILDCARD_PARENT.c_str();
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
                str2 = String(p->GetName().c_str()) + PATH_SEPARATOR + str2;
            }
            p = p->GetParent();
        }

        if (str2.empty())
        {
            return str;
        }
        if (str.empty())
        {
            return str2;
        }
        return str + PATH_SEPARATOR + str2;
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
    return const_cast<UIControl*>(FindControlByPathImpl(controlPath, rootControl));
}

const UIControl* UIControlHelpers::FindControlByPath(const String& controlPath, const UIControl* rootControl)
{
    return FindControlByPathImpl(controlPath, rootControl);
}

const UIControl* UIControlHelpers::FindControlByPathImpl(const String& controlPath, const UIControl* rootControl)
{
    using namespace UIControlHelpersDetails;
    Vector<String> strPath;
    Split(controlPath, PATH_SEPARATOR, strPath, false, true);

    Vector<FastName> path;
    path.reserve(strPath.size());
    for (const String& str : strPath)
    {
        path.push_back(FastName(str));
    }

    return FindControlByPathImpl(path.begin(), path.end(), rootControl);
}

const UIControl* UIControlHelpers::FindControlByPathImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, const UIControl* rootControl)
{
    using namespace UIControlHelpersDetails;

    const UIControl* control = rootControl;

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
                const UIControl* res = FindControlByPathImpl(nextIt, end, c);
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

const UIControl* UIControlHelpers::FindControlByPathRecursivelyImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, const UIControl* rootControl)
{
    const UIControl* control = rootControl;

    const UIControl* res = FindControlByPathImpl(begin, end, rootControl);
    if (res)
    {
        return res;
    }

    for (UIControl* c : control->GetChildren())
    {
        res = FindControlByPathRecursivelyImpl(begin, end, c);
        if (res)
        {
            return res;
        }
    }

    return nullptr;
}

void UIControlHelpers::ScrollToControl(DAVA::UIControl* control, bool toTopLeftForBigControls)
{
    UIControl* parent = control->GetParent();
    if (parent)
    {
        ScrollToRect(parent, control->GetRect(), 0.0f, toTopLeftForBigControls);
    }
}

void UIControlHelpers::ScrollToControlWithAnimation(DAVA::UIControl* control, float32 animationTime, bool toTopLeftForBigControls)
{
    UIControl* parent = control->GetParent();
    if (parent)
    {
        ScrollToRect(parent, control->GetRect(), animationTime, toTopLeftForBigControls);
    }
}

bool UIControlHelpers::IsControlNameValid(const FastName& controlName)
{
    using namespace UIControlHelpersDetails;

    if (RESERVED_NAMES.count(controlName) > 0)
        return false;

    return (String(controlName.c_str()).find_first_of(PATH_SEPARATOR) == String::npos);
}

void UIControlHelpers::ScrollToRect(DAVA::UIControl* control, const Rect& rect, float32 animationTime, bool toTopLeftForBigControls)
{
    UIList* list = dynamic_cast<UIList*>(control);
    if (list)
    {
        ScrollListToRect(list, rect, animationTime, toTopLeftForBigControls);
    }
    else
    {
        UIScrollView* scrollView = dynamic_cast<UIScrollView*>(control);
        if (scrollView)
        {
            ScrollUIScrollViewToRect(scrollView, rect, animationTime, toTopLeftForBigControls);
        }
    }

    UIControl* parent = control->GetParent();
    if (parent)
    {
        Rect r = rect;
        r += control->GetPosition() - control->GetPivotPoint();
        ScrollToRect(parent, r, animationTime, toTopLeftForBigControls);
    }
}

float32 UIControlHelpers::GetScrollPositionToShowControl(float32 controlPos, float32 controlSize, float32 scrollSize, bool toTopLeftForBigControls)
{
    if (controlSize > scrollSize)
    {
        if (controlPos > 0 || toTopLeftForBigControls)
        {
            return -controlPos;
        }
        else if (controlPos + controlSize < scrollSize)
        {
            return scrollSize - (controlPos + controlSize);
        }
        else
        {
            return 0;
        }
    }
    else if (controlPos < 0)
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

Rect UIControlHelpers::ScrollListToRect(UIList* list, const DAVA::Rect& rect, float32 animationTime, bool toTopLeftForBigControls)
{
    Vector2 scrollSize = list->GetSize();

    float32 scrollPos = list->GetScrollPosition();
    Rect r = rect;
    if (list->GetOrientation() == UIList::ORIENTATION_HORIZONTAL)
    {
        float32 delta = GetScrollPositionToShowControl(rect.x, rect.dx, scrollSize.dx, toTopLeftForBigControls);
        scrollPos += delta;
        r.x += delta;
    }
    else
    {
        float32 delta = GetScrollPositionToShowControl(rect.y, rect.dy, scrollSize.dy, toTopLeftForBigControls);
        scrollPos += delta;
        r.y += delta;
    }

    if (animationTime > 0.0f)
    {
        list->ScrollToPosition(-scrollPos, animationTime);
    }
    else
    {
        list->SetScrollPosition(-scrollPos);
    }

    return r;
}

Rect UIControlHelpers::ScrollUIScrollViewToRect(UIScrollView* scrollView, const DAVA::Rect& rect, float32 animationTime, bool toTopLeftForBigControls)
{
    Vector2 scrollSize = scrollView->GetSize();
    Vector2 scrollPos = scrollView->GetScrollPosition();

    Vector2 delta;
    delta.x = GetScrollPositionToShowControl(rect.x, rect.dx, scrollSize.dx, toTopLeftForBigControls);
    delta.y = GetScrollPositionToShowControl(rect.y, rect.dy, scrollSize.dy, toTopLeftForBigControls);
    scrollPos += delta;

    if (animationTime > 0.0f)
    {
        scrollView->ScrollToPosition(scrollPos, animationTime);
    }
    else
    {
        scrollView->SetScrollPosition(scrollPos);
    }
    return rect + delta;
}
}
