#include "UIFlowLayoutComponent.h"

namespace DAVA
{
UIFlowLayoutComponent::UIFlowLayoutComponent()
{
    
}

UIFlowLayoutComponent::UIFlowLayoutComponent(const UIFlowLayoutComponent &src)
    : padding(src.padding)
    , spacing(src.spacing)
    , dynamicPadding(src.dynamicPadding)
    , dynamicSpacing(src.dynamicSpacing)
    , useRtl(src.useRtl)
    , skipInvisibleControls(src.skipInvisibleControls)
{
    
}

UIFlowLayoutComponent::~UIFlowLayoutComponent()
{
    
}

UIFlowLayoutComponent* UIFlowLayoutComponent::Clone() const
{
    return new UIFlowLayoutComponent(*this);
}

float32 UIFlowLayoutComponent::GetPadding() const
{
    return padding;
}

void UIFlowLayoutComponent::SetPadding(float32 newPadding)
{
    padding = newPadding;
}

float32 UIFlowLayoutComponent::GetSpacing() const
{
    return spacing;
}

void UIFlowLayoutComponent::SetSpacing(float32 newSpacing)
{
    spacing = newSpacing;
}

bool UIFlowLayoutComponent::IsDynamicPadding() const
{
    return dynamicPadding;
}

void UIFlowLayoutComponent::SetDynamicPadding(bool dynamic)
{
    dynamicPadding = dynamic;
}

bool UIFlowLayoutComponent::IsDynamicSpacing() const
{
    return dynamicSpacing;
}

void UIFlowLayoutComponent::SetDynamicSpacing(bool dynamic)
{
    dynamicSpacing = dynamic;
}

bool UIFlowLayoutComponent::IsSkipInvisibleControls() const
{
    return skipInvisibleControls;
}

void UIFlowLayoutComponent::SetSkipInvisibleControls(bool skip)
{
    skipInvisibleControls = skip;
}

bool UIFlowLayoutComponent::IsUseRtl() const
{
    return useRtl;
}

void UIFlowLayoutComponent::SetUseRtl(bool use)
{
    useRtl = use;
}
    
}
