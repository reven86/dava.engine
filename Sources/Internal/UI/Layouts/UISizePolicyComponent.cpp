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


#include "UISizePolicyComponent.h"

#include "Math/Vector.h"

namespace DAVA
{
UISizePolicyComponent::UISizePolicyComponent()
{
    const float32 DEFAULT_VALUE = 100.0f;
    const float32 MIN_LIMIT = 0.0f;
    const float32 MAX_LIMIT = 99999.0f;
    
    for (int32 i = 0; i < Vector2::AXIS_COUNT; i++)
    {
        policy[i].policy = IGNORE_SIZE;
        policy[i].value = DEFAULT_VALUE;
        policy[i].min = MIN_LIMIT;
        policy[i].max = MAX_LIMIT;
    }
}

UISizePolicyComponent::UISizePolicyComponent(const UISizePolicyComponent &src)
{
    for (int32 i = 0; i < Vector2::AXIS_COUNT; i++)
    {
        policy[i].policy = src.policy[i].policy;
        policy[i].value = src.policy[i].value;
        policy[i].min = src.policy[i].min;
        policy[i].max = src.policy[i].max;
    }
}

UISizePolicyComponent::~UISizePolicyComponent()
{
    
}

UISizePolicyComponent* UISizePolicyComponent::Clone() const
{
    return new UISizePolicyComponent(*this);
}

UISizePolicyComponent::eSizePolicy UISizePolicyComponent::GetHorizontalPolicy() const
{
    return policy[Vector2::AXIS_X].policy;
}

void UISizePolicyComponent::SetHorizontalPolicy(eSizePolicy newPolicy)
{
    policy[Vector2::AXIS_X].policy = newPolicy;
}

float32 UISizePolicyComponent::GetHorizontalValue() const
{
    return policy[Vector2::AXIS_X].value;
}

void UISizePolicyComponent::SetHorizontalValue(float32 value)
{
    policy[Vector2::AXIS_X].value = value;
}

float32 UISizePolicyComponent::GetHorizontalMinValue() const
{
    return policy[Vector2::AXIS_X].min;
}

void UISizePolicyComponent::SetHorizontalMinValue(float32 value)
{
    policy[Vector2::AXIS_X].min = value;
}

float32 UISizePolicyComponent::GetHorizontalMaxValue() const
{
    return policy[Vector2::AXIS_X].max;
}

void UISizePolicyComponent::SetHorizontalMaxValue(float32 value)
{
    policy[Vector2::AXIS_X].max = value;
}

UISizePolicyComponent::eSizePolicy UISizePolicyComponent::GetVerticalPolicy() const
{
    return policy[Vector2::AXIS_Y].policy;
}

void UISizePolicyComponent::SetVerticalPolicy(eSizePolicy newPolicy)
{
    policy[Vector2::AXIS_Y].policy = newPolicy;
}

float32 UISizePolicyComponent::GetVerticalValue() const
{
    return policy[Vector2::AXIS_Y].value;
}

void UISizePolicyComponent::SetVerticalValue(float32 value)
{
    policy[Vector2::AXIS_Y].value = value;
}

float32 UISizePolicyComponent::GetVerticalMinValue() const
{
    return policy[Vector2::AXIS_Y].min;
}

void UISizePolicyComponent::SetVerticalMinValue(float32 value)
{
    policy[Vector2::AXIS_Y].min = value;
}

float32 UISizePolicyComponent::GetVerticalMaxValue() const
{
    return policy[Vector2::AXIS_Y].max;
}

void UISizePolicyComponent::SetVerticalMaxValue(float32 value)
{
    policy[Vector2::AXIS_Y].max = value;
}

UISizePolicyComponent::eSizePolicy UISizePolicyComponent::GetPolicyByAxis(int32 axis) const
{
    DVASSERT(0 <= axis && axis < Vector2::AXIS_COUNT);
    return policy[axis].policy;
}

float32 UISizePolicyComponent::GetValueByAxis(int32 axis) const
{
    DVASSERT(0 <= axis && axis < Vector2::AXIS_COUNT);
    return policy[axis].value;
}
    
float32 UISizePolicyComponent::GetMinValueByAxis(int32 axis) const
{
    DVASSERT(0 <= axis && axis < Vector2::AXIS_COUNT);
    return policy[axis].min;
}

float32 UISizePolicyComponent::GetMaxValueByAxis(int32 axis) const
{
    DVASSERT(0 <= axis && axis < Vector2::AXIS_COUNT);
    return policy[axis].max;
}

int32 UISizePolicyComponent::GetHorizontalPolicyAsInt() const
{
    return GetHorizontalPolicy();
}

void UISizePolicyComponent::SetHorizontalPolicyFromInt(int32 policy)
{
    SetHorizontalPolicy(static_cast<eSizePolicy>(policy));
}

int32 UISizePolicyComponent::GetVerticalPolicyAsInt() const
{
    return GetVerticalPolicy();
}

void UISizePolicyComponent::SetVerticalPolicyFromInt(int32 policy)
{
    SetVerticalPolicy(static_cast<eSizePolicy>(policy));
}

}
