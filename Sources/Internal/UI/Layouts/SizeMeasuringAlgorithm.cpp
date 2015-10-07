/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "SizeMeasuringAlgorithm.h"

#include "UILinearLayoutComponent.h"
#include "UIFlowLayoutComponent.h"
#include "UIFlowLayoutHintComponent.h"
#include "UISizePolicyComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{

SizeMeasuringAlgorithm::SizeMeasuringAlgorithm(Vector<ControlLayoutData> &layoutData_)
    : layoutData(layoutData_)
{
    
}

SizeMeasuringAlgorithm::~SizeMeasuringAlgorithm()
{
    
}

void SizeMeasuringAlgorithm::Apply(ControlLayoutData &data, Vector2::eAxis axis)
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
    
    switch (sizePolicy->GetPolicyByAxis(axis))
    {
        case UISizePolicyComponent::IGNORE_SIZE:
            ProcessIgnoreSizePolicy(data, axis);
            break;
            
        case UISizePolicyComponent::FIXED_SIZE:
            ProcessFixedSizePolicy(data, axis);
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CHILDREN_SUM:
            ProcessPercentOfChildrenSumPolicy(data, axis);
            break;
            
        case UISizePolicyComponent::PERCENT_OF_MAX_CHILD:
            ProcessPercentOfMaxChildPolicy(data, axis);
            break;
            
        case UISizePolicyComponent::PERCENT_OF_FIRST_CHILD:
            ProcessPercentOfFirstChildPolicy(data, axis);
            break;
            
        case UISizePolicyComponent::PERCENT_OF_LAST_CHILD:
            ProcessPercentOfLastChildPolicy(data, axis);
            break;
            
        case UISizePolicyComponent::PERCENT_OF_CONTENT:
            ProcessPercentOfContentPolicy(data, axis);
            break;
            
        case UISizePolicyComponent::PERCENT_OF_PARENT:
            ProcessPercentOfParentPolicy(data, axis);
            break;
            
        default:
            DVASSERT(false);
            break;
            
    }
}

void SizeMeasuringAlgorithm::ProcessIgnoreSizePolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    // do nothing
}

void SizeMeasuringAlgorithm::ProcessFixedSizePolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    float32 size = ClampValue(sizePolicy->GetValueByAxis(axis), axis);
    data.SetSize(axis, size);
}

void SizeMeasuringAlgorithm::ProcessPercentOfChildrenSumPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    if (flowLayout && flowLayout->IsEnabled())
    {
        switch (axis)
        {
        case Vector2::AXIS_X:
            ProcessHorizontalFlowLayoutPercentOfChildrenSumPolicy(data);
            break;
        case Vector2::AXIS_Y:
            ProcessVerticalFlowLayoutPercentOfChildrenSumPolicy(data);
            break;

        default:
            DVASSERT(false);
            break;
        }
    }
    else
    {
        ProcessDefaultPercentOfChildrenSumPolicy(data, axis);
    }
}
    
void SizeMeasuringAlgorithm::ProcessDefaultPercentOfChildrenSumPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    float32 value = 0;
    int32 processedChildrenCount = 0;

    bool newLineBeforeNext = false;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData &childData = layoutData[i];
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
        value += linearLayout->GetSpacing() * processedChildrenCount - 1;
    }
    
    ApplySize(data, value, axis);
}

void SizeMeasuringAlgorithm::ProcessHorizontalFlowLayoutPercentOfChildrenSumPolicy(ControlLayoutData& data)
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
    ApplySize(data, maxWidth, Vector2::AXIS_X);
}

void SizeMeasuringAlgorithm::ProcessVerticalFlowLayoutPercentOfChildrenSumPolicy(ControlLayoutData& data)
{
    DVASSERT(flowLayout && flowLayout->IsEnabled());

    float32 value = 0;

    int32 linesCount = 0;
    float32 lineHeight = 0;
    
    for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
    {
        ControlLayoutData &childData = layoutData[index];
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

    ApplySize(data, value, Vector2::AXIS_Y);
}

void SizeMeasuringAlgorithm::ProcessPercentOfMaxChildPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    float32 value = 0.0f;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData &childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            value = Max(value, GetSize(childData, axis));
        }
    }
    
    ApplySize(data, value, axis);
}

void SizeMeasuringAlgorithm::ProcessPercentOfFirstChildPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    float32 value = 0.0f;
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        const ControlLayoutData &childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            value = GetSize(childData, axis);
            break;
        }
    }
    
    ApplySize(data, value, axis);
}

void SizeMeasuringAlgorithm::ProcessPercentOfLastChildPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    float32 value = 0.0f;
    for (int32 i = data.GetLastChildIndex(); i >= data.GetFirstChildIndex(); i--)
    {
        const ControlLayoutData &childData = layoutData[i];
        if (!childData.HaveToSkipControl(skipInvisible))
        {
            value = GetSize(childData, axis);
            break;
        }
    }
    
    ApplySize(data, value, axis);
}

void SizeMeasuringAlgorithm::ProcessPercentOfContentPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    Vector2 constraints(-1.0f, -1.0f);
    if (data.GetControl()->IsHeightDependsOnWidth() && axis == Vector2::AXIS_Y)
    {
        constraints.x = data.GetSize(Vector2::AXIS_X);
    }
    
    float32 value = data.GetControl()->GetContentPreferredSize(constraints).data[axis];
    ApplySize(data, value, axis);
}

void SizeMeasuringAlgorithm::ProcessPercentOfParentPolicy(ControlLayoutData &data, Vector2::eAxis axis)
{
    // do nothing
}

void SizeMeasuringAlgorithm::ApplySize(ControlLayoutData &data, float32 value, Vector2::eAxis axis)
{
    float32 valueWithPadding = value + GetLayoutPadding(axis);
    float32 percentedValue = valueWithPadding * sizePolicy->GetValueByAxis(axis) / 100.0f;;
    float32 clampedValue = ClampValue(percentedValue, axis);
    data.SetSize(axis, clampedValue);
}
    
float32 SizeMeasuringAlgorithm::GetSize(const ControlLayoutData &data, Vector2::eAxis axis)
{
    UISizePolicyComponent *sizePolicy = data.GetControl()->GetComponent<UISizePolicyComponent>();
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
