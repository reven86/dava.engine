//
//  UILayoutComponent.cpp
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#include "UILayoutComponent.h"

#include "UILayout.h"

namespace DAVA
{
    const UIComponent::eType UILayoutComponent::TYPE = UIComponent::COMPONENT_LAYOUT;
    
    UILayoutComponent::UILayoutComponent() : layoutType(LAYOUT_ALIGN)
    {
        
    }
    
    UILayoutComponent::~UILayoutComponent()
    {
    }
    
    UIComponent::eType UILayoutComponent::GetType() const
    {
        return TYPE;
    }
    
    UILayoutComponent *UILayoutComponent::Clone() const 
    {
        UILayoutComponent *newComponent = new UILayoutComponent();
        newComponent->SetLayout(layoutType);
        return newComponent;
    }
    
    UILayoutComponent::eLayoutType UILayoutComponent::GetLayout() const
    {
        return layoutType;
    }
    
    void UILayoutComponent::SetLayout(eLayoutType _layoutType)
    {
        layoutType = _layoutType;
    }
}
