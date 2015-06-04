#include "UILayoutSystem.h"

#include "UILinearLayout.h"
#include "UIAnchorLayout.h"
#include "UILayoutComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
    UILayoutSystem::UILayoutSystem()
    {
        linearLayout = new UILinearLayout();
        anchorLayout = new UIAnchorLayout();
    }
    
    UILayoutSystem::~UILayoutSystem()
    {
        SafeDelete(linearLayout);
        SafeDelete(anchorLayout);
    }

    void UILayoutSystem::ApplyLayout(UIControl *control)
    {
        DoMeasurePhase(control);
        DoLayoutPhase(control);
    }
    
    void UILayoutSystem::DoMeasurePhase(UIControl *control)
    {
        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
            DoMeasurePhase(child);
        
        UILayout *layout = GetLayout(control);
        if (layout)
            layout->MeasureSize(control);
    }
    
    void UILayoutSystem::DoLayoutPhase(UIControl *control)
    {
        UILayout *layout = GetLayout(control);
        if (layout)
            layout->ApplyLayout(control);

        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
            DoMeasurePhase(child);
    }

    UILayout *UILayoutSystem::GetLayout(UIControl *control)
    {
        UILayoutComponent *component = control->GetComponent<UILayoutComponent>();
        if (component)
        {
            switch (component->GetLayoutType())
            {
                case UILayoutComponent::LINEAR_LAYOUT:
                    return linearLayout;
                    
                case UILayoutComponent::ANCHOR_LAYOUT:
                    return anchorLayout;
                    
                default:
                    DVASSERT(false);
                    break;
            }
        }
        return nullptr;
    }

}
