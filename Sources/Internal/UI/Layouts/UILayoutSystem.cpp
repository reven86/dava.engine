#include "UILayoutSystem.h"

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

    bool UILayoutSystem::IsRtl() const
    {
        return isRtl;
    }
    
    void UILayoutSystem::SetRtl(bool rtl)
    {
        isRtl = rtl;
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
        
        bool allowHorizontalAnchor = true;
        bool allowVerticalAnchor = true;
        
        if (linearLayoutComponent)
        {
            if (linearLayoutComponent->GetOrientation() == UILinearLayoutComponent::HORIZONTAL)
                allowHorizontalAnchor = false;
            else
                allowVerticalAnchor = false;
            
            ApplyLinearLayout(control, linearLayoutComponent);
        }
        
        ApplyAnchorLayout(control, allowHorizontalAnchor, allowVerticalAnchor);

        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
            DoLayoutPhase(child);
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
                    value = control->GetPreferredSize().data[axis] * hintValue / 100.0f;
                    
                case UISizeHintComponent::PERCENT_OF_PARENT:
                    value = newSize.data[axis]; // ignore
                    break;
                    
            }
            
            if (policy == UISizeHintComponent::PERCENT_OF_CHILDREN_SUM ||
                policy == UISizeHintComponent::PERCENT_OF_MAX_CHILD ||
                policy == UISizeHintComponent::PERCENT_OF_FIRST_CHILD ||
                policy == UISizeHintComponent::PERCENT_OF_LAST_CHILD)
            {
                UILinearLayoutComponent *layout = control->GetComponent<UILinearLayoutComponent>();
                if (layout && layout->GetOrientation() == axis)
                {
                    if (policy == UISizeHintComponent::PERCENT_OF_CHILDREN_SUM && !children.empty())
                        value += layout->GetSpacing() * (children.size() - 1);
                    
                    value += layout->GetPadding() * 2.0f;
                }
            }
            newSize.data[axis] = value;
        }
        
        if (control->GetSize() != newSize)
            control->SetSize(newSize);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Linear Layout
    ////////////////////////////////////////////////////////////////////////////////
    
    void UILayoutSystem::ApplyLinearLayout(UIControl *control, UILinearLayoutComponent *layout)
    {
        float32 fixedSize = 0;
        float32 totalPercent = 0;
        
        const DAVA::List<UIControl*> &children = control->GetChildren();
        if (children.empty())
            return;
        
        bool inverse = isRtl && layout->IsUseRtl() && layout->GetOrientation() == UILinearLayoutComponent::HORIZONTAL;
        
        const int32 axis = static_cast<int32>(layout->GetOrientation());
        for (UIControl *child : children)
        {
            const UISizeHintComponent *sizeHint = child->GetComponent<UISizeHintComponent>();
            if (sizeHint)
            {
                if (sizeHint->GetPolicyByAxis(axis) == UISizeHintComponent::PERCENT_OF_PARENT)
                    totalPercent += sizeHint->GetValueByAxis(axis);
                else
                    fixedSize += child->GetSize().data[axis];
            }
            else
            {
                fixedSize += child->GetSize().data[axis];
            }
        }
        
        const float32 padding = layout->GetPadding();
        const float32 spacing = layout->GetSpacing();
        int32 spacesCount = children.size() - 1;
        float32 contentSize = control->GetSize().data[axis] - padding * 2.0f;
        float32 restSize = contentSize - fixedSize - spacesCount * spacing;
        float32 position = padding;
        if (inverse)
            position = contentSize + padding;
        for (UIControl *child : children)
        {
            float32 size;
            const UISizeHintComponent *sizeHint = child->GetComponent<UISizeHintComponent>();
            if (sizeHint)
            {
                if (sizeHint->GetPolicyByAxis(axis) == UISizeHintComponent::PERCENT_OF_PARENT)
                    size = restSize * sizeHint->GetValueByAxis(axis) / totalPercent;
                else
                    size = child->GetSize().data[axis];
            }
            else
            {
                size = child->GetSize().data[axis];
            }
            
            size = Max(size, 0.0f);
            if (axis == AXIS_X)
            {
                if (inverse)
                    child->SetPosition(Vector2(position - size, child->GetPosition().y));
                else
                    child->SetPosition(Vector2(position, child->GetPosition().y));
                child->SetSize(Vector2(size, child->GetSize().dy));
            }
            else
            {
                child->SetPosition(Vector2(child->GetPosition().x, position));
                child->SetSize(Vector2(child->GetSize().dx, size));
            }
            
            if (inverse)
                position -= size + spacing;
            else
                position += size + spacing;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Anchor Layout
    ////////////////////////////////////////////////////////////////////////////////
    
    void UILayoutSystem::ApplyAnchorLayout(UIControl *control, bool allowHorizontal, bool allowVertical)
    {
        const Vector2 &parentSize = control->GetSize();
        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
        {
            const UISizeHintComponent *sizeHint = child->GetComponent<UISizeHintComponent>();
            if (sizeHint)
            {
                if (allowHorizontal && sizeHint->GetHorizontalPolicy() == UISizeHintComponent::PERCENT_OF_PARENT)
                {
                    float32 size = control->GetSize().dx * sizeHint->GetHorizontalValue() / 100.0f;
                    child->SetSize(Vector2(size, child->GetSize().dy));
                }
                if (allowVertical && sizeHint->GetVerticalPolicy() == UISizeHintComponent::PERCENT_OF_PARENT)
                {
                    float32 size = control->GetSize().dy * sizeHint->GetVerticalValue() / 100.0f;
                    child->SetSize(Vector2(child->GetSize().dx, size));
                }
            }
            
            UIAnchorHintComponent *hint = child->GetComponent<UIAnchorHintComponent>();
            if (hint)
            {
                const Rect &rect = child->GetRect();
                Rect newRect = rect;
                
                if (allowHorizontal && (hint->IsLeftAnchorEnabled() || hint->IsHCenterAnchorEnabled() || hint->IsRightAnchorEnabled()))
                {
                    bool leftEnabled = hint->IsLeftAnchorEnabled();
                    float32 left = hint->GetLeftAnchor();

                    bool rightEnabled = hint->IsRightAnchorEnabled();
                    float32 right = hint->GetRightAnchor();

                    bool hCenterEnabled = hint->IsHCenterAnchorEnabled();
                    float32 hCenter = hint->GetHCenterAnchor();
                    
                    if (isRtl && hint->IsUseRtl())
                    {
                        leftEnabled = hint->IsRightAnchorEnabled();
                        left = hint->GetRightAnchor();
                        
                        rightEnabled = hint->IsLeftAnchorEnabled();
                        right = hint->GetLeftAnchor();
                        
                        hCenter = -hCenter;
                    }
                    
                    GetAxisDataByAnchorData(rect.dx, parentSize.x,
                                            leftEnabled, left,
                                            hCenterEnabled, hCenter,
                                            rightEnabled, right,
                                            newRect.x, newRect.dx);
                    
                }
                if (allowVertical && (hint->IsTopAnchorEnabled() || hint->IsVCenterAnchorEnabled() || hint->IsBottomAnchorEnabled()))
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
