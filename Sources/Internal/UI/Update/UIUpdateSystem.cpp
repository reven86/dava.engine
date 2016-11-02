#include "UIUpdateSystem.h"
#include "UIUpdateComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
void UIUpdateSystem::RegisterControl(UIControl* control)
{
}

void UIUpdateSystem::UnregisterControl(UIControl* control)
{
}

void UIUpdateSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT && control->IsVisible())
    {
        components.insert(component);
    }
}

void UIUpdateSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT)
    {
        components.erase(component);
    }
}

void UIUpdateSystem::OnControlVisible(UIControl* control)
{
    uint32 count = control->GetComponentCount<UIUpdateComponent>();
    for (uint32 i = 0; i < count; ++i)
    {
        UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>(i);
        if (component)
        {
            components.insert(component);
        }
    }
}

void UIUpdateSystem::OnControlInvisible(UIControl* control)
{
    uint32 count = control->GetComponentCount<UIUpdateComponent>();
    for (uint32 i = 0; i < count; ++i)
    {
        UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>(i);
        if (component)
        {
            components.erase(component);
        }
    }
}

void UIUpdateSystem::Process(float32 elapsedTime)
{
    for (UIComponent* c : components)
    {
        if (c->GetControl())
        {
            c->GetControl()->Update(elapsedTime);
        }
    }
}
}
