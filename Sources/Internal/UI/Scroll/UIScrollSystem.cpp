#include "UIScrollSystem.h"

#include "UI/UIControl.h"
#include "UI/UIScrollViewContainer.h"
#include "UI/Components/UIComponent.h"
#include "UI/Scroll/UIScrollComponent.h"

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
    if (control->GetComponent(Type::Instance<UIScrollComponent>()))
    {
        scrollViewContainers.push_back(DynamicTypeCheck<UIScrollViewContainer*>(control));
    }
}

void UIScrollSystem::UnregisterControl(UIControl* control)
{
    if (control->GetComponent(Type::Instance<UIScrollComponent>()))
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
    if (component->GetType() == Type::Instance<UIScrollComponent>())
    {
        scrollViewContainers.push_back(DynamicTypeCheck<UIScrollViewContainer*>(control));
    }
}

void UIScrollSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIScrollComponent>())
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
