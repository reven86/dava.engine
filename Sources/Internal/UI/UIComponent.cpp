//
//  UIComponent.cpp
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#include "UIComponent.h"

#include "UILayoutComponent.h"
#include "UILayoutSizeHintComponent.h"

namespace DAVA
{
    UIComponent::UIComponent()
    {
    }
    
    UIComponent::~UIComponent()
    {
    }
    
    UIComponent *UIComponent::CreateByType(eType type)
    {
        switch (type)
        {
            case COMPONENT_LAYOUT:
                return new UILayoutComponent();
                
            case COMPONENT_LAYOUT_SIZE_HINT:
                return new UILayoutSizeHintComponent();
                
            case COMPONENT_LAYOUT_ALIGN_HINT:
                return NULL; // TODO
                
            default:
                DVASSERT(false);
                return NULL;
        }
    }

}
