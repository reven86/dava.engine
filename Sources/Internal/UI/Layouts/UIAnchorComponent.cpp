#include "UIAnchorComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UIAnchorComponent::UIAnchorComponent()
{
    SetEnabled(true);
}

UIAnchorComponent::UIAnchorComponent(const UIAnchorComponent& src)
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

bool UIAnchorComponent::IsEnabled() const
{
    return flags.test(FLAG_ENABLED);
}

void UIAnchorComponent::SetEnabled(bool enabled)
{
    if (flags.test(FLAG_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_ENABLED, enabled);
    SetLayoutDirty();
}

bool UIAnchorComponent::IsLeftAnchorEnabled() const
{
    return flags.test(FLAG_LEFT_ENABLED);
}

void UIAnchorComponent::SetLeftAnchorEnabled(bool enabled)
{
    if (flags.test(FLAG_LEFT_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_LEFT_ENABLED, enabled);
    SetLayoutDirty();
}

float32 UIAnchorComponent::GetLeftAnchor() const
{
    return leftAnchor;
}

void UIAnchorComponent::SetLeftAnchor(float32 anchor)
{
    if (leftAnchor == anchor)
    {
        return;
    }

    leftAnchor = anchor;
    SetLayoutDirty();
}

bool UIAnchorComponent::IsHCenterAnchorEnabled() const
{
    return flags.test(FLAG_HCENTER_ENABLED);
}

void UIAnchorComponent::SetHCenterAnchorEnabled(bool enabled)
{
    if (flags.test(FLAG_HCENTER_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_HCENTER_ENABLED, enabled);
    SetLayoutDirty();
}

float32 UIAnchorComponent::GetHCenterAnchor() const
{
    return hCenterAnchor;
}

void UIAnchorComponent::SetHCenterAnchor(float32 anchor)
{
    if (hCenterAnchor == anchor)
    {
        return;
    }
    hCenterAnchor = anchor;
    SetLayoutDirty();
}

bool UIAnchorComponent::IsRightAnchorEnabled() const
{
    return flags.test(FLAG_RIGHT_ENABLED);
}

void UIAnchorComponent::SetRightAnchorEnabled(bool enabled)
{
    if (flags.test(FLAG_RIGHT_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_RIGHT_ENABLED, enabled);
    SetLayoutDirty();
}

float32 UIAnchorComponent::GetRightAnchor() const
{
    return rightAnchor;
}

void UIAnchorComponent::SetRightAnchor(float32 anchor)
{
    if (rightAnchor == anchor)
    {
        return;
    }

    rightAnchor = anchor;
    SetLayoutDirty();
}

bool UIAnchorComponent::IsTopAnchorEnabled() const
{
    return flags.test(FLAG_TOP_ENABLED);
}

void UIAnchorComponent::SetTopAnchorEnabled(bool enabled)
{
    if (flags.test(FLAG_TOP_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_TOP_ENABLED, enabled);
    SetLayoutDirty();
}

float32 UIAnchorComponent::GetTopAnchor() const
{
    return topAnchor;
}

void UIAnchorComponent::SetTopAnchor(float32 anchor)
{
    if (topAnchor == anchor)
    {
        return;
    }

    topAnchor = anchor;
    SetLayoutDirty();
}

bool UIAnchorComponent::IsVCenterAnchorEnabled() const
{
    return flags.test(FLAG_VCENTER_ENABLED);
}

void UIAnchorComponent::SetVCenterAnchorEnabled(bool enabled)
{
    if (flags.test(FLAG_VCENTER_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_VCENTER_ENABLED, enabled);
    SetLayoutDirty();
}

float32 UIAnchorComponent::GetVCenterAnchor() const
{
    return vCenterAnchor;
}

void UIAnchorComponent::SetVCenterAnchor(float32 anchor)
{
    if (vCenterAnchor == anchor)
    {
        return;
    }

    vCenterAnchor = anchor;
    SetLayoutDirty();
}

bool UIAnchorComponent::IsBottomAnchorEnabled() const
{
    return flags.test(FLAG_BOTTOM_ENABLED);
}

void UIAnchorComponent::SetBottomAnchorEnabled(bool enabled)
{
    if (flags.test(FLAG_BOTTOM_ENABLED) == enabled)
    {
        return;
    }

    flags.set(FLAG_BOTTOM_ENABLED, enabled);
    SetLayoutDirty();
}

float32 UIAnchorComponent::GetBottomAnchor() const
{
    return bottomAnchor;
}

void UIAnchorComponent::SetBottomAnchor(float32 anchor)
{
    if (bottomAnchor == anchor)
    {
        return;
    }

    bottomAnchor = anchor;
    SetLayoutDirty();
}

bool UIAnchorComponent::IsUseRtl() const
{
    return flags.test(FLAG_USE_RTL);
}

void UIAnchorComponent::SetUseRtl(bool use)
{
    if (flags.test(FLAG_USE_RTL) == use)
    {
        return;
    }

    flags.set(FLAG_USE_RTL, use);
    SetLayoutDirty();
}

void UIAnchorComponent::SetLayoutDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
