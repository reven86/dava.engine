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

#include "LinearLayoutAlgorithm.h"

#include "UISizePolicyComponent.h"

#include "AnchorLayoutAlgorithm.h"
#include "LayoutHelpers.h"

#include "UI/UIControl.h"

namespace DAVA
{
LinearLayoutAlgorithm::LinearLayoutAlgorithm(Vector<ControlLayoutData>& layoutData_, bool isRtl_)
    : layoutData(layoutData_)
    , isRtl(isRtl_)
{
}

LinearLayoutAlgorithm::~LinearLayoutAlgorithm()
{
}

void LinearLayoutAlgorithm::SetInverse(bool inverse_)
{
    inverse = inverse_;
}

void LinearLayoutAlgorithm::SetSkipInvisible(bool skipInvisible_)
{
    skipInvisible = skipInvisible_;
}

void LinearLayoutAlgorithm::SetPadding(float32 padding_)
{
    initialPadding = padding_;
}

void LinearLayoutAlgorithm::SetSpacing(float32 spacing_)
{
    initialSpacing = spacing_;
}

void LinearLayoutAlgorithm::SetDynamicPadding(bool dynamicPadding_)
{
    dynamicPadding = dynamicPadding_;
}

void LinearLayoutAlgorithm::SetDynamicSpacing(bool dynamicSpacing_)
{
    dynamicSpacing = dynamicSpacing_;
}

void LinearLayoutAlgorithm::Apply(ControlLayoutData& data, Vector2::eAxis axis)
{
    if (data.HasChildren())
    {
        Apply(data, axis, data.GetFirstChildIndex(), data.GetLastChildIndex());
    }
}

void LinearLayoutAlgorithm::Apply(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex)
{
    DVASSERT(firstIndex <= lastIndex);

    InitializeParams(data, axis, firstIndex, lastIndex);

    if (childrenCount > 0)
    {
        CalculateDependentOnParentSizes(data, axis, firstIndex, lastIndex);
        CalculateDynamicPaddingAndSpaces(data, axis);
        PlaceChildren(data, axis, firstIndex, lastIndex);
    }

    AnchorLayoutAlgorithm anchorAlg(layoutData, isRtl);
    anchorAlg.Apply(data, axis, true, firstIndex, lastIndex);
}

void LinearLayoutAlgorithm::InitializeParams(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex)
{
    padding = initialPadding;
    spacing = initialSpacing;

    fixedSize = 0.0f;
    totalPercent = 0.0f;

    childrenCount = 0;
    for (int32 i = firstIndex; i <= lastIndex; i++)
    {
        const ControlLayoutData& childData = layoutData[i];
        if (childData.HaveToSkipControl(skipInvisible))
        {
            continue;
        }

        childrenCount++;

        const UISizePolicyComponent* sizeHint = childData.GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizeHint != nullptr && sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
        {
            totalPercent += sizeHint->GetValueByAxis(axis);
        }
        else
        {
            fixedSize += childData.GetSize(axis);
        }
    }

    spacesCount = childrenCount - 1;
    contentSize = data.GetSize(axis) - padding * 2.0f;
    restSize = contentSize - fixedSize - spacesCount * spacing;
}

void LinearLayoutAlgorithm::CalculateDependentOnParentSizes(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex)
{
    int32 index = firstIndex;
    while (index <= lastIndex)
    {
        ControlLayoutData& childData = layoutData[index];

        bool haveToSkip = childData.HasFlag(ControlLayoutData::FLAG_SIZE_CALCULATED) || childData.HaveToSkipControl(skipInvisible);

        bool needRestart = false;
        if (!haveToSkip)
        {
            bool sizeWasLimited = CalculateChildDependentOnParentSize(childData, axis);
            if (sizeWasLimited)
            {
                needRestart = true;
                childData.SetFlag(ControlLayoutData::FLAG_SIZE_CALCULATED);
            }
        }

        if (needRestart)
        {
            index = firstIndex;
        }
        else
        {
            index++;
        }
    }
}

bool LinearLayoutAlgorithm::CalculateChildDependentOnParentSize(ControlLayoutData& childData, Vector2::eAxis axis)
{
    const UISizePolicyComponent* sizeHint = childData.GetControl()->GetComponent<UISizePolicyComponent>();
    if (sizeHint != nullptr && sizeHint->GetPolicyByAxis(axis) == UISizePolicyComponent::PERCENT_OF_PARENT)
    {
        float32 percents = sizeHint->GetValueByAxis(axis);
        float32 size = 0.0f;
        if (totalPercent > LayoutHelpers::EPSILON)
        {
            size = restSize * percents / Max(totalPercent, 100.0f);
        }

        float32 minSize = sizeHint->GetMinValueByAxis(axis);
        float32 maxSize = sizeHint->GetMaxValueByAxis(axis);
        if (size < minSize || maxSize < size)
        {
            size = Clamp(size, minSize, maxSize);
            childData.SetSize(axis, size);

            restSize -= size;
            totalPercent -= percents;

            return true;
        }
        else
        {
            childData.SetSize(axis, size);
        }
    }
    return false;
}

void LinearLayoutAlgorithm::CalculateDynamicPaddingAndSpaces(ControlLayoutData& data, Vector2::eAxis axis)
{
    if (totalPercent < 100.0f - LayoutHelpers::EPSILON && restSize > LayoutHelpers::EPSILON)
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
}

void LinearLayoutAlgorithm::PlaceChildren(ControlLayoutData& data, Vector2::eAxis axis, int32 firstIndex, int32 lastIndex)
{
    float32 position = padding;
    if (inverse)
    {
        position = data.GetSize(axis) - padding;
    }

    for (int32 i = firstIndex; i <= lastIndex; i++)
    {
        ControlLayoutData& childData = layoutData[i];
        if (childData.HaveToSkipControl(skipInvisible))
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
