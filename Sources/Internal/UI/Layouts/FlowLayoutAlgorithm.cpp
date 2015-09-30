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
    int32 firstIndex = data.GetFirstChildIndex();

    bool newLineBeforeNext = false;
    int32 childrenInLine = 0;
    float32 usedSize = 0.0f;

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
            childSize = sizePolicy->GetHorizontalValue() * (data.GetWidth() - horizontalPadding * 2.0f) / 100.0f;
            childSize = Clamp(childSize, sizePolicy->GetHorizontalMinValue(), sizePolicy->GetHorizontalMaxValue());
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
            LayoutLine(data, firstIndex, index - 1, childrenInLine, usedSize);
            firstIndex = index;
            childrenInLine = 1;
            usedSize = 0.0f;
        }
        
        float32 restSize = data.GetWidth() - usedSize;
        restSize -= horizontalPadding * 2.0f;
        if (childrenInLine > 0)
            restSize -= horizontalSpacing * (childrenInLine - 1);
        restSize -= childSize;
        if (restSize < -EPSILON)
        {
            if (index > firstIndex)
            {
                LayoutLine(data, firstIndex, index - 1, childrenInLine, restSize);
                firstIndex = index;
                childrenInLine = 1;
                restSize = childSize;
            }
            else
            {
                LayoutLine(data, firstIndex, index, 1, 0.0f);
                childrenInLine = 0;
                firstIndex = index + 1;
                childSize = 0.0f;
            }
        }
        else
        {
            childrenInLine++;
            usedSize += childSize;
        }
    }
    
    if (firstIndex <= data.GetLastChildIndex())
    {
        LayoutLine(data, firstIndex, data.GetLastChildIndex(), childrenInLine, usedSize);
    }
}

void FlowLayoutAlgorithm::LayoutLine(ControlLayoutData &data, int32 firstIndex, int32 lastIndex, int32 childrenCount, float32 childrenSize)
{
    float32 padding = horizontalPadding;
    float32 spacing = horizontalSpacing;
    CorrectPaddingAndSpacing(padding, spacing, dynamicHorizontalPadding, dynamicHorizontalSpacing, data.GetWidth() - childrenSize, childrenCount);
    
    float32 position = padding;
    if (inverse)
    {
        position = data.GetWidth() - padding;
    }
    
    for (int32 i = firstIndex; i <= lastIndex; i++)
    {
        ControlLayoutData &childData = layoutData[i];
        if (childData.HaveToSkipControl(skipInvisible))
        {
            continue;
        }
        
        float32 size = childData.GetWidth();
        childData.SetPosition(Vector2::AXIS_X, inverse ? position - size : position);
        
        if (inverse)
        {
            position -= size + spacing;
        }
        else
        {
            position += size + spacing;
        }
    }
    
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
    CorrectPaddingAndSpacing(verticalPadding, verticalSpacing, dynamicVerticalPadding, dynamicVerticalSpacing, restSize, linesCount);
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
        
        childData.SetPosition(Vector2::AXIS_Y, top); // TODO: Anchors
    }
}

void FlowLayoutAlgorithm::CorrectPaddingAndSpacing(float32 &padding, float32 &spacing, bool dynamicPadding, bool dynamicSpacing, float32 restSize, int32 childrenCount)
{
    if (childrenCount > 0)
    {
        int32 spacesCount = childrenCount - 1;
        restSize -= padding * 2.0f;
        restSize -= spacing * spacesCount;
        if (restSize > EPSILON)
        {
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
                
                float32 delta = restSize / cnt;
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
    }
}

}
