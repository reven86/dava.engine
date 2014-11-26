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
    
    UILayoutComponent::UILayoutComponent() : layout(NULL)
    {
        
    }
    
    UILayoutComponent::~UILayoutComponent()
    {
        SafeRelease(layout);
    }
    
    UIComponent::eType UILayoutComponent::GetType() const
    {
        return TYPE;
    }
    
    Vector2 UILayoutComponent::MeasureSize(UIControl *control)
    {
        if (layout)
            return layout->MeasureSize(control);
        return Vector2(0, 0);
    }

    void UILayoutComponent::ApplayLayout(UIControl *control)
    {
        if (layout)
            layout->Apply(control);
    }

}
