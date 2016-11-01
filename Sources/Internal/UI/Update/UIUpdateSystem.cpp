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
}

void UIUpdateSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
}

void UIUpdateSystem::OnControlVisible(UIControl* control)
{
    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component)
    {
        components.push_back(component);
    }
}

void UIUpdateSystem::OnControlInvisible(UIControl* control)
{
    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component)
    {
        components.remove(component);
    }
}

void UIUpdateSystem::Process(float32 elapsedTime)
{
    for (UIUpdateComponent* c : components)
    {
        const auto& f = c->GetUpdateFunction();
        if (f)
        {
            f(elapsedTime);
        }
    }
}
}
