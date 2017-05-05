#include "UIScrollSystem.h"

#include "UI/UIControl.h"
#include "UI/UIScrollViewContainer.h"
#include "UI/UIControlHelpers.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
UIScrollSystem::UIScrollSystem()
{
}

UIScrollSystem::~UIScrollSystem()
{
}

void UIScrollSystem::RegisterControl(UIControl* control)
{
    if (control->GetComponent(UIComponent::SCROLL_COMPONENT))
    {
        scrollViewContainers.push_back(DynamicTypeCheck<UIScrollViewContainer*>(control));
    }
}

void UIScrollSystem::UnregisterControl(UIControl* control)
{
    if (control->GetComponent(UIComponent::SCROLL_COMPONENT))
    {
        auto it = std::find(scrollViewContainers.begin(), scrollViewContainers.end(), control);
        if (it != scrollViewContainers.end())
        {
            scrollViewContainers.erase(it);
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void UIScrollSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::SCROLL_COMPONENT)
    {
        scrollViewContainers.push_back(DynamicTypeCheck<UIScrollViewContainer*>(control));
    }
}

void UIScrollSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::SCROLL_COMPONENT)
    {
        auto it = std::find(scrollViewContainers.begin(), scrollViewContainers.end(), control);
        if (it != scrollViewContainers.end())
        {
            scrollViewContainers.erase(it);
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void UIScrollSystem::Process(DAVA::float32 elapsedTime)
{
    for (UIScrollViewContainer* container : scrollViewContainers)
    {
        container->Update(elapsedTime);
    }

    for (const ScheduledControl& c : scheduledControls)
    {
        ScrollToScheduledControl(c);
    }

    scheduledControls.clear();
}

void UIScrollSystem::PrepareForScreenshot(UIControl* control)
{
    PrepareForScreenshotImpl(control);
    for (ScheduledControl& c : scheduledControls)
    {
        if (c.marked)
        {
            ScrollToScheduledControl(c);
        }
    }
}

void UIScrollSystem::PrepareForScreenshotImpl(UIControl* control)
{
    for (UIControl* c : control->GetChildren())
    {
        PrepareForScreenshot(c);
        if (c->GetComponent(UIComponent::SCROLL_COMPONENT) != nullptr)
        {
            c->Update(0);
        }

        auto it = std::find_if(scheduledControls.begin(), scheduledControls.end(), [control](const ScheduledControl& c) {
            return c.control.Get() == control;
        });
        if (it != scheduledControls.end())
        {
            it->marked = true;
        }
    }
}

void UIScrollSystem::ScheduleScrollToControl(UIControl* control)
{
    ScheduleScrollToControlImpl(control, false, 0.0f);
}

void UIScrollSystem::ScheduleScrollToControlWithAnimation(UIControl* control, float32 animationTime)
{
    ScheduleScrollToControlImpl(control, true, animationTime);
}

void UIScrollSystem::ScheduleScrollToControlImpl(UIControl* control, bool withAnimation, float32 animationTime)
{
    auto it = std::find_if(scheduledControls.begin(), scheduledControls.end(), [control](const ScheduledControl& c) {
        return c.control.Get() == control;
    });

    if (it == scheduledControls.end())
    {
        scheduledControls.emplace_back(control, withAnimation, animationTime);
    }
}

void UIScrollSystem::ScrollToScheduledControl(const ScheduledControl& c)
{
    if (c.withAnimation)
    {
        UIControlHelpers::ScrollToControlWithAnimation(c.control.Get(), c.animationTime);
    }
    else
    {
        UIControlHelpers::ScrollToControl(c.control.Get());
    }
}
}
