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

#include "FlowLayoutAlgorithm.h"

#include "UIFlowLayoutComponent.h"
#include "UISizePolicyComponent.h"
#include "UIFlowLayoutHintComponent.h"

#include "LinearLayoutAlgorithm.h"
#include "AnchorLayoutAlgorithm.h"

#include "UI/UIControl.h"

namespace DAVA
{
FlowLayoutAlgorithm::FlowLayoutAlgorithm(Vector<ControlLayoutData> &layoutData_, bool isRtl_)
    : layoutData(layoutData_)
    , isRtl(isRtl_)
{
    
}

FlowLayoutAlgorithm::~FlowLayoutAlgorithm()
{
    
}

void FlowLayoutAlgorithm::Apply(ControlLayoutData &data, Vector2::eAxis axis)
{
    UIFlowLayoutComponent *layout = data.GetControl()->GetComponent<UIFlowLayoutComponent>();
    DVASSERT(layout != nullptr);
    
    inverse = layout->GetOrientation() == UIFlowLayoutComponent::ORIENTATION_RIGHT_TO_LEFT;
    if (isRtl && layout->IsUseRtl())
        inverse = !inverse;
    
    skipInvisible = layout->IsSkipInvisibleControls();

    horizontalPadding = layout->GetHorizontalPadding();
    horizontalSpacing = layout->GetHorizontalSpacing();
    dynamicHorizontalPadding = layout->IsDynamicHorizontalPadding();
    dynamicHorizontalSpacing = layout->IsDynamicHorizontalSpacing();
    
    verticalPadding = layout->GetVerticalPadding();
    verticalSpacing = layout->GetVerticalSpacing();
    dynamicVerticalPadding = layout->IsDynamicVerticalPadding();
    dynamicVerticalSpacing = layout->IsDynamicVerticalSpacing();

    if (data.HasChildren())
    {
        switch (axis)
        {
            case Vector2::AXIS_X:
                ProcessXAxis(data, layout);
                break;
                
            case Vector2::AXIS_Y:
                ProcessYAxis(data, layout);
                break;
                
            default:
                DVASSERT(false);
                break;
        }
    }
    
    AnchorLayoutAlgorithm anchorAlg(layoutData, axis);
    anchorAlg.Apply(data, axis, true, data.GetFirstChildIndex(), data.GetLastChildIndex());
}
    
void FlowLayoutAlgorithm::ProcessXAxis(ControlLayoutData &data, UIFlowLayoutComponent *component)
{
    float32 rowSize = data.GetWidth() - horizontalPadding * 2;
    float32 restSize = rowSize;
    
    int32 firstIndex = data.GetFirstChildIndex();

    bool newLineBeforeNext = false;
    for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
    {
        ControlLayoutData &childData = layoutData[index];
        if (childData.HaveToSkipControl(skipInvisible))
        {
            continue;
        }
        
        float32 childSize = childData.GetWidth();
        UISizePolicyComponent *sizePolicy = childData.GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr && sizePolicy->GetHorizontalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT)
        {
            childSize = sizePolicy->GetHorizontalMinValue();
        }
        
        bool newLineBeforeThis = newLineBeforeNext;
        newLineBeforeNext = false;
        UIFlowLayoutHintComponent *hint = childData.GetControl()->GetComponent<UIFlowLayoutHintComponent>();
        if (hint != nullptr)
        {
            newLineBeforeThis |= hint->IsNewLineBeforeThis();
            newLineBeforeNext = hint->IsNewLineAfterThis();
        }
        
        if (newLineBeforeThis && index > firstIndex)
        {
            LayoutLine(data, firstIndex, index - 1);
            firstIndex = index;
            restSize = rowSize;
        }
        
        if (restSize < childSize - EPSILON)
        {
            if (index > firstIndex)
            {
                LayoutLine(data, firstIndex, index - 1);
                firstIndex = index;
                restSize = rowSize - childSize - horizontalSpacing;
            }
            else
            {
                LayoutLine(data, firstIndex, index);
                firstIndex = index + 1;
                restSize = rowSize;
            }
        }
        else
        {
            restSize = restSize - childSize - horizontalSpacing;
        }
    }
    
    if (firstIndex <= data.GetLastChildIndex())
    {
        LayoutLine(data, firstIndex, data.GetLastChildIndex());
    }
}

