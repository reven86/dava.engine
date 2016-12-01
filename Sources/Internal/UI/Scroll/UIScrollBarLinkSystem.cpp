#include "UIScrollBarLinkSystem.h"

#include "UI/Scroll/UIScrollBarDelegateComponent.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIScrollBar.h"

namespace DAVA
{
UIScrollBarLinkSystem::UIScrollBarLinkSystem()
{
}

UIScrollBarLinkSystem::~UIScrollBarLinkSystem()
{
}

void UIScrollBarLinkSystem::RegisterControl(UIControl* control)
{
    UIScrollBarDelegateComponent* component = control->GetComponent<UIScrollBarDelegateComponent>();
    if (component)
    {
        if (!TryToRestoreLink(component, nullptr))
        {
            RegisterScrollBarDelegateComponent(component);
        }
    }
    else
    {
        TryToRestoreLink(nullptr, control);
    }
}

void UIScrollBarLinkSystem::UnregisterControl(UIControl* control)
{
    UIScrollBarDelegateComponent* component = control->GetComponent<UIScrollBarDelegateComponent>();
    if (component)
    {
        TryToBreakLink(component, nullptr);
        //            UnregisterScrollBarDelegateComponent(component);
    }
    else
    {
        TryToBreakLink(nullptr, control);
    }
}

void UIScrollBarLinkSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIScrollBarDelegateComponent::C_TYPE)
    {
        //RegisterScrollBarDelegateComponent(static_cast<UIScrollBarDelegateComponent*>(component));
        if (!TryToRestoreLink(static_cast<UIScrollBarDelegateComponent*>(component), nullptr))
        {
            RegisterScrollBarDelegateComponent(static_cast<UIScrollBarDelegateComponent*>(component));
        }
    }
}

void UIScrollBarLinkSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIScrollBarDelegateComponent::C_TYPE)
    {
        //UnregisterScrollBarDelegateComponent(static_cast<UIScrollBarDelegateComponent*>(component));
        TryToBreakLink(static_cast<UIScrollBarDelegateComponent*>(component), nullptr);
    }
}

void UIScrollBarLinkSystem::Process(DAVA::float32 elapsedTime)
{
    for (Link& link : links)
    {
        if (link.linkedControl == nullptr || link.component->IsPathToDelegateDirty())
        {
            UIControl* control = link.component->GetControl()->FindByPath(link.component->GetPathToDelegate());
            if (control != nullptr)
            {
                SetupLink(&link, control);
            }
            else
            {
                BreakLink(&link);
            }
            link.component->ResetPathToDelegateDirty();
        }
    }
}

void UIScrollBarLinkSystem::SetRestoreLinks(bool restore)
{
    restoreLinks = restore;
}

void UIScrollBarLinkSystem::RegisterScrollBarDelegateComponent(UIScrollBarDelegateComponent* component)
{
    links.push_back(Link(component));
}

void UIScrollBarLinkSystem::UnregisterScrollBarDelegateComponent(UIScrollBarDelegateComponent* component)
{
    auto it = std::find_if(links.begin(), links.end(), [component](Link& l) { return l.component == component; });
    if (it != links.end())
    {
        Link& link = *it;
        BreakLink(&link);
        links.erase(it);
    }
}

void UIScrollBarLinkSystem::SetupLink(Link* link, UIControl* control)
{
    UIScrollBarDelegate* delegate = dynamic_cast<UIScrollBarDelegate*>(control);
    DVASSERT(delegate != nullptr);

    UIScrollBar* scrollBar = dynamic_cast<UIScrollBar*>(link->component->GetControl());
    DVASSERT(scrollBar != nullptr);

    if (scrollBar && delegate)
    {
        link->linkedControl = control;
        scrollBar->SetDelegate(delegate);
    }
}

void UIScrollBarLinkSystem::BreakLink(Link* link)
{
    UIScrollBar* scrollBar = dynamic_cast<UIScrollBar*>(link->component->GetControl());
    if (scrollBar)
    {
        scrollBar->SetDelegate(nullptr);
    }
}

bool UIScrollBarLinkSystem::TryToRestoreLink(UIScrollBarDelegateComponent* component, UIControl* linkedControl)
{
    if (restoreLinks)
    {
        Vector<Link>::iterator it;
        if (component != nullptr)
        {
            it = std::find_if(brokenLinks.begin(), brokenLinks.end(), [component](Link& l) { return l.component == component; });
        }
        else if (linkedControl != nullptr)
        {
            it = std::find_if(brokenLinks.begin(), brokenLinks.end(), [linkedControl](Link& l) { return l.linkedControl == linkedControl; });
        }
        else
        {
            DVASSERT(false);
        }

        if (it != brokenLinks.end())
        {
            it->component->SetPathToDelegate(UIControlHelpers::GetPathToOtherControl(it->component->GetControl(), it->linkedControl));
            SetupLink(&(*it), it->linkedControl);

            links.push_back(*it);
            brokenLinks.erase(it);
            return true;
        }
    }

    return false;
}

bool UIScrollBarLinkSystem::TryToBreakLink(UIScrollBarDelegateComponent* component, UIControl* linkedControl)
{
    Vector<Link>::iterator it;
    if (component != nullptr)
    {
        it = std::find_if(links.begin(), links.end(), [component](Link& l) { return l.component == component; });
    }
    else if (linkedControl != nullptr)
    {
        it = std::find_if(links.begin(), links.end(), [linkedControl](Link& l) { return l.linkedControl == linkedControl; });
    }

    if (it != links.end())
    {
        BreakLink(&(*it));

        if (restoreLinks)
        {
            brokenLinks.push_back(*it);
        }
        links.erase(it);

        return true;
    }

    return false;
}
}
