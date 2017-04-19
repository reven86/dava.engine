#include "SizeMeasuringAlgorithm.h"

#include "UILinearLayoutComponent.h"
#include "UIFlowLayoutComponent.h"
#include "UIFlowLayoutHintComponent.h"
#include "UISizePolicyComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
SizeMeasuringAlgorithm::SizeMeasuringAlgorithm(Vector<ControlLayoutData>& layoutData_)
    : layoutData(layoutData_)
{
}

SizeMeasuringAlgorithm::~SizeMeasuringAlgorithm()
{
}

void SizeMeasuringAlgorithm::Apply(ControlLayoutData& data, Vector2::eAxis axis)
{
    linearLayout = nullptr;
    flowLayout = data.GetControl()->GetComponent<UIFlowLayoutComponent>();
    sizePolicy = data.GetControl()->GetComponent<UISizePolicyComponent>();

    if (sizePolicy == nullptr)
    {
        return;
    }

    skipInvisible = false;

    if (flowLayout && flowLayout->IsEnabled())
    {
        skipInvisible = flowLayout->IsSkipInvisibleControls();
    }
    else
    {
        linearLayout = data.GetControl()->GetComponent<UILinearLayoutComponent>();
        if (linearLayout != nullptr && linearLayout->IsEnabled())
        {
            skipInvisible = linearLayout->IsSkipInvisibleControls();
        }
    }

    float32 value = 0.0f;
    UISizePolicyComponent::eSizePolicy policy = sizePolicy->GetPolicyByAxis(axis);
    switch (policy)
    {
    case UISizePolicyComponent::IGNORE_SIZE:
    case UISizePolicyComponent::PERCENT_OF_PARENT:
        // do nothing
        break;

    case UISizePolicyComponent::FIXED_SIZE:
        value = CalculateFixedSize(data, axis);
        break;

    case UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM:
        value = CalculatePercentOfChildrenSum(data, axis);
        break;

    case UISizePolicyComponent::PERCENT_OF_MAX_CHILD:
        value = CalculatePercentOfMaxChild(data, axis);
        break;

    case UISizePolicyComponent::PERCENT_OF_FIRST_CHILD:
        value = CalculatePercentOfFirstChild(data, axis);
        break;

    case UISizePolicyComponent::PERCENT_OF_LAST_CHILD:
        value = CalculatePercentOfLastChild(data, axis);
        break;

    case UISizePolicyComponent::PERCENT_OF_CONTENT:
        value = CalculatePercentOfContent(data, axis);
        break;

    default:
        DVASSERT(false);
        break;
    }

    if (policy != UISizePolicyComponent::IGNORE_SIZE && policy != UISizePolicyComponent::PERCENT_OF_PARENT)
    {
        ApplySize(data, value, axis);
    }
}

float32 SizeMeasuringAlgorithm::CalculateFixedSize(ControlLayoutData& data, Vector2::eAxis axis)
{
    return ClampValue(sizePolicy->GetValueByAxis(axis), axis);
}

float32 SizeMeasuringAlgorithm::CalculatePercentOfChildrenSum(ControlLayoutData& data, Vector2::eAxis axis)
{
    if (flowLayout && flowLayout->IsEnabled())
    {
        switch (axis)
        {
        case Vector2::AXIS_X:
            return CalculateHorizontalFlowLayoutPercentOfChildrenSum(data);

        case Vector2::AXIS_Y:
            return CalculateVerticalFlowLayoutPercentOfChildrenSum(data);

        default:
            DVASSERT(false);
            return 0.0f;
        }
    }
    else
    {
        return CalculateDefaultPercentOfChildrenSum(data, axis);
    }
}

float32 SizeMeasuringAlgorithm::CalculateDefaultPercentOfChildrenSum(ControlLayoutData& data, Vector2::eAxis axis)
{
    float32 value = 0;
    int32 processedChildrenCount = 0;

    bool newLineBeforeNext = false;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData& childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            bool newLineBeforeThis = newLineBeforeNext;
            newLineBeforeNext = false;
            UIFlowLayoutHintComponent* hint = childData.GetControl()->GetComponent<UIFlowLayoutHintComponent>();
            if (hint != nullptr)
            {
                newLineBeforeThis |= hint->IsNewLineBeforeThis();
                newLineBeforeNext = hint->IsNewLineAfterThis();
            }

            processedChildrenCount++;
            value += GetSize(childData, axis);
        }
    }

    if (linearLayout && linearLayout->IsEnabled() && axis == linearLayout->GetAxis() && processedChildrenCount > 0)
    {
        value += linearLayout->GetSpacing() * (processedChildrenCount - 1);
    }

    return value;
}

