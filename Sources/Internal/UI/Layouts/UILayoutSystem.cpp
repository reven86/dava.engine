#include "UILayoutSystem.h"

#include "UIAnchorLayoutComponent.h"
#include "UILinearLayoutComponent.h"

#include "UIAnchorLayout.h"
#include "UILinearLayout.h"

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
        if (control->GetComponent<UILinearLayoutComponent>())
        {
            return linearLayout;
        }
        else if (control->GetComponent<UIAnchorLayoutComponent>())
        {
            return anchorLayout;
        }
        return nullptr;
    }

}
