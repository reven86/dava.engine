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

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

namespace DAVA
{
    
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
            SizeMeasuringAlgorithm alg(layoutData);
            alg.Apply(*it, axis);
        }

        // layout phase
        for (auto it = layoutData.begin(); it != layoutData.end(); ++it)
        {
            UIFlowLayoutComponent *flowLayoutComponent = it->GetControl()->GetComponent<UIFlowLayoutComponent>();
            if (flowLayoutComponent && flowLayoutComponent->IsEnabled())
            {
                FlowLayoutAlgorithm alg(layoutData, isRtl);
                alg.Apply(*it, axis);
            }
            else
            {
                UILinearLayoutComponent *linearLayoutComponent = it->GetControl()->GetComponent<UILinearLayoutComponent>();
                bool anchorOnlyIgnoredControls = false;
                if (linearLayoutComponent != nullptr && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
                {
                    LinearLayoutAlgorithm alg(layoutData, isRtl);
                    alg.Apply(*it, axis);
                    
                    anchorOnlyIgnoredControls = true;
                }
                
                ApplyAnchorLayout(*it, axis, anchorOnlyIgnoredControls);
            }
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
// Anchor Layout
////////////////////////////////////////////////////////////////////////////////

void UILayoutSystem::ApplyAnchorLayout(ControlLayoutData &data, Vector2::eAxis axis, bool onlyForIgnoredControls)
{
    
    for (int32 i = data.GetFirstChildIndex(); i <= data.GetLastChildIndex(); i++)
    {
        ControlLayoutData &childData = layoutData[i];
        if (onlyForIgnoredControls && childData.GetControl()->GetComponentCount(UIComponent::IGNORE_LAYOUT_COMPONENT) == 0)
        {
            continue;
        }
        
        const UISizePolicyComponent *sizeHint = childData.GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizeHint != nullptr)
        {
            if (sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                float32 size = data.GetSize(axis) * sizeHint->GetValueByAxis(axis) / 100.0f;
                size = Clamp(size, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));

                childData.SetSize(axis, size);
            }
        }
        
        UIAnchorComponent *hint = childData.GetControl()->GetComponent<UIAnchorComponent>();
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

}
