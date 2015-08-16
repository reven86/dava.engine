#include "UIFlowLayoutComponent.h"

namespace DAVA
{
UIFlowLayoutComponent::UIFlowLayoutComponent()
{
    
}

UIFlowLayoutComponent::UIFlowLayoutComponent(const UIFlowLayoutComponent &src)
    : horizontalPadding(src.horizontalPadding)
    , horizontalSpacing(src.horizontalSpacing)
    , verticalPadding(src.verticalPadding)
    , verticalSpacing(src.verticalSpacing)
    , flags(src.flags)
{
    
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
    return horizontalPadding;
}

void UIFlowLayoutComponent::SetHorizontalPadding(float32 padding)
{
    horizontalPadding = padding;
}

float32 UIFlowLayoutComponent::GetHorizontalSpacing() const
{
    return horizontalSpacing;
}

void UIFlowLayoutComponent::SetHorizontalSpacing(float32 spacing)
{
    horizontalSpacing = spacing;
}

bool UIFlowLayoutComponent::IsDynamicHorizontalPadding() const
{
    return flags.test(FLAG_DYNAMIC_HORIZONTAL_PADDING);
}

void UIFlowLayoutComponent::SetDynamicHorizontalPadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_HORIZONTAL_PADDING, dynamic);
}

bool UIFlowLayoutComponent::IsDynamicHorizontalSpacing() const
{
    return flags.test(FLAG_DYNAMIC_HORIZONTAL_SPACING);
}

void UIFlowLayoutComponent::SetDynamicHorizontalSpacing(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_HORIZONTAL_SPACING, dynamic);
}

float32 UIFlowLayoutComponent::GetVerticalPadding() const
{
    return verticalPadding;
}

void UIFlowLayoutComponent::SetVerticalPadding(float32 padding)
{
    verticalPadding = padding;
}

float32 UIFlowLayoutComponent::GetVerticalSpacing() const
{
    return verticalSpacing;
}

void UIFlowLayoutComponent::SetVerticalSpacing(float32 spacing)
{
    verticalSpacing = spacing;
}

bool UIFlowLayoutComponent::IsDynamicVerticalPadding() const
{
    return flags.test(FLAG_DYNAMIC_VERTICAL_PADDING);
}

void UIFlowLayoutComponent::SetDynamicVerticalPadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_VERTICAL_PADDING, dynamic);
}

bool UIFlowLayoutComponent::IsDynamicVerticalSpacing() const
{
    return flags.test(FLAG_DYNAMIC_VERTICAL_SPACING);
}

void UIFlowLayoutComponent::SetDynamicVerticalSpacing(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_VERTICAL_SPACING, dynamic);
}

    
bool UIFlowLayoutComponent::IsUseRtl() const
{
    return flags.test(FLAG_USE_RTL);
}

void UIFlowLayoutComponent::SetUseRtl(bool use)
{
    flags.set(FLAG_USE_RTL, use);
}

bool UIFlowLayoutComponent::IsSkipInvisibleControls() const
{
    return flags.test(FLAG_SKIP_INVISIBLE_CONTROLS);
}
    
void UIFlowLayoutComponent::SetSkipInvisibleControls(bool skip)
{
    flags.set(FLAG_SKIP_INVISIBLE_CONTROLS, skip);
}

int32 UIFlowLayoutComponent::GetOrientationAsInt() const
{
    return GetOrientation();
}

void UIFlowLayoutComponent::SetOrientationFromInt(int32 orientation)
{
    SetOrientation(static_cast<eOrientation>(orientation));
}

}
