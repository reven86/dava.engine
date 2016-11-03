#include "UIUpdateSystem.h"
#include "UIUpdateComponent.h"
#include "UICustomUpdateDeltaComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
void UIUpdateSystem::RegisterControl(UIControl* control)
{
    // Do nothing because this system work only with visible controls
}

void UIUpdateSystem::UnregisterControl(UIControl* control)
{
    // Do nothing because this system work only with visible controls
}

void UIUpdateSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT && control->IsVisible())
    {
        UICustomUpdateDeltaComponent* customUpdate = FindParentComponent(control);
        binds.emplace_back(component, customUpdate);
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT && control->IsVisible())
    {
        UpdateCustomComponentInBinds();
    }
}

void UIUpdateSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT)
    {
        binds.erase(std::remove_if(binds.begin(), binds.end(), [component](const UpdateBind& b)
                                   {
                                       return b.updateComponent == component;
                                   }));
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT && control->IsVisible())
    {
        UpdateCustomComponentInBinds();
    }
}

void UIUpdateSystem::OnControlVisible(UIControl* control)
{
    UICustomUpdateDeltaComponent* customUpdate = FindParentComponent(control);
    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component)
    {
        binds.emplace_back(component, customUpdate);
    }
}

void UIUpdateSystem::OnControlInvisible(UIControl* control)
{
    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component)
    {
        binds.erase(std::remove_if(binds.begin(), binds.end(), [component](const UpdateBind& b)
                                   {
                                       return b.updateComponent == component;
                                   }));
    }

    // If specified control has `UICustomUpdateDeltaComponent` component then
    // we don't need remove this component from all bind of children because
    // all children binds will be remove as *invisible*
}

void UIUpdateSystem::Process(float32 elapsedTime)
{
    for (const UpdateBind& b : binds)
    {
        if (b.customDeltaComponent)
        {
            b.updateComponent->GetControl()->Update(b.customDeltaComponent->GetDelta());
        }
        else
        {
            b.updateComponent->GetControl()->Update(elapsedTime);
        }
    }
}

UICustomUpdateDeltaComponent* UIUpdateSystem::FindParentComponent(UIControl* ctrl)
{
    while (ctrl)
    {
        UICustomUpdateDeltaComponent* comp = ctrl->GetComponent<UICustomUpdateDeltaComponent>();
        if (comp)
        {
            return comp;
        }
        ctrl = ctrl->GetParent();
    }
    return nullptr;
}

void UIUpdateSystem::UpdateCustomComponentInBinds()
{
    // For all binds find near `UICustomUpdateDeltaComponent` set it
    // to current bind. It is faster that search for all controls children and
    // check its components because count of binds is much less that count of
    // controls
    for (UpdateBind& b : binds)
    {
        b.customDeltaComponent = FindParentComponent(b.updateComponent->GetControl());
    }
}
}
