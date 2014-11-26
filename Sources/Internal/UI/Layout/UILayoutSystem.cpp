//
//  UILayoutSystem.cpp
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#include "UILayoutSystem.h"

#include "../UIControl.h"
#include "UILayoutComponent.h"
#include "UILayoutHintComponent.h"

namespace DAVA
{
    UILayoutSystem::UILayoutSystem()
    {
        
    }
    
    UILayoutSystem::~UILayoutSystem()
    {
        
    }
    
    void UILayoutSystem::ApplayLayout(UIControl *control)
    {
        MeasureChildren(control);
        LayoutChildren(control);
    }
    
    void UILayoutSystem::MeasureChildren(UIControl *control)
    {
        const List<UIControl*> &children = control->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            MeasureChildren(*it);
        }
        
        UILayoutComponent *layoutComponent = control->GetComponent<UILayoutComponent>();
        if (layoutComponent)
        {
            UILayoutHintComponent *hintComponent = control->GetComponent<UILayoutHintComponent>();
            if (hintComponent)
            {
                Vector2 size = layoutComponent->MeasureSize(control);
                hintComponent->SetMeasuredSize(size);
            }
        }
    }
    
    void UILayoutSystem::LayoutChildren(UIControl *control)
    {
        UILayoutComponent *layoutComponent = control->GetComponent<UILayoutComponent>();
        if (layoutComponent)
            layoutComponent->ApplayLayout(control);
        
        const List<UIControl*> &children = control->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
            LayoutChildren(*it);
    }
}
