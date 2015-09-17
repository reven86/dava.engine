#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "Platform/SystemTimer.h"

namespace DAVA
{
    
struct UILayoutSystem::ControlDescr
{
    UIControl *control;
    int32 flags;
    int32 firstChild;
    int32 lastChild;
    Vector2 size;
    Vector2 position;
    
    bool HasChildren() const {
        return lastChild >= firstChild;
    }
};


namespace
{
    const int32 FLAG_SIZE_CHANGED = 0x01;
    const int32 FLAG_POSITION_CHANGED = 0x02;
    const int32 FLAG_X_SIZE_CALCULATED = 0x04;
    const int32 FLAG_Y_SIZE_CALCULATED = 0x08;
}
    
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

void UILayoutSystem::ApplyLayout(UIControl *workControl)
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    CollectControls(workControl);
    
    for (int32 axisIndex = 0; axisIndex < Vector2::AXIS_COUNT; axisIndex++)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisIndex);
        
        // measure phase
        for (auto it = controls.rbegin(); it != controls.rend(); ++it)
        {
            MeasureControl(*it, axis);
        }

        // layout phase
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            UILinearLayoutComponent *linearLayoutComponent = it->control->GetComponent<UILinearLayoutComponent>();
            bool anchorOnlyIgnoredControls = false;
            if (linearLayoutComponent && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
            {
                ApplyLinearLayout(*it, linearLayoutComponent, axis);
                anchorOnlyIgnoredControls = true;
            }
            
            ApplyAnchorLayout(*it, axis, anchorOnlyIgnoredControls);
        }
    }
    
    for (ControlDescr &descr : controls)
    {
        UIControl *control = descr.control;
        
        if (descr.flags & FLAG_POSITION_CHANGED)
        {
            control->SetPosition(descr.position + control->GetPivotPoint());
        }
        
        if (descr.flags & FLAG_SIZE_CHANGED)
        {
            control->SetSize(descr.size);
            control->SetPropertyLocalFlag(indexOfSizeProperty, true);
            control->OnSizeChanged();
        }
        
        control->ResetLayoutDirty();
    }
    
    uint64 endTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::Debug("Changed controls: %d, time: %llu ms", controls.size(), endTime - startTime);

    controls.clear();
    
}

void UILayoutSystem::CollectControls(UIControl *control)
{
    controls.clear();
    
    ControlDescr descr;
    descr.control = control;
    descr.position = control->GetPosition() - control->GetPivotPoint();
    descr.size = control->GetSize();
    controls.emplace_back(descr);
    
    CollectControlChildren(control, 0);
}
    
void UILayoutSystem::CollectControlChildren(UIControl *control, int32 parentIndex)
{
    int32 index = static_cast<int32>(controls.size());
    const List<UIControl*> &children = control->GetChildren();
    controls[parentIndex].firstChild = index;
    controls[parentIndex].lastChild = index + static_cast<int32>(children.size() - 1);
    for (UIControl *child : children)
    {
        ControlDescr descr;
        descr.control = child;
        descr.flags = 0;
        descr.position = child->GetPosition() - child->GetPivotPoint();
        descr.size = child->GetSize();
        controls.emplace_back(descr);
    }

    for (UIControl *child : children)
    {
        CollectControlChildren(child, index);
        index++;
    }
}

UIControl *UILayoutSystem::FindControl(UIControl *control) const
{
    UIControl *parent = control->GetParent();
    if (parent == nullptr)
        return control;
    
    UISizePolicyComponent *sizePolicy = parent->GetComponent<UISizePolicyComponent>();
    if (sizePolicy)
    {
        if (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y))
            return FindControl(parent);
    }
    
    return control;
}

