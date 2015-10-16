#include "UIFlowLayoutComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UIFlowLayoutComponent::UIFlowLayoutComponent()
{
    SetEnabled(true);
    SetSkipInvisibleControls(true);

    for (int32 i = 0; i < Vector2::AXIS_COUNT; i++)
    {
        padding[i] = 0.0f;
        spacing[i] = 0.0f;
    }
}

UIFlowLayoutComponent::UIFlowLayoutComponent(const UIFlowLayoutComponent& src)
    : flags(src.flags)
{
    for (int32 i = 0; i < Vector2::AXIS_COUNT; i++)
    {
        padding[i] = src.padding[i];
        spacing[i] = src.spacing[i];
    }
}

UIFlowLayoutComponent::~UIFlowLayoutComponent()
{
}

UIFlowLayoutComponent* UIFlowLayoutComponent::Clone() const
{
    return new UIFlowLayoutComponent(*this);
}

bool UIFlowLayoutComponent::IsEnabled() const
{
    return flags.test(FLAG_ENABLED);
}

void UIFlowLayoutComponent::SetEnabled(bool enabled)
{
    flags.set(FLAG_ENABLED, enabled);
}

UIFlowLayoutComponent::eOrientation UIFlowLayoutComponent::GetOrientation() const
{
    return flags.test(FLAG_IS_RIGHT_TO_LEFT) ? ORIENTATION_RIGHT_TO_LEFT : ORIENTATION_LEFT_TO_RIGHT;
}

void UIFlowLayoutComponent::SetOrientation(eOrientation orientation)
{
    flags.set(FLAG_IS_RIGHT_TO_LEFT, orientation == ORIENTATION_RIGHT_TO_LEFT);
}

float32 UIFlowLayoutComponent::GetHorizontalPadding() const
{
    return padding[Vector2::AXIS_X];
}

void UIFlowLayoutComponent::SetHorizontalPadding(float32 newPadding)
{
    padding[Vector2::AXIS_X] = newPadding;
}

float32 UIFlowLayoutComponent::GetHorizontalSpacing() const
{
    return spacing[Vector2::AXIS_X];
}

void UIFlowLayoutComponent::SetHorizontalSpacing(float32 newSpacing)
{
    spacing[Vector2::AXIS_X] = newSpacing;
    SetLayoutDirty();
}

bool UIFlowLayoutComponent::IsDynamicHorizontalPadding() const
{
    return flags.test(FLAG_DYNAMIC_HORIZONTAL_PADDING);
}

void UIFlowLayoutComponent::SetDynamicHorizontalPadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_HORIZONTAL_PADDING, dynamic);
    SetLayoutDirty();
}

bool UIFlowLayoutComponent::IsDynamicHorizontalInLinePadding() const
{
    return flags.test(FLAG_DYNAMIC_HORIZONTAL_IN_LINE_PADDING);
}

void UIFlowLayoutComponent::SetDynamicHorizontalInLinePadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_HORIZONTAL_IN_LINE_PADDING, dynamic);
    SetLayoutDirty();
}

bool UIFlowLayoutComponent::IsDynamicHorizontalSpacing() const
{
    return flags.test(FLAG_DYNAMIC_HORIZONTAL_SPACING);
}

void UIFlowLayoutComponent::SetDynamicHorizontalSpacing(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_HORIZONTAL_SPACING, dynamic);
    SetLayoutDirty();
}

float32 UIFlowLayoutComponent::GetVerticalPadding() const
{
    return padding[Vector2::AXIS_Y];
}

void UIFlowLayoutComponent::SetVerticalPadding(float32 newPadding)
{
    padding[Vector2::AXIS_Y] = newPadding;
    SetLayoutDirty();
}

float32 UIFlowLayoutComponent::GetVerticalSpacing() const
{
    return spacing[Vector2::AXIS_Y];
}

void UIFlowLayoutComponent::SetVerticalSpacing(float32 newSpacing)
{
    spacing[Vector2::AXIS_Y] = newSpacing;
    SetLayoutDirty();
}

bool UIFlowLayoutComponent::IsDynamicVerticalPadding() const
{
    return flags.test(FLAG_DYNAMIC_VERTICAL_PADDING);
}

void UIFlowLayoutComponent::SetDynamicVerticalPadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_VERTICAL_PADDING, dynamic);
    SetLayoutDirty();
}

bool UIFlowLayoutComponent::IsDynamicVerticalSpacing() const
{
    return flags.test(FLAG_DYNAMIC_VERTICAL_SPACING);
}

void UIFlowLayoutComponent::SetDynamicVerticalSpacing(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_VERTICAL_SPACING, dynamic);
    SetLayoutDirty();
}

float32 UIFlowLayoutComponent::GetPaddingByAxis(int32 axis)
{
    return padding[axis];
}

float32 UIFlowLayoutComponent::GetSpacingByAxis(int32 axis)
{
    return spacing[axis];
}

bool UIFlowLayoutComponent::IsUseRtl() const
{
    return flags.test(FLAG_USE_RTL);
}

void UIFlowLayoutComponent::SetUseRtl(bool use)
{
    flags.set(FLAG_USE_RTL, use);
    SetLayoutDirty();
}

bool UIFlowLayoutComponent::IsSkipInvisibleControls() const
{
    return flags.test(FLAG_SKIP_INVISIBLE_CONTROLS);
}

void UIFlowLayoutComponent::SetSkipInvisibleControls(bool skip)
{
    flags.set(FLAG_SKIP_INVISIBLE_CONTROLS, skip);
    SetLayoutDirty();
}

int32 UIFlowLayoutComponent::GetOrientationAsInt() const
{
    return GetOrientation();
}

void UIFlowLayoutComponent::SetOrientationFromInt(int32 orientation)
{
    SetOrientation(static_cast<eOrientation>(orientation));
}

void UIFlowLayoutComponent::SetLayoutDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
