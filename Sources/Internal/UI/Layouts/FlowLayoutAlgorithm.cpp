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
}
    
void FlowLayoutAlgorithm::ProcessXAxis(const ControlLayoutData &data, UIFlowLayoutComponent *component)
{
    const float32 padding = component->GetHorizontalPadding();
    const float32 spacing = component->GetHorizontalSpacing();
    const float32 controlSize = data.GetWidth();
    float32 x = padding;
    for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
    {
        ControlLayoutData &childData = layoutData[index];
        if (childData.HaveToSkipControl(skipInvisible))
            continue;
        
        float32 childSize = childData.GetWidth();
        UISizePolicyComponent *sizePolicy = childData.GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy)
        {
            if (sizePolicy->GetHorizontalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                float32 restSize = data.GetWidth() - padding * 2;
                float32 value = sizePolicy->GetHorizontalValue();
                if (value < 100.0f && value > EPSILON)
                {
                    int32 potentialSpacesCount = static_cast<int32>(100.0f / value) - 1;
                    restSize -= potentialSpacesCount * spacing;
                }
                
                childSize = restSize * value / 100.0f;
                childSize = Clamp(childSize, sizePolicy->GetHorizontalMinValue(), sizePolicy->GetHorizontalMaxValue());
                childData.SetSize(Vector2::AXIS_X, childSize);
            }
        }
        
        if (x > padding + EPSILON && x + childSize > controlSize - padding + EPSILON)
        {
            x = padding;
            childData.SetFlag(ControlLayoutData::FLAG_FLOW_LAYOUT_NEW_LINE);
        }
        childData.SetPosition(Vector2::AXIS_X, x);
        x += childData.GetWidth() + spacing;
    }
}

void FlowLayoutAlgorithm::ProcessYAxis(const ControlLayoutData &data, UIFlowLayoutComponent *component)
{
    const float32 padding = component->GetVerticalPadding();
    const float32 spacing = component->GetVerticalSpacing();

    float32 y = padding;
    float32 lineHeight = 0;
    for (int32 index = data.GetFirstChildIndex(); index <= data.GetLastChildIndex(); index++)
    {
        ControlLayoutData &childData = layoutData[index];
        if (childData.HaveToSkipControl(skipInvisible))
            continue;

        float32 childSize = childData.GetHeight();
        UISizePolicyComponent *sizePolicy = childData.GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy)
        {
            if (sizePolicy->GetVerticalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                float32 restSize = data.GetHeight() - padding * 2.0f;
                childSize = restSize * sizePolicy->GetVerticalValue() / 100.0f;
                childSize = Clamp(childSize, sizePolicy->GetVerticalMinValue(), sizePolicy->GetVerticalMaxValue());
                childData.SetSize(Vector2::AXIS_Y, childSize);
            }
        }
        
        if (childData.HasFlag(ControlLayoutData::FLAG_FLOW_LAYOUT_NEW_LINE))
        {
            y += lineHeight + spacing;
            lineHeight = 0;
        }
        childData.SetPosition(Vector2::AXIS_Y, y);
        
        lineHeight = Max(lineHeight, childData.GetHeight());
    }

}
    
}
