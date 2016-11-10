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
        UICustomUpdateDeltaComponent* customDeltaComponent = control->GetComponent<UICustomUpdateDeltaComponent>();
        binds.emplace_back(component, customDeltaComponent);
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT && control->IsVisible())
    {
        UIUpdateComponent* updateComponent = control->GetComponent<UIUpdateComponent>();
        if (updateComponent)
        {
            auto it = std::find_if(binds.begin(), binds.end(), [updateComponent](const UpdateBind& b) {
                return b.updateComponent == updateComponent;
            });
            if (it != binds.end())
            {
                it->customDeltaComponent = DynamicTypeCheck<UICustomUpdateDeltaComponent*>(component);
            }
        }
    }
}

void UIUpdateSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT && control->IsVisible())
    {
        binds.erase(std::remove_if(binds.begin(), binds.end(), [component](const UpdateBind& b)
                                   {
                                       return b.updateComponent == component;
                                   }));
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT && control->IsVisible())
    {
        auto it = std::find_if(binds.begin(), binds.end(), [component](const UpdateBind& b) {
            return b.customDeltaComponent == component;
        });
        if (it != binds.end())
        {
            it->customDeltaComponent = nullptr;
        }
    }
}

void UIUpdateSystem::OnControlVisible(UIControl* control)
{
    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component)
    {
        UICustomUpdateDeltaComponent* customDeltaComponent = control->GetComponent<UICustomUpdateDeltaComponent>();
        binds.emplace_back(component, customDeltaComponent);
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

UIUpdateSystem::UpdateBind::UpdateBind(UIComponent* uc, UICustomUpdateDeltaComponent* cdc)
    : updateComponent(uc)
    , customDeltaComponent(cdc)
{
}
}
