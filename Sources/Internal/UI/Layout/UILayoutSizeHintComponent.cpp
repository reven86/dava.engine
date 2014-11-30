//
//  UILayoutSizeHintComponent.cpp
//  Framework
//
//  Created by Dmitry Belsky on 28.11.14.
//
//

#include "UILayoutSizeHintComponent.h"

namespace DAVA
{
    const UIComponent::eType UILayoutSizeHintComponent::TYPE = UIComponent::COMPONENT_LAYOUT_SIZE_HINT;

    UILayoutSizeHintComponent::UILayoutSizeHintComponent()
        : horizontalPolicy(FIXED_SIZE)
        , horizontalValue(100)
        , verticalPolicy(FIXED_SIZE)
        , verticalValue(50)
    {
        
    }
    
    UILayoutSizeHintComponent::~UILayoutSizeHintComponent()
    {
        
    }
    
    UIComponent::eType UILayoutSizeHintComponent::GetType() const
    {
        return TYPE;
    }

    UILayoutSizeHintComponent *UILayoutSizeHintComponent::Clone() const
    {
        UILayoutSizeHintComponent *newComponent = new UILayoutSizeHintComponent();
        newComponent->SetHorizontalPolicy(horizontalPolicy);
        newComponent->SetHorizontalValue(horizontalValue);
        newComponent->SetVerticalPolicy(verticalPolicy);
        newComponent->SetVerticalValue(verticalValue);
        newComponent->SetMeasuredSize(measuredSize);
        return newComponent;
    }

    UILayoutSizeHintComponent::eSizePolicy UILayoutSizeHintComponent::GetHorizontalPolicy() const
    {
        return horizontalPolicy;
    }
    
    void UILayoutSizeHintComponent::SetHorizontalPolicy(eSizePolicy policy)
    {
        horizontalPolicy = policy;
    }
    
    float UILayoutSizeHintComponent::GetHorizontalValue() const
    {
        return horizontalValue;
    }
    
    void UILayoutSizeHintComponent::SetHorizontalValue(float value)
    {
        horizontalValue = value;
    }

    UILayoutSizeHintComponent::eSizePolicy UILayoutSizeHintComponent::GetVerticalPolicy() const
    {
        return verticalPolicy;
    }
    
    void UILayoutSizeHintComponent::SetVerticalPolicy(eSizePolicy policy)
    {
        verticalPolicy = policy;
    }
    
    float UILayoutSizeHintComponent::GetVerticalValue() const
    {
        return verticalValue;
    }

    void UILayoutSizeHintComponent::SetVerticalValue(float value)
    {
        verticalValue = value;
    }
    
    const Vector2 &UILayoutSizeHintComponent::GetMeasuredSize() const
    {
        return measuredSize;
    }
    
    void UILayoutSizeHintComponent::SetMeasuredSize(const Vector2 size)
    {
        measuredSize = size;
    }

    UILayoutSizeHintComponent::eSizePolicy UILayoutSizeHintComponent::GetPolicy(eUIOrientation orientation)
    {
        return orientation == UI_ORIENTATION_HORIZONTAL ? horizontalPolicy : verticalPolicy;
    }
    
    float UILayoutSizeHintComponent::GetValue(eUIOrientation orientation)
    {
        return orientation == UI_ORIENTATION_HORIZONTAL ? horizontalValue : verticalValue;
    }
    

}