void FlowLayoutAlgorithm::LayoutLine(ControlLayoutData &data, int32 firstIndex, int32 lastIndex)
{
    LinearLayoutAlgorithm alg(layoutData);

    alg.SetInverse(inverse);
    alg.SetSkipInvisible(skipInvisible);
    
    alg.SetPadding(horizontalPadding);
    alg.SetSpacing(horizontalSpacing);
    
    alg.SetDynamicPadding(dynamicHorizontalPadding);
    alg.SetDynamicSpacing(dynamicHorizontalSpacing);
    
    alg.Apply(data, Vector2::AXIS_X, firstIndex, lastIndex);
    
    layoutData[lastIndex].SetFlag(ControlLayoutData::FLAG_LAST_IN_LINE);
}

void FlowLayoutAlgorithm::ProcessYAxis(ControlLayoutData &data, UIFlowLayoutComponent *component)
{
    CalculateDynamicPaddingAndSpaces(data);

    float32 lineHeight = 0;
    float32 y = verticalPadding;
    float32 firstIndex = data.GetFirstChildIndex();
    for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
    {
        ControlLayoutData &childData = layoutData[index];
        if (childData.HaveToSkipControl(skipInvisible))
        {
            continue;
        }

        lineHeight = Max(lineHeight, childData.GetHeight());

        if (childData.HasFlag(ControlLayoutData::FLAG_LAST_IN_LINE))
        {
            LayoutLineVertically(data, firstIndex, index, y, y + lineHeight);
            y += lineHeight + verticalSpacing;
            lineHeight = 0;
            firstIndex = index + 1;
        }
    }

}
 
void FlowLayoutAlgorithm::CalculateDynamicPaddingAndSpaces(ControlLayoutData &data)
{
    int32 linesCount = 0;
    float32 contentSize = 0.0f;
    float32 lineHeight = 0.0f;
    if (dynamicVerticalPadding || dynamicVerticalSpacing)
    {
        for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
        {
            ControlLayoutData &childData = layoutData[index];
            if (childData.HaveToSkipControl(skipInvisible))
            {
                continue;
            }
            
            lineHeight = Max(lineHeight, childData.GetHeight());

            if (childData.HasFlag(ControlLayoutData::FLAG_LAST_IN_LINE))
            {
                linesCount++;
                contentSize += lineHeight;
                lineHeight = 0.0f;
            }
        }
    }
    
    float32 restSize = data.GetHeight() - contentSize;
    if (linesCount > 0)
    {
        int32 spacesCount = linesCount - 1;
        restSize -= verticalPadding * 2.0f;
        restSize -= verticalSpacing * spacesCount;
        if (restSize > EPSILON)
        {
            if (dynamicVerticalPadding || (dynamicVerticalSpacing && spacesCount > 0))
            {
                int32 cnt = 0;
                if (dynamicVerticalPadding)
                {
                    cnt = 2;
                }
                
                if (dynamicVerticalSpacing)
                {
                    cnt += spacesCount;
                }
                
                float32 delta = restSize / cnt;
                if (dynamicVerticalPadding)
                {
                    verticalPadding += delta;
                }
                
                if (dynamicVerticalSpacing)
                {
                    verticalSpacing += delta;
                }
            }
        }
    }
}

void FlowLayoutAlgorithm::LayoutLineVertically(ControlLayoutData &data, int32 firstIndex, int32 lastIndex, float32 top, float32 bottom)
{
    for (int32 index = firstIndex; index <= lastIndex; index++)
    {
        ControlLayoutData &childData = layoutData[index];
        if (childData.HaveToSkipControl(skipInvisible))
        {
            continue;
        }
        UISizePolicyComponent *sizePolicy = childData.GetControl()->GetComponent<UISizePolicyComponent>();
        float32 childSize = childData.GetHeight();
        if (sizePolicy)
        {
            if (sizePolicy->GetVerticalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                childSize = (bottom - top) * sizePolicy->GetVerticalValue() / 100.0f;
                childSize = Clamp(childSize, sizePolicy->GetVerticalMinValue(), sizePolicy->GetVerticalMaxValue());
                childData.SetSize(Vector2::AXIS_Y, childSize);
            }
        }
        
        childData.SetPosition(Vector2::AXIS_Y, top);
    }
}
    
}