float32 SizeMeasuringAlgorithm::CalculateHorizontalFlowLayoutPercentOfChildrenSum(ControlLayoutData& data)
{
    DVASSERT(flowLayout && flowLayout->IsEnabled());

    float32 lineWidth = 0.0f;
    float32 maxWidth = 0.0f;
    bool newLineBeforeNext = false;
    bool firstInLine = true;

    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData& childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            float32 childSize = childData.GetWidth();
            UISizePolicyComponent* sizePolicy = childData.GetControl()->GetComponent<UISizePolicyComponent>();
            if (sizePolicy != nullptr && sizePolicy->GetHorizontalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                childSize = sizePolicy->GetHorizontalMinValue();
            }

            bool newLineBeforeThis = newLineBeforeNext;
            newLineBeforeNext = false;
            UIFlowLayoutHintComponent* hint = childData.GetControl()->GetComponent<UIFlowLayoutHintComponent>();
            if (hint != nullptr)
            {
                newLineBeforeThis |= hint->IsNewLineBeforeThis();
                newLineBeforeNext = hint->IsNewLineAfterThis();
            }

            if (newLineBeforeThis)
            {
                maxWidth = Max(maxWidth, lineWidth);
                lineWidth = 0.0f;
                firstInLine = true;
            }

            lineWidth += childSize;
            if (!firstInLine)
            {
                lineWidth += flowLayout->GetHorizontalSpacing();
            }
            firstInLine = false;
        }
    }

    maxWidth = Max(maxWidth, lineWidth);
    return maxWidth;
}

float32 SizeMeasuringAlgorithm::CalculateVerticalFlowLayoutPercentOfChildrenSum(ControlLayoutData& data)
{
    DVASSERT(flowLayout && flowLayout->IsEnabled());

    float32 value = 0;

    int32 linesCount = 0;
    float32 lineHeight = 0;

    for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
    {
        ControlLayoutData& childData = layoutData[index];
        if (childData.HaveToSkipControl(skipInvisible))
            continue;

        lineHeight = Max(lineHeight, childData.GetHeight());

        if (childData.HasFlag(ControlLayoutData::FLAG_LAST_IN_LINE))
        {
            value += lineHeight;
            linesCount++;
            lineHeight = 0;
        }
    }
    if (linesCount > 0)
    {
        value += flowLayout->GetVerticalSpacing() * (linesCount - 1);
    }

    return value;
}

float32 SizeMeasuringAlgorithm::CalculatePercentOfMaxChild(ControlLayoutData& data, Vector2::eAxis axis)
{
    float32 value = 0.0f;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData& childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            value = Max(value, GetSize(childData, axis));
        }
    }

    return value;
}

float32 SizeMeasuringAlgorithm::CalculatePercentOfFirstChild(ControlLayoutData& data, Vector2::eAxis axis)
{
    float32 value = 0.0f;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData& childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            value = GetSize(childData, axis);
            break;
        }
    }

    return value;
}

float32 SizeMeasuringAlgorithm::CalculatePercentOfLastChild(ControlLayoutData& data, Vector2::eAxis axis)
{
    float32 value = 0.0f;
    for (int32 i = data.GetLastChildIndex(); i >= data.GetFirstChildIndex(); i--)
    {
        const ControlLayoutData& childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            value = GetSize(childData, axis);
            break;
        }
    }

    return value;
}

float32 SizeMeasuringAlgorithm::CalculatePercentOfContent(ControlLayoutData& data, Vector2::eAxis axis)
{
    Vector2 constraints(-1.0f, -1.0f);
    if (data.GetControl()->IsHeightDependsOnWidth() && axis == Vector2::AXIS_Y)
    {
        constraints.x = data.GetSize(Vector2::AXIS_X);
    }

    return data.GetControl()->GetContentPreferredSize(constraints).data[axis];
}

void SizeMeasuringAlgorithm::ApplySize(ControlLayoutData& data, float32 value, Vector2::eAxis axis)
{
    float32 valueWithPadding = value + GetLayoutPadding(axis);
    float32 percentedValue = valueWithPadding * sizePolicy->GetValueByAxis(axis) / 100.0f;

    float32 clampedValue = ClampValue(percentedValue, axis);
    data.SetSize(axis, clampedValue);
}

float32 SizeMeasuringAlgorithm::GetSize(const ControlLayoutData& data, Vector2::eAxis axis)
{
    UISizePolicyComponent* sizePolicy = data.GetControl()->GetComponent<UISizePolicyComponent>();
    if (sizePolicy && sizePolicy->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
    {
        return sizePolicy->GetMinValueByAxis(axis);
    }
    return data.GetSize(axis);
}

float32 SizeMeasuringAlgorithm::GetLayoutPadding(Vector2::eAxis axis)
{
    if (flowLayout && flowLayout->IsEnabled())
    {
        return flowLayout->GetPaddingByAxis(axis) * 2.0f;
    }
    else if (linearLayout != nullptr && linearLayout->IsEnabled() && linearLayout->GetAxis() == axis)
    {
        return linearLayout->GetPadding() * 2.0f;
    }
    return 0.0f;
}

float32 SizeMeasuringAlgorithm::ClampValue(float32 value, Vector2::eAxis axis)
{
    UISizePolicyComponent::eSizePolicy policy = sizePolicy->GetPolicyByAxis(axis);
    if (policy != UISizePolicyComponent::PERCENT_OF_PARENT && policy != UISizePolicyComponent::IGNORE_SIZE)
    {
        return Clamp(value, sizePolicy->GetMinValueByAxis(axis), sizePolicy->GetMaxValueByAxis(axis));
    }
    return value;
}
}
