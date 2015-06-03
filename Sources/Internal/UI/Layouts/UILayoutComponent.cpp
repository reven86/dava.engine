#include "UILayoutComponent.h"

namespace DAVA
{
    UILayoutComponent::UILayoutComponent()
    {
        
    }
    
    UILayoutComponent::UILayoutComponent(const UILayoutComponent &src)
        : type(src.type)
    {
        
    }
    
    UILayoutComponent::~UILayoutComponent()
    {
        
    }
    
    UILayoutComponent* UILayoutComponent::Clone()
    {
        return new UILayoutComponent(*this);
    }

    UILayoutComponent::eLayoutType UILayoutComponent::GetLayoutType() const
    {
        return type;
    }
    
    void UILayoutComponent::SetLayoutType(eLayoutType _type)
    {
        type = _type;
    }
    
    int32 UILayoutComponent::GetLayoutTypeAsInt() const
    {
        return static_cast<int32>(GetLayoutType());
    }
    
    void UILayoutComponent::SetLayoutTypeFromInt(int32 type)
    {
        SetLayoutType(static_cast<eLayoutType>(type));
    }

}
