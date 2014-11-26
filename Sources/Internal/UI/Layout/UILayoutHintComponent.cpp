//
//  UILayoutHintComponent.cpp
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#include "UILayoutHintComponent.h"

namespace DAVA
{
    const UIComponent::eType UILayoutHintComponent::TYPE = UIComponent::COMPONENT_LAYOUT_HINT;
    
    UILayoutHintComponent::UILayoutHintComponent()
    {
        
    }
    
    UILayoutHintComponent::~UILayoutHintComponent()
    {
        
    }
    
    UIComponent::eType UILayoutHintComponent::GetType() const
    {
        return TYPE;
    }
    
    const Vector2 &UILayoutHintComponent::GetMeasuredSize() const
    {
        return measuredSize;
    }
    
    void UILayoutHintComponent::SetMeasuredSize(const Vector2 &size)
    {
        measuredSize = size;
    }

}
