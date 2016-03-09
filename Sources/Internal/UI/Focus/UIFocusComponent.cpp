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

#include "UIFocusComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UIFocusComponent::UIFocusComponent()
{
}

UIFocusComponent::UIFocusComponent(const UIFocusComponent& src)
    : enabled(src.enabled)
    , requestFocus(src.requestFocus)
    , policy(src.policy)
    , leftControl(src.leftControl)
    , rightControl(src.rightControl)
    , upControl(src.upControl)
    , downControl(src.downControl)
    , tabOrder(src.tabOrder)
{
}

UIFocusComponent::~UIFocusComponent()
{
}

UIFocusComponent* UIFocusComponent::Clone() const
{
    return new UIFocusComponent(*this);
}

bool UIFocusComponent::IsEnabled() const
{
    return enabled;
}

void UIFocusComponent::SetEnabled(bool value)
{
    enabled = value;
}

bool UIFocusComponent::IsRequestFocus() const
{
    return requestFocus;
}

void UIFocusComponent::SetRequestFocus(bool value)
{
    requestFocus = value;
}

UIFocusComponent::ePolicy UIFocusComponent::GetPolicy() const
{
    return policy;
}

void UIFocusComponent::SetPolicy(ePolicy policy_)
{
    policy = policy_;
}

const String& UIFocusComponent::GetLeft() const
{
    return leftControl;
}

void UIFocusComponent::SetLeft(const String& val)
{
    leftControl = val;
}

const String& UIFocusComponent::GetRight() const
{
    return rightControl;
}

void UIFocusComponent::SetRight(const String& val)
{
    rightControl = val;
}

const String& UIFocusComponent::GetUp() const
{
    return upControl;
}

void UIFocusComponent::SetUp(const String& val)
{
    upControl = val;
}

const String& UIFocusComponent::GetDown() const
{
    return downControl;
}

void UIFocusComponent::SetDown(const String& val)
{
    downControl = val;
}

const String& UIFocusComponent::GetControlInDirection(FocusHelpers::Direction dir)
{
    switch (dir)
    {
    case FocusHelpers::LEFT:
        return leftControl;

    case FocusHelpers::RIGHT:
        return rightControl;

    case FocusHelpers::UP:
        return upControl;

    case FocusHelpers::DOWN:
        return downControl;

    default:
        DVASSERT(false);
        return leftControl;
    }
}

int32 UIFocusComponent::GetTabOrder() const
{
    return tabOrder;
}

void UIFocusComponent::SetTabOrder(int32 val)
{
    tabOrder = val;
}

int32 UIFocusComponent::GetPolicyAsInt() const
{
    return static_cast<int32>(policy);
}

void UIFocusComponent::SetPolicyFromInt(int32 policy)
{
    SetPolicy(static_cast<ePolicy>(policy));
}
}
