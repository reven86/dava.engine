#include "UIFakeComponent.h"

namespace DAVA
{

UIFakeComponent::UIFakeComponent()
    : value(0)
{
    
}

UIFakeComponent::~UIFakeComponent()
{
    
}

UIFakeComponent* UIFakeComponent::Clone(UIControl * toControl)
{
    UIFakeComponent *component = new UIFakeComponent();
    component->SetControl(toControl);
    
    component->Clone(toControl);
    component->SetValue(value);
    return component;
}

}
