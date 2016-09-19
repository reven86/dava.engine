#include "UILinearLayoutComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UILinearLayoutComponent::UILinearLayoutComponent()
{
    SetEnabled(true);
    SetSkipInvisibleControls(true);
}

UILinearLayoutComponent::UILinearLayoutComponent(const UILinearLayoutComponent& src)
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
    if (flags.test(FLAG_ENABLED) == enabled)
    {
        return;
    }

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
    bool vertical = false;
    bool inverse = false;
    switch (direction)
    {
    case LEFT_TO_RIGHT:
        vertical = false;
        inverse = false;
        break;
    case RIGHT_TO_LEFT:
        vertical = false;
        inverse = true;
        break;
    case TOP_DOWN:
        vertical = true;
        inverse = false;
        break;
    case BOTTOM_UP:
        vertical = true;
        inverse = true;
        break;
    }

    if (flags.test(FLAG_ORIENTATION_VERTICAL) == vertical &&
        flags.test(FLAG_ORIENTATION_INVERSE) == inverse)
    {
        return;
    }

    flags.set(FLAG_ORIENTATION_VERTICAL, vertical);
    flags.set(FLAG_ORIENTATION_INVERSE, inverse);
    SetLayoutDirty();
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
    if (padding == newPadding)
    {
        return;
    }

    padding = newPadding;
    SetLayoutDirty();
}

float32 UILinearLayoutComponent::GetSpacing() const
{
    return spacing;
}

void UILinearLayoutComponent::SetSpacing(float32 newSpacing)
{
    if(spacing == newSpacing)
    {
        return;
    }

    spacing = newSpacing;
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsDynamicPadding() const
{
    return flags.test(FLAG_DYNAMIC_PADDING);
}

void UILinearLayoutComponent::SetDynamicPadding(bool dynamic)
{
    if (flags.test(FLAG_DYNAMIC_PADDING) == dynamic)
    {
        return;
    }

    flags.set(FLAG_DYNAMIC_PADDING, dynamic);
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsDynamicSpacing() const
{
    return flags.test(FLAG_DYNAMIC_SPACING);
}

void UILinearLayoutComponent::SetDynamicSpacing(bool dynamic)
{
    if (flags.test(FLAG_DYNAMIC_SPACING) == dynamic)
    {
        return;
    }

    flags.set(FLAG_DYNAMIC_SPACING, dynamic);
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsSkipInvisibleControls() const
{
    return flags.test(FLAG_SKIP_INVISIBLE_CONTROLS);
}

void UILinearLayoutComponent::SetSkipInvisibleControls(bool skip)
{
    if (flags.test(FLAG_SKIP_INVISIBLE_CONTROLS) == skip)
    {
        return;
    }

    flags.set(FLAG_SKIP_INVISIBLE_CONTROLS, skip);
    SetLayoutDirty();
}

bool UILinearLayoutComponent::IsUseRtl() const
{
    return flags.test(FLAG_RTL);
}

void UILinearLayoutComponent::SetUseRtl(bool use)
{
    if (flags.test(FLAG_RTL) == use)
    {
        return;
    }

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