////////////////////////////////////////////////////////////////////////////////
// Measuring
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::MeasureControl(ControlDescr &descr, Vector2::eAxis axis)
{
    UIControl *control = descr.control;
    UISizePolicyComponent *sizeHint = descr.control->GetComponent<UISizePolicyComponent>();
    if (sizeHint == nullptr)
        return;
    
    bool skipInvisible = false;
    
    UILinearLayoutComponent *linearLayout = nullptr;
    
    linearLayout = control->GetComponent<UILinearLayoutComponent>();
    if (linearLayout)
        skipInvisible = linearLayout->IsSkipInvisibleControls();
    
    UISizePolicyComponent::eSizePolicy policy = sizeHint->GetPolicyByAxis(axis);
    float32 hintValue = sizeHint->GetValueByAxis(axis);
    float32 value = 0;
    int32 processedChildrenCount = 0;
    
    switch (policy)
    {
        case UISizePolicyComponent::IGNORE_SIZE:
            value = descr.size.data[axis]; // ignore
            break;
            
        case UISizePolicyComponent::FIXED_SIZE:
            value = hintValue;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM:
            for (int32 i = descr.firstChild; i <= descr.lastChild; i++)
            {
                if (!HaveToSkipControl(controls[i].control, skipInvisible))
                {
                    processedChildrenCount++;
                    value += controls[i].size.data[axis];
                }
                
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_MAX_CHILD:
            for (int32 i = descr.firstChild; i <= descr.lastChild; i++)
            {
                if (!HaveToSkipControl(controls[i].control, skipInvisible))
                {
                    processedChildrenCount = 1;
                    value = Max(value, controls[i].size.data[axis]);
                }
            }
            value = value * hintValue / 100.0f;
            break;

        case UISizePolicyComponent::PERCENT_OF_FIRST_CHILD:
            if (descr.HasChildren())
            {
                value = controls[descr.firstChild].size.data[axis];
                processedChildrenCount = 1;
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_LAST_CHILD:
            if (descr.HasChildren())
            {
                value = controls[descr.lastChild].size.data[axis];
                processedChildrenCount = 1;
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CONTENT:
        {
            Vector2 constraints(-1.0f, -1.0f);
            if (control->IsHeightDependsOnWidth() && axis == Vector2::AXIS_Y)
            {
                constraints.x = descr.size.x;
            }
            value = control->GetContentPreferredSize(constraints).data[axis] * hintValue / 100.0f;
            break;
        }
            
        case UISizePolicyComponent::PERCENT_OF_PARENT:
            value = descr.size.data[axis]; // ignore
            break;
            
        default:
            DVASSERT(false);
            break;
            
    }
    
    if (sizeHint->IsDependsOnChildren(axis) && linearLayout && linearLayout->GetAxis() == axis)
    {
        if (policy == UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM && processedChildrenCount > 0)
            value += linearLayout->GetSpacing() * (processedChildrenCount - 1);
        
        value += linearLayout->GetPadding() * 2.0f;
    }
    
    if (policy != UISizePolicyComponent::PERCENT_OF_PARENT && policy != UISizePolicyComponent::IGNORE_SIZE)
    {
        value = Clamp(value, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));
    }
    
    descr.size.data[axis] = value;
    descr.flags |= FLAG_SIZE_CHANGED;
}

////////////////////////////////////////////////////////////////////////////////
// Linear Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyLinearLayout(ControlDescr &descr, UILinearLayoutComponent *layout, Vector2::eAxis axis)
{
    float32 fixedSize = 0.0f;
    float32 totalPercent = 0.0f;
    
    if (!descr.HasChildren())
        return;
    
    bool inverse = isRtl && layout->IsUseRtl() && layout->GetOrientation() == UILinearLayoutComponent::HORIZONTAL;
    const bool skipInvisible = layout->IsSkipInvisibleControls();

    int32 childrenCount = 0;
    for (int32 i = descr.firstChild; i <= descr.lastChild; i++)
    {
        ControlDescr &child = controls[i];
        if (HaveToSkipControl(child.control, skipInvisible))
            continue;
        
        childrenCount++;
        
        const UISizePolicyComponent *sizeHint = child.control->GetComponent<UISizePolicyComponent>();
        if (sizeHint)
        {
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
                totalPercent += sizeHint->GetValueByAxis(axis);
            else
                fixedSize += child.size.data[axis];
        }
        else
        {
            fixedSize += child.size.data[axis];
        }
    }
    
    if (childrenCount > 0)
    {
        float32 padding = layout->GetPadding();
        float32 spacing = layout->GetSpacing();
        
        int32 spacesCount = childrenCount - 1;
        float32 contentSize = descr.size.data[axis] - padding * 2.0f;
        float32 restSize = contentSize - fixedSize - spacesCount * spacing;
        
        bool haveToCalculateSizes = true;
        
        int32 flagSizeCalculated = axis == Vector2::AXIS_X ? FLAG_X_SIZE_CALCULATED : FLAG_Y_SIZE_CALCULATED;
        while (haveToCalculateSizes)
        {
            haveToCalculateSizes = false;
            for (int32 i = descr.firstChild; i <= descr.lastChild; i++)
            {
                ControlDescr &child = controls[i];
                
                if (child.flags & flagSizeCalculated)
                    continue;
                
                if (HaveToSkipControl(child.control, skipInvisible))
                {
                    child.flags |= flagSizeCalculated;
                    continue;
                }
                
                const UISizePolicyComponent *sizeHint = child.control->GetComponent<UISizePolicyComponent>();
                if (sizeHint && sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
                {
                    float32 percents = sizeHint->GetValueByAxis(axis);
                    float32 size = 0.0f;
                    if (totalPercent > EPSILON)
                        size = restSize * percents / Max(totalPercent, 100.0f);
                    
                    float32 minSize = sizeHint->GetMinValueByAxis(axis);
                    float32 maxSize = sizeHint->GetMaxValueByAxis(axis);
                    if (size < minSize || maxSize < size)
                    {
                        size = Clamp(size, minSize, maxSize);
                        child.size.data[axis] = size;
                        child.flags |= FLAG_SIZE_CHANGED | flagSizeCalculated;
                        
                        restSize -= size;
                        totalPercent -= percents;
                        
                        haveToCalculateSizes = true;
                        break;
                    }
                    
                    child.size.data[axis] = size;
                    child.flags |= FLAG_SIZE_CHANGED;
                }
            }
        }
        
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
            position = descr.size.data[axis] - padding;

        for (int32 i = descr.firstChild; i <= descr.lastChild; i++)
        {
            ControlDescr &child = controls[i];
            if (HaveToSkipControl(child.control, skipInvisible))
                continue;

            float32 size = child.size.data[axis];
            child.position.data[axis] = inverse ? position - size : position;
            child.flags |= FLAG_POSITION_CHANGED;
            
            if (inverse)
                position -= size + spacing;
            else
                position += size + spacing;
            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Anchor Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyAnchorLayout(ControlDescr &descr, Vector2::eAxis axis, bool onlyForIgnoredControls)
{
    
    for (int32 i = descr.firstChild; i <= descr.lastChild; i++)
    {
        ControlDescr &child = controls[i];
        if (onlyForIgnoredControls && child.control->GetComponentCount(UIComponent::IGNORE_LAYOUT_COMPONENT) == 0)
            continue;
        
        const UISizePolicyComponent *sizeHint = child.control->GetComponent<UISizePolicyComponent>();
        if (sizeHint)
        {
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                float32 size = descr.size.data[axis] * sizeHint->GetValueByAxis(axis) / 100.0f;
                size = Clamp(size, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));

                child.size.data[axis] = size;
                child.flags |= FLAG_SIZE_CHANGED;
            }
        }
        
        UIAnchorComponent *hint = child.control->GetComponent<UIAnchorComponent>();
        if (hint)
        {
            float v1 = 0.0f;
            bool v1Enabled = false;

            float v2 = 0.0f;
            bool v2Enabled = false;

            float v3 = 0.0f;
            bool v3Enabled = false;
            
            switch (axis)
            {
                case Vector2::AXIS_X:
                    v1Enabled = hint->IsLeftAnchorEnabled();
                    v1 = hint->GetLeftAnchor();
                    
                    v2Enabled = hint->IsHCenterAnchorEnabled();
                    v2 = hint->GetHCenterAnchor();
                    
                    v3Enabled = hint->IsRightAnchorEnabled();
                    v3 = hint->GetRightAnchor();

                    if (isRtl && hint->IsUseRtl())
                    {
                        v1Enabled = hint->IsRightAnchorEnabled();
                        v1 = hint->GetRightAnchor();
                        
                        v3Enabled = hint->IsLeftAnchorEnabled();
                        v3 = hint->GetLeftAnchor();
                        
                        v2 = -v2;
                    }
                    break;
                    
                case Vector2::AXIS_Y:
                    v1Enabled = hint->IsTopAnchorEnabled();
                    v1 = hint->GetTopAnchor();
                    
                    v2Enabled = hint->IsVCenterAnchorEnabled();
                    v2 = hint->GetVCenterAnchor();
                    
                    v3Enabled = hint->IsBottomAnchorEnabled();
                    v3 = hint->GetBottomAnchor();

                    break;
                    
                default:
                    DVASSERT(false);
                    break;
            }
            
            if (v1Enabled || v2Enabled || v3Enabled)
            {
                float32 parentSize = descr.size.data[axis];
                
                if (v1Enabled && v3Enabled) // left and right
                {
                    child.position.data[axis] = v1;
                    child.size.data[axis] = parentSize - (v1 + v3);
                    child.flags |= FLAG_POSITION_CHANGED | FLAG_SIZE_CHANGED;
                }
                else if (v1Enabled && v2Enabled) // left and center
                {
                    child.position.data[axis] = v1;
                    child.size.data[axis] = parentSize / 2.0f - (v1 - v2);
                    child.flags |= FLAG_POSITION_CHANGED | FLAG_SIZE_CHANGED;
                }
                else if (v2Enabled && v3Enabled) // center and right
                {
                    child.position.data[axis] = parentSize / 2.0f + v2;
                    child.size.data[axis] = parentSize / 2.0f - (v2 + v3);
                    child.flags |= FLAG_POSITION_CHANGED | FLAG_SIZE_CHANGED;
                }
                else if (v1Enabled) // left
                {
                    child.position.data[axis] = v1;
                    child.flags |= FLAG_POSITION_CHANGED;
                }
                else if (v2Enabled) // center
                {
                    child.position.data[axis] = (parentSize - child.size.data[axis]) / 2.0f + v2;
                    child.flags |= FLAG_POSITION_CHANGED;
                }
                else if (v3Enabled) // right
                {
                    child.position.data[axis] = parentSize - (child.size.data[axis] + v3);
                    child.flags |= FLAG_POSITION_CHANGED;
                }

            }
        }
    }
}

bool UILayoutSystem::HaveToSkipControl(UIControl *control, bool skipInvisible) const
{
    return (skipInvisible && !control->GetVisible()) || control->GetComponentCount(UIComponent::IGNORE_LAYOUT_COMPONENT) > 0;
}

}
