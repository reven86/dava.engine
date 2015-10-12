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

#include "AnchorLayoutAlgorithm.h"

#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
    
AnchorLayoutAlgorithm::AnchorLayoutAlgorithm(Vector<ControlLayoutData> &layoutData_, bool isRtl_)
    : layoutData(layoutData_)
    , isRtl(isRtl_)
{
    
}

AnchorLayoutAlgorithm::~AnchorLayoutAlgorithm()
{
    
}

void AnchorLayoutAlgorithm::Apply(ControlLayoutData &data, Vector2::eAxis axis, bool onlyForIgnoredControls)
{
    Apply(data, axis, onlyForIgnoredControls, data.GetFirstChildIndex(), data.GetLastChildIndex());
}
    
void AnchorLayoutAlgorithm::Apply(ControlLayoutData &data, Vector2::eAxis axis, bool onlyForIgnoredControls, int32 firstIndex, int32 lastIndex)
{
    for (int32 i = firstIndex; i <= lastIndex; i++)
    {
        ControlLayoutData &childData = layoutData[i];
        if (!onlyForIgnoredControls || childData.HaveToSkipControl(false))
        {
            const UISizePolicyComponent* sizeHint = childData.GetControl()->GetComponent<UISizePolicyComponent>();
            if (sizeHint != nullptr && sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
            {
                float32 size = data.GetSize(axis) * sizeHint->GetValueByAxis(axis) / 100.0f;
                size = Clamp(size, sizeHint->GetMinValueByAxis(axis), sizeHint->GetMaxValueByAxis(axis));
                childData.SetSize(axis, size);
            }

            ApplyAnchor(childData, axis, 0.0f, data.GetSize(axis), isRtl);
        }
    }
}

void AnchorLayoutAlgorithm::ApplyAnchor(ControlLayoutData& data, Vector2::eAxis axis, float32 min, float32 max, bool isRtl)
{
    UIAnchorComponent* hint = data.GetControl()->GetComponent<UIAnchorComponent>();
    if (hint != nullptr && hint->IsEnabled())
    {
        float32 v1 = 0.0f;
        bool v1Enabled = false;

        float32 v2 = 0.0f;
        bool v2Enabled = false;

        float32 v3 = 0.0f;
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
            float32 parentSize = max - min;

            if (v1Enabled && v3Enabled) // left and right
            {
                data.SetPosition(axis, v1 + min);
                data.SetSize(axis, parentSize - (v1 + v3));
            }
            else if (v1Enabled && v2Enabled) // left and center
            {
                data.SetPosition(axis, v1 + min);
                data.SetSize(axis, parentSize / 2.0f - (v1 - v2));
            }
            else if (v2Enabled && v3Enabled) // center and right
            {
                data.SetPosition(axis, parentSize / 2.0f + v2 + min);
                data.SetSize(axis, parentSize / 2.0f - (v2 + v3));
            }
            else if (v1Enabled) // left
            {
                data.SetPosition(axis, v1 + min);
            }
            else if (v2Enabled) // center
            {
                data.SetPosition(axis, (parentSize - data.GetSize(axis)) / 2.0f + v2 + min);
            }
            else if (v3Enabled) // right
            {
                data.SetPosition(axis, parentSize - (data.GetSize(axis) + v3) + min);
            }
        }
    }
}

}
