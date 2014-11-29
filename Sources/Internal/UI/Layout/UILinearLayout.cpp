//
//  UILinearLayout.cpp
//  Framework
//
//  Created by Dmitry Belsky on 29.11.14.
//
//

#include "UILinearLayout.h"

#include "../UIControl.h"

namespace DAVA
{
    UILinearLayout::UILinearLayout()
    {
        
    }
    
    UILinearLayout::~UILinearLayout()
    {
        
    }
    
    void UILinearLayout::Apply(UIControl *control)
    {
        
        const List<UIControl*> &children = control->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            
        }
    }
    
}
