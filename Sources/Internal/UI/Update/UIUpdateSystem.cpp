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
        modified = true;
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
        modified = true;
    }
}

void UIUpdateSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIComponent::UPDATE_COMPONENT)
    {
        UICustomUpdateDeltaComponent* customDeltaComponent = control->GetComponent<UICustomUpdateDeltaComponent>();
        binds.emplace_back(static_cast<UIUpdateComponent*>(component), customDeltaComponent);
        modified = true;
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
                modified = true;
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
        modified = true;
    }
    else if (component->GetType() == UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT)
    {
        auto it = std::find_if(binds.begin(), binds.end(), [component](const UpdateBind& b) {
            return b.customDeltaComponent == component;
        });
        if (it != binds.end())
        {
            it->customDeltaComponent = nullptr;
            modified = true;
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

    for (UpdateBind& b : binds)
    {
        b.updated = false;
    }

    modified = false;
    auto it = binds.begin();
    auto itEnd = binds.end();
    for (; it != itEnd; ++it)
    {
        UpdateBind& b = *it;

        if (b.updated)
        {
            continue;
        }
        b.updated = true;

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

        if (modified)
        {
            modified = false;
            it = binds.begin();
            itEnd = binds.end();
            continue;
        }
    }
}

UIUpdateSystem::UpdateBind::UpdateBind(UIUpdateComponent* uc, UICustomUpdateDeltaComponent* cdc)
    : updateComponent(uc)
    , customDeltaComponent(cdc)
{
}
}
