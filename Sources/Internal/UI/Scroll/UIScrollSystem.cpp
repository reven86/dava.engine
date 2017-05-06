#include "UIScrollSystem.h"

#include "UI/UIControl.h"
#include "UI/UIScrollViewContainer.h"
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
}
}
