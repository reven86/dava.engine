#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

namespace DAVA
{
UILayoutSystem::UILayoutSystem()
    : indexOfSizeProperty(-1)
{
    indexOfSizeProperty = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyIndex(FastName("size"));
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

bool UILayoutSystem::IsDirty() const
{
    return dirty;
}

void UILayoutSystem::SetDirty()
{
    dirty = true;
}

void UILayoutSystem::ResetDirty()
{
    dirty = false;
}

void UILayoutSystem::ApplyLayout(UIControl *control)
{
    for (int32 axis = 0; axis < Vector2::AXIS_COUNT; axis++)
    {
        DoMeasurePhase(control, static_cast<Vector2::eAxis>(axis));
        DoLayoutPhase(control, static_cast<Vector2::eAxis>(axis));
    }
    
    for (UIControl *control : changedControls)
    {
        control->SetPropertyLocalFlag(indexOfSizeProperty, true);
        control->OnSizeChanged();
    }
    changedControls.clear();
}

void UILayoutSystem::DoMeasurePhase(UIControl *control, Vector2::eAxis axis)
{
    const List<UIControl*> &children = control->GetChildren();
    for (UIControl *child : children)
        DoMeasurePhase(child, axis);
    
    UISizePolicyComponent *sizeHint = control->GetComponent<UISizePolicyComponent>();
    if (sizeHint)
    {
        MeasureControl(control, sizeHint, axis);
    }
}

void UILayoutSystem::DoLayoutPhase(UIControl *control, Vector2::eAxis axis)
{
    UILinearLayoutComponent *linearLayoutComponent = control->GetComponent<UILinearLayoutComponent>();
    bool anchorOnlyIgnoredControls = false;
    if (linearLayoutComponent && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
    {
        ApplyLinearLayout(control, linearLayoutComponent, axis);
        anchorOnlyIgnoredControls = true;
    }

    ApplyAnchorLayout(control, axis, anchorOnlyIgnoredControls);

    const List<UIControl*> &children = control->GetChildren();
    for (UIControl *child : children)
        DoLayoutPhase(child, axis);
}

////////////////////////////////////////////////////////////////////////////////
// Measuring
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::MeasureControl(UIControl *control, UISizePolicyComponent *sizeHint, Vector2::eAxis axis)
{
    DVASSERT(sizeHint);
    
    Vector2 newSize = control->GetSize();
    
    bool skipInvisible = false;
    
    
    const DAVA::List<UIControl*> &children = control->GetChildren();
    UILinearLayoutComponent *linearLayout = nullptr;
    
    linearLayout = control->GetComponent<UILinearLayoutComponent>();
    if (linearLayout)
        skipInvisible = linearLayout->IsSkipInvisibleControls();
    
    UISizePolicyComponent::eSizePolicy policy = sizeHint->GetPolicyByAxis(axis);
    float32 hintValue = sizeHint->GetValueByAxis(axis);
    float32 value = 0;
    
    switch (policy)
    {
        case UISizePolicyComponent::IGNORE_SIZE:
            value = newSize.data[axis]; // ignore
            break;
            
        case UISizePolicyComponent::FIXED_SIZE:
            value = hintValue;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM:
            for (UIControl *child : children)
            {
                if (!HaveToSkipControl(child, skipInvisible))
                    value += child->GetSize().data[axis];
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_MAX_CHILD:
            for (UIControl *child : children)
            {
                if (!HaveToSkipControl(child, skipInvisible))
                    value = Max(value, child->GetSize().data[axis]);
            }
            value = value * hintValue / 100.0f;
            break;

        case UISizePolicyComponent::PERCENT_OF_FIRST_CHILD:
            if (!children.empty())
                value = children.front()->GetSize().data[axis];
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_LAST_CHILD:
            if (!children.empty())
                value = children.back()->GetSize().data[axis];
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CONTENT:
        {
            Vector2 constraints(-1.0f, -1.0f);
            if (control->IsHeightDependsOnWidth() && axis == Vector2::AXIS_Y)
            {
                constraints.x = newSize.x;
            }
            value = control->GetContentPreferredSize(constraints).data[axis] * hintValue / 100.0f;
            break;
        }
            
        case UISizePolicyComponent::PERCENT_OF_PARENT:
            value = newSize.data[axis]; // ignore
            break;
            
    }
    
    if (policy == UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM ||
        policy == UISizePolicyComponent::PERCENT_OF_MAX_CHILD ||
        policy == UISizePolicyComponent::PERCENT_OF_FIRST_CHILD ||
        policy == UISizePolicyComponent::PERCENT_OF_LAST_CHILD)
    {
        if (linearLayout && linearLayout->GetAxis() == axis)
        {
            if (policy == UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM && !children.empty())
                value += linearLayout->GetSpacing() * (children.size() - 1);
            
            value += linearLayout->GetPadding() * 2.0f;
        }
    }
    
    if (policy != UISizePolicyComponent::PERCENT_OF_PARENT && policy != UISizePolicyComponent::IGNORE_SIZE)
    {
        value = Clamp(value, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));
    }
    newSize.data[axis] = value;
    
    if (control->GetSize() != newSize)
    {
        control->SetSize(newSize);
        changedControls.insert(control);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Linear Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyLinearLayout(UIControl *control, UILinearLayoutComponent *layout, Vector2::eAxis axis)
{
    float32 fixedSize = 0.0f;
    float32 totalPercent = 0.0f;
    
    const DAVA::List<UIControl*> &children = control->GetChildren();
    if (children.empty())
        return;
    
    bool inverse = isRtl && layout->IsUseRtl() && layout->GetOrientation() == UILinearLayoutComponent::HORIZONTAL;
    const bool skipInvisible = layout->IsSkipInvisibleControls();

    int32 childrenCount = 0;
    for (UIControl *child : children)
    {
        if (HaveToSkipControl(child, skipInvisible))
            continue;
        
        childrenCount++;
        
        const UISizePolicyComponent *sizeHint = child->GetComponent<UISizePolicyComponent>();
        if (sizeHint)
        {
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
                totalPercent += sizeHint->GetValueByAxis(axis);
            else
                fixedSize += child->GetSize().data[axis];
        }
        else
        {
            fixedSize += child->GetSize().data[axis];
        }
    }
    
    if (childrenCount > 0)
    {
        float32 padding = layout->GetPadding();
        float32 spacing = layout->GetSpacing();
        
        int32 spacesCount = childrenCount - 1;
        float32 contentSize = control->GetSize().data[axis] - padding * 2.0f;
        float32 restSize = contentSize - fixedSize - spacesCount * spacing;
        
        if (totalPercent < 100.0f - EPSILON && restSize > EPSILON)
        {
            bool dynamicPadding = layout->IsDynamicPadding();
            bool dynamicSpacing = layout->IsDynamicSpacing();
            if (dynamicPadding || (dynamicSpacing && spacesCount > 0))
            {
                int32 cnt = 0;
                if (dynamicPadding)
                    cnt = 2;
                if (dynamicSpacing)
                    cnt += spacesCount;
                
                float32 delta = restSize * (1.0f - totalPercent / 100.0f) / cnt;
                if (dynamicPadding)
                    padding += delta;
                
                if (dynamicSpacing)
                    spacing += delta;
            }
        }
        
        float32 position = padding;
        if (inverse)
            position = control->GetSize().data[axis] - padding;

        for (UIControl *child : children)
        {
            if (HaveToSkipControl(child, skipInvisible))
                continue;

            float32 size = 0.0f;
            const UISizePolicyComponent *sizeHint = child->GetComponent<UISizePolicyComponent>();
            if (sizeHint)
            {
                if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
                {
                    if (totalPercent < EPSILON)
                        size = 0.0f;
                    else
                        size = restSize * sizeHint->GetValueByAxis(axis) / Max(totalPercent, 100.0f);
                    size = Clamp(size, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));
                }
                else
                    size = child->GetSize().data[axis];
            }
            else
            {
                size = child->GetSize().data[axis];
            }
            
            size = Max(size, 0.0f);
            if (axis == Vector2::AXIS_X)
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
            
            changedControls.insert(child);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Anchor Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyAnchorLayout(UIControl *control, Vector2::eAxis axis, bool onlyForIgnoredControls)
{
    const Vector2 &parentSize = control->GetSize();
    const List<UIControl*> &children = control->GetChildren();
    for (UIControl *child : children)
    {
        if (onlyForIgnoredControls && child->GetComponentCount(UIComponent::IGNORE_LAYOUT_COMPONENT) == 0)
            continue;
        
        const UISizePolicyComponent *sizeHint = child->GetComponent<UISizePolicyComponent>();
        if (sizeHint)
        {
            Vector2 newSize = child->GetSize();
            
            float32 s = newSize.data[axis];
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                s = control->GetSize().data[axis] * sizeHint->GetValueByAxis(axis) / 100.0f;
                s = Clamp(s, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));
            }
            newSize.data[axis] = s;
            
            if (newSize != child->GetSize())
            {
                child->SetSize(newSize);
                changedControls.insert(child);
            }
        }
        
        UIAnchorComponent *hint = child->GetComponent<UIAnchorComponent>();
        if (hint)
        {
            const Rect &rect = child->GetRect();
            Rect newRect = rect;
            
            if (axis == Vector2::AXIS_X && (hint->IsLeftAnchorEnabled() || hint->IsHCenterAnchorEnabled() || hint->IsRightAnchorEnabled()))
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
            if (axis == Vector2::AXIS_Y && (hint->IsTopAnchorEnabled() || hint->IsVCenterAnchorEnabled() || hint->IsBottomAnchorEnabled()))
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
                changedControls.insert(child);
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

bool UILayoutSystem::HaveToSkipControl(UIControl *control, bool skipInvisible) const
{
    return (skipInvisible && !control->GetVisible()) || control->GetComponentCount(UIComponent::IGNORE_LAYOUT_COMPONENT) > 0;
}

}
