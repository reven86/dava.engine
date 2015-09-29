#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "Platform/SystemTimer.h"

namespace DAVA
{
    
class UILayoutSystem::ControlLayoutData
{
public:
    enum eFlag
    {
        FLAG_NONE = 0,
        FLAG_SIZE_CHANGED = 1 << 0,
        FLAG_POSITION_CHANGED = 1 << 1,
        FLAG_X_SIZE_CALCULATED = 1 << 2,
        FLAG_Y_SIZE_CALCULATED = 1 << 3
    };
    
public:
    ControlLayoutData(UIControl *control_) : control(control_)
    {
        position = control->GetPosition() - control->GetPivotPoint();
        size = control->GetSize();
    }
    
    void ApplyLayoutToControl(int32 indexOfSizeProperty)
    {
        if (HasFlag(FLAG_POSITION_CHANGED))
        {
            control->SetPosition(position + control->GetPivotPoint());
        }
        
        if (HasFlag(FLAG_SIZE_CHANGED))
        {
            control->SetSize(size);
            control->SetPropertyLocalFlag(indexOfSizeProperty, true);
            control->OnSizeChanged();
        }
        
        control->ResetLayoutDirty();
    }

    UIControl *GetControl() const
    {
        return control;
    }
    
    template<class T> T* GetComponent() const
    {
        return control->GetComponent<T>(0);
    }
    
    bool HasComponent(UIComponent::eType type) const
    {
        return control->GetComponentCount(type) > 0;
    }
    
    bool HasFlag(eFlag flag) const
    {
        return (flags & flag) == flag;
    }
    
    void SetFlag(eFlag flag)
    {
        flags |= flag;
    }
    
    int32 GetFirstChildIndex() const
    {
        return firstChild;
    }
    
    void SetFirstChildIndex(int32 index)
    {
        firstChild = index;
    }
    
    int32 GetLastChildIndex() const
    {
        return lastChild;
    }
    
    void SetLastChildIndex(int32 index)
    {
        lastChild = index;
    }
    
    bool HasChildren() const
    {
        return lastChild >= firstChild;
    }
    
    float32 GetSize(Vector2::eAxis axis) const
    {
        return size.data[axis];
    }
    
    void SetSize(Vector2::eAxis axis, float32 value)
    {
        size.data[axis] = value;
        flags |= FLAG_SIZE_CHANGED;
    }
    
    float32 GetPosition(Vector2::eAxis axis) const
    {
        return position.data[axis];
    }
    
    void SetPosition(Vector2::eAxis axis, float32 value)
    {
        position.data[axis] = value;
        flags |= FLAG_POSITION_CHANGED;
    }
    
private:
    UIControl *control;
    int32 flags = FLAG_NONE;
    int32 firstChild = 0;
    int32 lastChild = -1;
    Vector2 size;
    Vector2 position;
};

UILayoutSystem::UILayoutSystem()
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

void UILayoutSystem::ApplyLayout(UIControl *inputContainer, bool considerDenendenceOnChildren)
{
    UIControl *container = inputContainer;
    if (considerDenendenceOnChildren)
    {
        while (container->GetParent() != nullptr)
        {
            UISizePolicyComponent *sizePolicy = container->GetParent()->GetComponent<UISizePolicyComponent>();
            if (sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y)))
            {
                container = container->GetParent();
            }
            else
            {
                break;
            }
        }
    }

    CollectControls(container);
    
    for (int32 axisIndex = 0; axisIndex < Vector2::AXIS_COUNT; axisIndex++)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisIndex);
        
        // measure phase
        for (auto it = layoutData.rbegin(); it != layoutData.rend(); ++it)
        {
            MeasureControl(*it, axis);
        }

        // layout phase
        for (auto it = layoutData.begin(); it != layoutData.end(); ++it)
        {
            UILinearLayoutComponent *linearLayoutComponent = it->GetComponent<UILinearLayoutComponent>();
            bool anchorOnlyIgnoredControls = false;
            if (linearLayoutComponent != nullptr && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
            {
                ApplyLinearLayout(*it, linearLayoutComponent, axis);
                anchorOnlyIgnoredControls = true;
            }
            
            ApplyAnchorLayout(*it, axis, anchorOnlyIgnoredControls);
        }
    }
    
    for (ControlLayoutData &data : layoutData)
    {
        data.ApplyLayoutToControl(indexOfSizeProperty);
    }

    layoutData.clear();
}

void UILayoutSystem::CollectControls(UIControl *control)
{
    layoutData.clear();
    layoutData.emplace_back(ControlLayoutData(control));
    CollectControlChildren(control, 0);
}
    
