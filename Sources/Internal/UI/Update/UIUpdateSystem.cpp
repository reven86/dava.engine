#include "UIUpdateSystem.h"
#include "UIUpdateComponent.h"
#include "UICustomUpdateDeltaComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
UIUpdateSystem::~UIUpdateSystem()
{
}

void UIUpdateSystem::RegisterControl(UIControl* control)
{
    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component)
    {
        UICustomUpdateDeltaComponent* customDeltaComponent = control->GetComponent<UICustomUpdateDeltaComponent>();
        binds.emplace_back(component, customDeltaComponent);
    }
}

void UIUpdateSystem::UnregisterControl(UIControl* control)
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

void UIUpdateSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT)
    {
        UICustomUpdateDeltaComponent* customDeltaComponent = control->GetComponent<UICustomUpdateDeltaComponent>();
        binds.emplace_back(static_cast<UIUpdateComponent*>(component), customDeltaComponent);
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT)
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
    if (component->GetType() == UIComponent::UPDATE_COMPONENT)
    {
        binds.erase(std::remove_if(binds.begin(), binds.end(), [component](const UpdateBind& b)
                                   {
                                       return b.updateComponent == component;
                                   }));
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT)
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
}

void UIUpdateSystem::OnControlInvisible(UIControl* control)
{
}

void UIUpdateSystem::Process(float32 elapsedTime)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_UI_CONTROL_SYSTEM))
    {
        return;
    }

    for (const UpdateBind& b : binds)
    {
        if (!b.updateComponent->GetUpdateInvisible() && !b.updateComponent->GetControl()->IsVisible())
        {
            // Skip invisible controls if invisible update is disabled
            continue;
        }

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

UIUpdateSystem::UpdateBind::UpdateBind(UIUpdateComponent* uc, UICustomUpdateDeltaComponent* cdc)
    : updateComponent(uc)
    , customDeltaComponent(cdc)
{
}
}
