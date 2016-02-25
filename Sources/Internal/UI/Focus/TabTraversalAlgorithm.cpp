#include "TabTraversalAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/Focus/UIFocusComponent.h"

namespace DAVA
{
TabTraversalAlgorithm::TabTraversalAlgorithm(UIControl* root_)
{
    root = root_;
}

TabTraversalAlgorithm::~TabTraversalAlgorithm()
{
}

UIControl* TabTraversalAlgorithm::GetNextControl(UIControl* focusedControl, FocusHelpers::TabDirection dir)
{
    if (focusedControl && root != focusedControl)
    {
        UIControl* parent = focusedControl->GetParent();

        if (parent)
        {
            Vector<UIControl*> children;
            PrepareChildren(parent, children);

            UIControl* res = nullptr;
            if (dir == FocusHelpers::FORWARD)
            {
                res = FindNextControl(focusedControl, children.begin(), children.end(), dir);
            }
            else
            {
                res = FindNextControl(focusedControl, children.rbegin(), children.rend(), dir);
            }

            if (res)
            {
                return res;
            }
            else
            {
                return GetNextControl(parent, dir);
            }
        }
    }
    return nullptr;
}

template <typename It>
UIControl* TabTraversalAlgorithm::FindNextControl(UIControl* focusedControl, It begin, It end, FocusHelpers::TabDirection dir)
{
    auto it = begin;
    while (it != end && *it != focusedControl)
    {
        ++it;
    }

    if (it == end)
    {
        return nullptr;
    }

    ++it;

    for (; it != end; ++it)
    {
        UIControl* res = FindFirstControl(*it, dir);
        if (res)
        {
            return res;
        }
    }

    return nullptr;
}

UIControl* TabTraversalAlgorithm::FindFirstControl(UIControl* control, FocusHelpers::TabDirection dir)
{
    if (FocusHelpers::CanFocusControl(control))
    {
        return control;
    }

    Vector<UIControl*> children;
    PrepareChildren(control, children);

    if (dir == FocusHelpers::FORWARD)
    {
        return FindFirstControlRecursive(children.begin(), children.end(), dir);
    }
    else
    {
        return FindFirstControlRecursive(children.rbegin(), children.rend(), dir);
    }
}

template <typename It>
UIControl* TabTraversalAlgorithm::FindFirstControlRecursive(It begin, It end, FocusHelpers::TabDirection dir)
{
    for (auto it = begin; it != end; ++it)
    {
        UIControl* res = FindFirstControl(*it, dir);
        if (res)
        {
            return res;
        }
    }
    return nullptr;
}

void TabTraversalAlgorithm::PrepareChildren(UIControl* control, Vector<UIControl*>& children)
{
    children.clear();
    children.reserve(control->GetChildren().size());
    children.insert(children.end(), control->GetChildren().begin(), control->GetChildren().end());

    std::stable_sort(children.begin(), children.end(), [](UIControl* c1, UIControl* c2) {
        UIFocusComponent* f1 = c1->GetComponent<UIFocusComponent>();
        if (f1 == nullptr)
        {
            return false;
        }

        UIFocusComponent* f2 = c2->GetComponent<UIFocusComponent>();
        return f2 == nullptr || f1->GetTabOrder() < f2->GetTabOrder(); // important: f1 != nullptr
    });
}
}
