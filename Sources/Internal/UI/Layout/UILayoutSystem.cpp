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
#include "UILayoutSizeHintComponent.h"
#include "UILayout.h"
#include "UIAlignLayout.h"
#include "UILinearLayout.h"

namespace DAVA
{
    UILayoutSystem::UILayoutSystem()
        : alignLayout(new UIAlignLayout())
        , linearLayout(new UILinearLayout())
    {
        
    }
    
    UILayoutSystem::~UILayoutSystem()
    {
        SafeRelease(alignLayout);
        SafeRelease(linearLayout);
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
            MeasureChildren(*it);
        
        MeasureControlSize(control);
    }
    
    void UILayoutSystem::MeasureControlSize(UIControl *control)
    {
        UILayoutSizeHintComponent *sizeHintComponent = control->GetComponent<UILayoutSizeHintComponent>();
        if (sizeHintComponent)
        {
            float values[2] = {0.0f, 0.0f};
            for (int i = 0; i < 2; i++)
            {
                eUIOrientation orientation = (eUIOrientation) i;
                
                switch (sizeHintComponent->GetPolicy(orientation))
                {
                    case UILayoutSizeHintComponent::FIXED_SIZE:
                        values[i] = sizeHintComponent->GetValue(orientation);
                        break;
                        
                    case UILayoutSizeHintComponent::SIZE_FROM_CONTENT:
                        values[i] = GetControlContentSize(control, orientation);
                        break;
                        
                    case UILayoutSizeHintComponent::SIZE_FROM_MAX_CHILDREN:
                    {
                        float maxVal = 0;
                        const List<UIControl*> &children = control->GetChildren();
                        for (auto it = children.begin(); it != children.end(); ++it)
                            maxVal = Max(maxVal, GetControlSize(control, orientation));
                        
                        values[i] = maxVal;
                        break;
                    }
                        
                    case UILayoutSizeHintComponent::SUM_SIZE_FROM_ALL_CHILDREN:
                    {
                        float maxVal = 0;
                        const List<UIControl*> &children = control->GetChildren();
                        for (auto it = children.begin(); it != children.end(); ++it)
                            maxVal += GetControlSize(control, orientation);
                        
                        values[i] = maxVal;
                        break;
                    }
                        
                    case UILayoutSizeHintComponent::FIRST_CHILDREN:
                    {
                        const List<UIControl*> &children = control->GetChildren();
                        if (!children.empty())
                            values[i] = GetControlSize(*children.begin(), orientation);
                        break;
                    }
                        
                    case UILayoutSizeHintComponent::PERCENT_OF_PARENT:
                        // do nothing, hint will be used later on layout step
                        break;
                        
                    default:
                        DVASSERT(false);
                }
            }
            sizeHintComponent->SetMeasuredSize(Vector2(values));
        }
    }
    
    float UILayoutSystem::GetControlSize(const UIControl *control, eUIOrientation orientation) const
    {
        return control->GetSize().data[orientation];
    }
    
    float UILayoutSystem::GetControlContentSize(const UIControl *control, eUIOrientation orientation) const
    {
        const Vector2 &size = control->GetSize();
        return orientation == UI_ORIENTATION_HORIZONTAL ? size.dx : size.dy;
    }
    
    void UILayoutSystem::LayoutChildren(UIControl *control)
    {
        UILayout *layout = GetLayout(control);
        if (layout)
            layout->Apply(control);
        
        const List<UIControl*> &children = control->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
            LayoutChildren(*it);
    }
    
    UILayout *UILayoutSystem::GetLayout(UIControl *control)
    {
        UILayoutComponent *layoutComponent = control->GetComponent<UILayoutComponent>();
        if (!layoutComponent)
            return NULL;
        
        switch (layoutComponent->GetLayout())
        {
            case UILayoutComponent::LAYOUT_ALIGN:
                return linearLayout;
                
            case UILayoutComponent::LAYOUT_HORIZONTAL:
                return linearLayout;
                
            case UILayoutComponent::LAYOUT_VERTICAL:
                return linearLayout;
                
            default:
                DVASSERT(false);
        }
        return NULL;
    }

}
