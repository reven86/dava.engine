#include "FocusHelpers.h"

#include "UIFocusComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
bool FocusHelpers::CanFocusControl(UIControl* control)
{
    if (control == nullptr)
    {
        return false;
    }

    UIFocusComponent* focus = control->GetComponent<UIFocusComponent>();
    return focus != nullptr && control->IsVisible() &&
    !control->GetDisabled() && focus &&
    focus->IsEnabled() && focus->GetPolicy() == UIFocusComponent::FOCUSABLE;
}
}
