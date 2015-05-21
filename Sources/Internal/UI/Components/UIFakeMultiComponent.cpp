#include "UIFakeMultiComponent.h"

namespace DAVA
{

UIFakeMultiComponent::UIFakeMultiComponent()
    : value(0)
    , strValue("")
{
    
}
    
UIFakeMultiComponent::UIFakeMultiComponent(UIFakeMultiComponent *src)
    : value(src->value)
    , strValue(src->strValue)
{
    
}
    

UIFakeMultiComponent::~UIFakeMultiComponent()
{
    
}

UIFakeMultiComponent* UIFakeMultiComponent::Clone()
{
    return new UIFakeMultiComponent();
}

}
