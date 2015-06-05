#include "UILinearLayout.h"

#include "UI/UIControl.h"

namespace DAVA
{
    UILinearLayout::UILinearLayout()
    {
        
    }
    
    UILinearLayout::~UILinearLayout()
    {
        
    }
    
    void UILinearLayout::MeasureSize(UIControl *control)
    {
        
    }
    
    void UILinearLayout::ApplyLayout(UIControl *control)
    {
        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
        {
            
        }
    }
}
