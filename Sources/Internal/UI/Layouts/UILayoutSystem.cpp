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

#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIFlowLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "SizeMeasuringAlgorithm.h"
#include "LinearLayoutAlgorithm.h"
#include "FlowLayoutAlgorithm.h"
#include "AnchorLayoutAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

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

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
}

void UILayoutSystem::ApplyLayout(UIControl *control, bool considerDenendenceOnChildren)
{
    UIControl *container = control;
    if (considerDenendenceOnChildren)
    {
        container = FindNotDependentOnChildrenControl(container);
    }

    CollectControls(container);
    
    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);
    
    ApplySizesAndPositions();
}

UIControl *UILayoutSystem::FindNotDependentOnChildrenControl(UIControl *control) const
{
    UIControl *result = control;
    while (result->GetParent() != nullptr)
    {
        UISizePolicyComponent *sizePolicy = result->GetParent()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y)))
        {
            result = result->GetParent();
        }
        else
        {
            break;
        }
    }
    
    return result;
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

void UILayoutSystem::ProcessAxis(Vector2::eAxis axis)
{
    DoMeasurePhase(axis);
    DoLayoutPhase(axis);
}

void UILayoutSystem::DoMeasurePhase(Vector2::eAxis axis)
{
    for (auto it = layoutData.rbegin(); it != layoutData.rend(); ++it)
    {
        SizeMeasuringAlgorithm(layoutData).Apply(*it, axis);
    }
}

void UILayoutSystem::DoLayoutPhase(Vector2::eAxis axis)
{
    for (auto it = layoutData.begin(); it != layoutData.end(); ++it)
    {
        UIFlowLayoutComponent *flowLayoutComponent = it->GetControl()->GetComponent<UIFlowLayoutComponent>();
        if (flowLayoutComponent && flowLayoutComponent->IsEnabled())
        {
            FlowLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis);
        }
        else
        {
            UILinearLayoutComponent *linearLayoutComponent = it->GetControl()->GetComponent<UILinearLayoutComponent>();
            if (linearLayoutComponent != nullptr && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
            {
                
                LinearLayoutAlgorithm alg(layoutData, isRtl);
                
                alg.SetInverse(isRtl && linearLayoutComponent->IsUseRtl() && linearLayoutComponent->GetOrientation() == UILinearLayoutComponent::HORIZONTAL);
                alg.SetSkipInvisible(linearLayoutComponent->IsSkipInvisibleControls());
                
                alg.SetPadding(linearLayoutComponent->GetPadding());
                alg.SetSpacing(linearLayoutComponent->GetSpacing());
                
                alg.SetDynamicPadding(linearLayoutComponent->IsDynamicPadding());
                alg.SetDynamicSpacing(linearLayoutComponent->IsDynamicSpacing());
                

                alg.Apply(*it, axis);
            }
            else
            {
                AnchorLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis, false);
            }
        }
    }
}

void UILayoutSystem::ApplySizesAndPositions()
{
    int32 indexOfSizeProperty = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyIndex(FastName("size"));
    
    for (ControlLayoutData &data : layoutData)
    {
        data.ApplyLayoutToControl(indexOfSizeProperty);
    }
    
    layoutData.clear();
}

}
