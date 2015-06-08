#include "UILayoutSystem.h"

#include "UIAnchorLayoutComponent.h"
#include "UILinearLayoutComponent.h"
#include "UIAnchorHintComponent.h"
#include "UISizeHintComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
    UILayoutSystem::UILayoutSystem()
    {
    }
    
    UILayoutSystem::~UILayoutSystem()
    {
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
        
        UISizeHintComponent *sizeHint = control->GetComponent<UISizeHintComponent>();
        if (sizeHint)
        {
            MeasureControl(control, sizeHint);
        }
    }
    
    void UILayoutSystem::DoLayoutPhase(UIControl *control)
    {
        UILinearLayoutComponent *linearLayoutComponent = control->GetComponent<UILinearLayoutComponent>();
        
        if (linearLayoutComponent)
        {
            ApplyLinearLayout(control, linearLayoutComponent);
        }
        else if (control->GetComponent<UIAnchorLayoutComponent>())
        {
            ApplyAnchorLayout(control);
        }

        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
            DoMeasurePhase(child);
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Measuring
    ////////////////////////////////////////////////////////////////////////////////
    
    void UILayoutSystem::MeasureControl(UIControl *control, UISizeHintComponent *sizeHint)
    {
        DVASSERT(sizeHint);
        
        Vector2 newSize = control->GetSize();
        
        const DAVA::List<UIControl*> &children = control->GetChildren();
        
        for (int32 axis = 0; axis < 2; axis++)
        {
            UISizeHintComponent::eSizePolicy policy = sizeHint->GetPolicyByAxis(axis);
            float32 hintValue = sizeHint->GetValueByAxis(axis);
            float32 value = 0;
            
            switch (policy)
            {
                case UISizeHintComponent::IGNORE:
                    value = newSize.data[axis]; // ignore
                    break;
                    
                case UISizeHintComponent::FIXED_SIZE:
                    value = hintValue;
                    break;
                    
                case UISizeHintComponent::PERCENT_OF_CHILDREN_SUM:
                    for (UIControl *child : children)
                        value += child->GetSize().data[axis];
                    value = value * hintValue / 100.0f;
                    break;
                    
                case UISizeHintComponent::PERCENT_OF_MAX_CHILD:
                    for (UIControl *child : children)
                        value = Max(value, child->GetSize().data[axis]);
                    value = value * hintValue / 100.0f;
                    break;

                case UISizeHintComponent::PERCENT_OF_FIRST_CHILD:
                    if (!children.empty())
                        value = children.front()->GetSize().data[axis];
                    value = value * hintValue / 100.0f;
                    break;
                    
                case UISizeHintComponent::PERCENT_OF_LAST_CHILD:
                    if (!children.empty())
                        value = children.back()->GetSize().data[axis];
                    value = value * hintValue / 100.0f;
                    break;
                    
                case UISizeHintComponent::PERCENT_OF_CONTENT:
                    value = control->GetContentSize().data[axis] * hintValue / 100.0f;
                    
                case UISizeHintComponent::PERCENT_OF_PARENT:
                    value = newSize.data[axis]; // ignore
                    break;
                    
            }
            newSize.data[axis] = value;
        }
        
        if (control->GetSize() != newSize)
            control->SetSize(newSize);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Linear Layout
    ////////////////////////////////////////////////////////////////////////////////
    
    void UILayoutSystem::ApplyLinearLayout(UIControl *control, UILinearLayoutComponent *linearLayoutComponent)
    {
        
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Anchor Layout
    ////////////////////////////////////////////////////////////////////////////////
    
    void UILayoutSystem::ApplyAnchorLayout(UIControl *control)
    {
        const Vector2 &parentSize = control->GetSize();
        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
        {
            UIAnchorHintComponent *hint = child->GetComponent<UIAnchorHintComponent>();
            if (hint)
            {
                const Rect &rect = child->GetRect();
                Rect newRect = rect;
                
                if (hint->IsLeftAnchorEnabled() || hint->IsHCenterAnchorEnabled() || hint->IsRightAnchorEnabled())
                {
                    GetAxisDataByAnchorData(rect.dx, parentSize.x,
                                            hint->IsLeftAnchorEnabled(), hint->GetLeftAnchor(),
                                            hint->IsHCenterAnchorEnabled(), hint->GetHCenterAnchor(),
                                            hint->IsRightAnchorEnabled(), hint->GetRightAnchor(),
                                            newRect.x, newRect.dx);
                    
                }
                if (hint->IsTopAnchorEnabled() || hint->IsVCenterAnchorEnabled() || hint->IsBottomAnchorEnabled())
                {
                    GetAxisDataByAnchorData(rect.dy, parentSize.y,
                                            hint->IsTopAnchorEnabled(), hint->GetTopAnchor(),
                                            hint->IsVCenterAnchorEnabled(), hint->GetVCenterAnchor(),
                                            hint->IsBottomAnchorEnabled(), hint->GetBottomAnchor(),
                                            newRect.y, newRect.dy);
                }
                
                if (rect != newRect)
                {
                    child->SetSize(newRect.GetSize());
                    child->SetPosition(newRect.GetPosition() + child->GetPivotPoint());
                }
            }
        }
    }
    
    void UILayoutSystem::GetAxisDataByAnchorData(float32 size, float32 parentSize,
                                                 bool firstSideAnchorEnabled, float32 firstSideAnchor,
                                                 bool centerAnchorEnabled, float32 centerAnchor,
                                                 bool secondSideAnchorEnabled, float32 secondSideAnchor,
                                                 float32 &newPos, float32 &newSize)
    {
        if (firstSideAnchorEnabled && secondSideAnchorEnabled)
        {
            newPos = firstSideAnchor;
            newSize = parentSize - (firstSideAnchor + secondSideAnchor);
        }
        else if (firstSideAnchorEnabled && centerAnchorEnabled)
        {
            newPos = firstSideAnchor;
            newSize = parentSize / 2.0f - (firstSideAnchor - centerAnchor);
        }
        else if (centerAnchorEnabled && secondSideAnchorEnabled)
        {
            newPos = parentSize / 2.0f + centerAnchor;
            newSize = parentSize / 2.0f - (centerAnchor + secondSideAnchor);
        }
        else if (firstSideAnchorEnabled)
        {
            newPos = firstSideAnchor;
            newSize = size;
        }
        else if (secondSideAnchorEnabled)
        {
            newPos = parentSize - (size + secondSideAnchor);
            newSize = size;
        }
        else if (centerAnchorEnabled)
        {
            newPos = (parentSize - size) / 2.0f + centerAnchor;
            newSize = size;
        }
    }
    
    void UILayoutSystem::GetAnchorDataByAxisData(float32 size, float32 pos, float32 parentSize, bool firstSideAnchorEnabled, bool centerAnchorEnabled, bool secondSideAnchorEnabled, float32 &firstSideAnchor, float32 &centerAnchor, float32 &secondSideAnchor)
    {
        if (firstSideAnchorEnabled && secondSideAnchorEnabled)
        {
            firstSideAnchor = pos;
            secondSideAnchor = parentSize - (pos + size);
        }
        else if (firstSideAnchorEnabled && centerAnchorEnabled)
        {
            firstSideAnchor = pos;
            centerAnchor = pos + size - parentSize / 2.0f;
        }
        else if (centerAnchorEnabled && secondSideAnchorEnabled)
        {
            centerAnchor = pos - parentSize / 2.0f;
            secondSideAnchor = parentSize - (pos + size);
        }
        else if (firstSideAnchorEnabled)
        {
            firstSideAnchor = pos;
        }
        else if (secondSideAnchorEnabled)
        {
            secondSideAnchor = parentSize - (pos + size);
        }
        else if (centerAnchorEnabled)
        {
            centerAnchor = pos - parentSize / 2.0f + size / 2.0f;
        }
    }

}
