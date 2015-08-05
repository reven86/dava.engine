#include "UIAnchorComponent.h"

namespace DAVA
{
UIAnchorComponent::UIAnchorComponent()
{
    
}

UIAnchorComponent::UIAnchorComponent(const UIAnchorComponent &src)
    : flags(src.flags)
    , leftAnchor(src.leftAnchor)
    , hCenterAnchor(src.hCenterAnchor)
    , rightAnchor(src.rightAnchor)
    , topAnchor(src.topAnchor)
    , vCenterAnchor(src.vCenterAnchor)
    , bottomAnchor(src.bottomAnchor)
{
    
}
    
UIAnchorComponent::~UIAnchorComponent()
{
    
}
    
UIAnchorComponent* UIAnchorComponent::Clone() const
{
    return new UIAnchorComponent(*this);
}
    
bool UIAnchorComponent::IsLeftAnchorEnabled() const
{
    return flags.test(FLAG_LEFT_ENABLED);
}

void UIAnchorComponent::SetLeftAnchorEnabled(bool enabled)
{
    flags.set(FLAG_LEFT_ENABLED, enabled);
}
    
float32 UIAnchorComponent::GetLeftAnchor() const
{
    return leftAnchor;
}

void UIAnchorComponent::SetLeftAnchor(float32 anchor)
{
    leftAnchor = anchor;
}

bool UIAnchorComponent::IsHCenterAnchorEnabled() const
{
    return flags.test(FLAG_HCENTER_ENABLED);
}

void UIAnchorComponent::SetHCenterAnchorEnabled(bool enabled)
{
    flags.set(FLAG_HCENTER_ENABLED, enabled);
}
    
float32 UIAnchorComponent::GetHCenterAnchor() const
{
    return hCenterAnchor;
}

void UIAnchorComponent::SetHCenterAnchor(float32 anchor)
{
    hCenterAnchor = anchor;
}

bool UIAnchorComponent::IsRightAnchorEnabled() const
{
    return flags.test(FLAG_RIGHT_ENABLED);
}

void UIAnchorComponent::SetRightAnchorEnabled(bool enabled)
{
    flags.set(FLAG_RIGHT_ENABLED, enabled);
}
    
float32 UIAnchorComponent::GetRightAnchor() const
{
    return rightAnchor;
}

void UIAnchorComponent::SetRightAnchor(float32 anchor)
{
    rightAnchor = anchor;
}

bool UIAnchorComponent::IsTopAnchorEnabled() const
{
    return flags.test(FLAG_TOP_ENABLED);
}

void UIAnchorComponent::SetTopAnchorEnabled(bool enabled)
{
    flags.set(FLAG_TOP_ENABLED, enabled);
}
    
float32 UIAnchorComponent::GetTopAnchor() const
{
    return topAnchor;
}

void UIAnchorComponent::SetTopAnchor(float32 anchor)
{
    topAnchor = anchor;
}

bool UIAnchorComponent::IsVCenterAnchorEnabled() const
{
    return flags.test(FLAG_VCENTER_ENABLED);
}

void UIAnchorComponent::SetVCenterAnchorEnabled(bool enabled)
{
    flags.set(FLAG_VCENTER_ENABLED, enabled);
}
    
float32 UIAnchorComponent::GetVCenterAnchor() const
{
    return vCenterAnchor;
}

void UIAnchorComponent::SetVCenterAnchor(float32 anchor)
{
    vCenterAnchor = anchor;
}

bool UIAnchorComponent::IsBottomAnchorEnabled() const
{
    return flags.test(FLAG_BOTTOM_ENABLED);
}

void UIAnchorComponent::SetBottomAnchorEnabled(bool enabled)
{
    flags.set(FLAG_BOTTOM_ENABLED, enabled);
}
    
float32 UIAnchorComponent::GetBottomAnchor() const
{
    return bottomAnchor;
}

void UIAnchorComponent::SetBottomAnchor(float32 anchor)
{
    bottomAnchor = anchor;
}

bool UIAnchorComponent::IsUseRtl() const
{
    return flags.test(FLAG_USE_RTL);
}

void UIAnchorComponent::SetUseRtl(bool use)
{
    flags.set(FLAG_USE_RTL, use);
}

}