void UILayoutSystem::CollectControlChildren(UIControl *control, int32 parentIndex)
{
    int32 index = static_cast<int32>(layoutData.size());
    const List<UIControl*> &children = control->GetChildren();
    
    layoutData[parentIndex].SetFirstChildIndex(index);
    layoutData[parentIndex].SetLastChildIndex(index + static_cast<int32>(children.size() - 1));

    for (UIControl *child : children)
    {
        layoutData.emplace_back(ControlLayoutData(child));
    }

    for (UIControl *child : children)
    {
        CollectControlChildren(child, index);
        index++;
    }
}

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
}

////////////////////////////////////////////////////////////////////////////////
// Measuring
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::MeasureControl(ControlLayoutData &data, Vector2::eAxis axis)
{
    UISizePolicyComponent *sizeHint = data.GetComponent<UISizePolicyComponent>();
    if (sizeHint == nullptr)
    {
        return;
    }
    
    bool skipInvisible = false;
    
    UILinearLayoutComponent *linearLayout = nullptr;
    
    linearLayout = data.GetComponent<UILinearLayoutComponent>();
    if (linearLayout != nullptr)
    {
        skipInvisible = linearLayout->IsSkipInvisibleControls();
    }
    
    UISizePolicyComponent::eSizePolicy policy = sizeHint->GetPolicyByAxis(axis);
    float32 hintValue = sizeHint->GetValueByAxis(axis);
    float32 value = 0;
    int32 processedChildrenCount = 0;
    
    switch (policy)
    {
        case UISizePolicyComponent::IGNORE_SIZE:
            value = data.GetSize(axis); // ignore
            break;
            
        case UISizePolicyComponent::FIXED_SIZE:
            value = hintValue;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM:
            for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
            {
                const ControlLayoutData &childData = layoutData[i];
                if (!HaveToSkipControl(childData.GetControl(), skipInvisible))
                {
                    processedChildrenCount++;
                    value += childData.GetSize(axis);
                }
                
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_MAX_CHILD:
            for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
            {
                const ControlLayoutData &childData = layoutData[i];
                if (!HaveToSkipControl(childData.GetControl(), skipInvisible))
                {
                    processedChildrenCount = 1;
                    value = Max(value, childData.GetSize(axis));
                }
            }
            value = value * hintValue / 100.0f;
            break;

        case UISizePolicyComponent::PERCENT_OF_FIRST_CHILD:
            if (data.HasChildren())
            {
                value = layoutData[data.GetFirstChildIndex()].GetSize(axis);
                processedChildrenCount = 1;
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_LAST_CHILD:
            if (data.HasChildren())
            {
                value = layoutData[data.GetLastChildIndex()].GetSize(axis);
                processedChildrenCount = 1;
            }
            value = value * hintValue / 100.0f;
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CONTENT:
        {
            Vector2 constraints(-1.0f, -1.0f);
            if (data.GetControl()->IsHeightDependsOnWidth() && axis == Vector2::AXIS_Y)
            {
                constraints.x = data.GetSize(Vector2::AXIS_X);
            }
            value = data.GetControl()->GetContentPreferredSize(constraints).data[axis] * hintValue / 100.0f;
            break;
        }
            
        case UISizePolicyComponent::PERCENT_OF_PARENT:
            value = data.GetSize(axis); // ignore
            break;
            
        default:
            DVASSERT(false);
            break;
            
    }
    
    if (sizeHint->IsDependsOnChildren(axis) && linearLayout != nullptr && linearLayout->GetAxis() == axis)
    {
        if (policy == UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM && processedChildrenCount > 0)
        {
            value += linearLayout->GetSpacing() * (processedChildrenCount - 1);
        }
        
        value += linearLayout->GetPadding() * 2.0f;
    }
    
    if (policy != UISizePolicyComponent::PERCENT_OF_PARENT && policy != UISizePolicyComponent::IGNORE_SIZE)
    {
        value = Clamp(value, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));
    }
    
    data.SetSize(axis, value);
}

////////////////////////////////////////////////////////////////////////////////
// Linear Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyLinearLayout(ControlLayoutData &data, UILinearLayoutComponent *layout, Vector2::eAxis axis)
{
    float32 fixedSize = 0.0f;
    float32 totalPercent = 0.0f;
    
    if (!data.HasChildren())
    {
        return;
    }
    
    bool inverse = isRtl && layout->IsUseRtl() && layout->GetOrientation() == UILinearLayoutComponent::HORIZONTAL;
    const bool skipInvisible = layout->IsSkipInvisibleControls();

    int32 childrenCount = 0;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        ControlLayoutData &childData = layoutData[i];
        if (HaveToSkipControl(childData.GetControl(), skipInvisible))
        {
            continue;
        }
        
        childrenCount++;
        
        const UISizePolicyComponent *sizeHint = childData.GetComponent<UISizePolicyComponent>();
        if (sizeHint != nullptr)
        {
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                totalPercent += sizeHint->GetValueByAxis(axis);
            }
            else
            {
                fixedSize += childData.GetSize(axis);
            }
        }
        else
        {
            fixedSize += childData.GetSize(axis);
        }
    }
    
    if (childrenCount > 0)
    {
        float32 padding = layout->GetPadding();
        float32 spacing = layout->GetSpacing();
        
        int32 spacesCount = childrenCount - 1;
        float32 contentSize = data.GetSize(axis) - padding * 2.0f;
        float32 restSize = contentSize - fixedSize - spacesCount * spacing;
        
        bool haveToCalculateSizes = true;
        
        ControlLayoutData::eFlag flagSizeCalculated = axis == Vector2::AXIS_X ? ControlLayoutData::FLAG_X_SIZE_CALCULATED : ControlLayoutData::FLAG_Y_SIZE_CALCULATED;
        while (haveToCalculateSizes)
        {
            haveToCalculateSizes = false;
            for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
            {
                ControlLayoutData &childData = layoutData[i];
                
                if (childData.HasFlag(flagSizeCalculated))
                {
                    continue;
                }
                
                if (HaveToSkipControl(childData.GetControl(), skipInvisible))
                {
                    childData.SetFlag(flagSizeCalculated);
                    continue;
                }
                
                const UISizePolicyComponent *sizeHint = childData.GetComponent<UISizePolicyComponent>();
                if (sizeHint != nullptr && sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
                {
                    float32 percents = sizeHint->GetValueByAxis(axis);
                    float32 size = 0.0f;
                    if (totalPercent > EPSILON)
                    {
                        size = restSize * percents / Max(totalPercent, 100.0f);
                    }
                    
                    float32 minSize = sizeHint->GetMinValueByAxis(axis);
                    float32 maxSize = sizeHint->GetMaxValueByAxis(axis);
                    if (size < minSize || maxSize < size)
                    {
                        size = Clamp(size, minSize, maxSize);
                        childData.SetSize(axis, size);
                        childData.SetFlag(flagSizeCalculated);
                        
                        restSize -= size;
                        totalPercent -= percents;
                        
                        haveToCalculateSizes = true;
                        break;
                    }
                    
                    childData.SetSize(axis, size);
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
                {
                    cnt = 2;
                }

                if (dynamicSpacing)
                {
                    cnt += spacesCount;
                }
                
                float32 delta = restSize * (1.0f - totalPercent / 100.0f) / cnt;
                if (dynamicPadding)
                {
                    padding += delta;
                }
                
                if (dynamicSpacing)
                {
                    spacing += delta;
                }
            }
        }
        
        float32 position = padding;
        if (inverse)
        {
            position = data.GetSize(axis) - padding;
        }

        for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
        {
            ControlLayoutData &childData = layoutData[i];
            if (HaveToSkipControl(childData.GetControl(), skipInvisible))
            {
                continue;
            }

            float32 size = childData.GetSize(axis);
            childData.SetPosition(axis, inverse ? position - size : position);
            
            if (inverse)
            {
                position -= size + spacing;
            }
            else
            {
                position += size + spacing;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Anchor Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyAnchorLayout(ControlLayoutData &data, Vector2::eAxis axis, bool onlyForIgnoredControls)
{
    
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        ControlLayoutData &childData = layoutData[i];
        if (onlyForIgnoredControls && childData.HasComponent(UIComponent::IGNORE_LAYOUT_COMPONENT) == 0)
        {
            continue;
        }
        
        const UISizePolicyComponent *sizeHint = childData.GetComponent<UISizePolicyComponent>();
        if (sizeHint != nullptr)
        {
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                float32 size = data.GetSize(axis) * sizeHint->GetValueByAxis(axis) / 100.0f;
                size = Clamp(size, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));

                childData.SetSize(axis, size);
            }
        }
        
        UIAnchorComponent *hint = childData.GetComponent<UIAnchorComponent>();
        if (hint != nullptr)
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
                float32 parentSize = data.GetSize(axis);
                
                if (v1Enabled && v3Enabled) // left and right
                {
                    childData.SetPosition(axis, v1);
                    childData.SetSize(axis, parentSize - (v1 + v3));
                }
                else if (v1Enabled && v2Enabled) // left and center
                {
                    childData.SetPosition(axis, v1);
                    childData.SetSize(axis, parentSize / 2.0f - (v1 - v2));
                }
                else if (v2Enabled && v3Enabled) // center and right
                {
                    childData.SetPosition(axis, parentSize / 2.0f + v2);
                    childData.SetSize(axis, parentSize / 2.0f - (v2 + v3));
                }
                else if (v1Enabled) // left
                {
                    childData.SetPosition(axis, v1);
                }
                else if (v2Enabled) // center
                {
                    childData.SetPosition(axis, (parentSize - childData.GetSize(axis)) / 2.0f + v2);
                }
                else if (v3Enabled) // right
                {
                    childData.SetPosition(axis, parentSize - (childData.GetSize(axis) + v3));
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
