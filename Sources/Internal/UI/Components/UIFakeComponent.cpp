#include "UIFakeComponent.h"

namespace DAVA
{

UIFakeComponent::UIFakeComponent()
    : value(0)
{
    
}
    
UIFakeComponent::UIFakeComponent(UIFakeComponent *src)
    : value(src->value)
{
}


UIFakeComponent::~UIFakeComponent()
{
    
}

UIFakeComponent* UIFakeComponent::Clone()
{
    return new UIFakeComponent(this);
}

}
