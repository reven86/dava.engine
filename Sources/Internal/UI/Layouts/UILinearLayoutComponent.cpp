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

#include "UILinearLayoutComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UILinearLayoutComponent::UILinearLayoutComponent()
{
    SetEnabled(true);
    SetSkipInvisibleControls(true);
}

UILinearLayoutComponent::UILinearLayoutComponent(const UILinearLayoutComponent &src)
    : flags(src.flags)
    , padding(src.padding)
    , spacing(src.spacing)
{
}

UILinearLayoutComponent::~UILinearLayoutComponent()
{
}

UILinearLayoutComponent* UILinearLayoutComponent::Clone() const
{
    return new UILinearLayoutComponent(*this);
}

bool UILinearLayoutComponent::IsEnabled() const
{
    return flags.test(FLAG_ENABLED);
}

void UILinearLayoutComponent::SetEnabled(bool enabled)
{
    flags.set(FLAG_ENABLED, enabled);
    SetLayoutDirty();
}

UILinearLayoutComponent::eOrientation UILinearLayoutComponent::GetOrientation() const
{
    if (flags.test(FLAG_ORIENTATION_VERTICAL))
    {
        return flags.test(FLAG_ORIENTATION_INVERSE) ? BOTTOM_UP : TOP_DOWN;
    }
    else
    {
        return flags.test(FLAG_ORIENTATION_INVERSE) ? RIGHT_TO_LEFT : LEFT_TO_RIGHT;
    }
}

void UILinearLayoutComponent::SetOrientation(eOrientation direction)
{
    switch (direction)
    {
    case LEFT_TO_RIGHT:
        flags.set(FLAG_ORIENTATION_VERTICAL, false);
        flags.set(FLAG_ORIENTATION_INVERSE, false);
        break;
    case RIGHT_TO_LEFT:
        flags.set(FLAG_ORIENTATION_VERTICAL, false);
        flags.set(FLAG_ORIENTATION_INVERSE, true);
        break;
    case TOP_DOWN:
        flags.set(FLAG_ORIENTATION_VERTICAL, true);
        flags.set(FLAG_ORIENTATION_INVERSE, false);
        break;
    case BOTTOM_UP:
        flags.set(FLAG_ORIENTATION_VERTICAL, true);
        flags.set(FLAG_ORIENTATION_INVERSE, true);
        break;
    }
}

Vector2::eAxis UILinearLayoutComponent::GetAxis() const
{
    return flags.test(FLAG_ORIENTATION_VERTICAL) ? Vector2::AXIS_Y : Vector2::AXIS_X;
}

bool UILinearLayoutComponent::IsInverse() const
{
    return flags.test(FLAG_ORIENTATION_INVERSE);
}

float32 UILinearLayoutComponent::GetPadding() const
{
    return padding;
}

void UILinearLayoutComponent::SetPadding(float32 newPadding)
{
    padding = newPadding;
    SetLayoutDirty();
}

float32 UILinearLayoutComponent::GetSpacing() const
{
    return spacing;
}

void UILinearLayoutComponent::SetSpacing(float32 newSpacing)
{
    spacing = newSpacing;
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsDynamicPadding() const
{
    return flags.test(FLAG_DYNAMIC_PADDING);
}

void UILinearLayoutComponent::SetDynamicPadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_PADDING, dynamic);
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsDynamicSpacing() const
{
    return flags.test(FLAG_DYNAMIC_SPACING);
}

void UILinearLayoutComponent::SetDynamicSpacing(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_SPACING, dynamic);
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsSkipInvisibleControls() const
{
    return flags.test(FLAG_SKIP_INVISIBLE_CONTROLS);
}

void UILinearLayoutComponent::SetSkipInvisibleControls(bool skip)
{
    flags.set(FLAG_SKIP_INVISIBLE_CONTROLS, skip);
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsUseRtl() const
{
    return flags.test(FLAG_RTL);
}

void UILinearLayoutComponent::SetUseRtl(bool use)
{
    flags.set(FLAG_RTL, use);
    SetLayoutDirty();
}

int32 UILinearLayoutComponent::GetOrientationAsInt() const
{
    return static_cast<int32>(GetOrientation());
}

void UILinearLayoutComponent::SetOrientationFromInt(int32 orientation)
{
    SetOrientation(static_cast<eOrientation>(orientation));
}

void UILinearLayoutComponent::SetLayoutDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
