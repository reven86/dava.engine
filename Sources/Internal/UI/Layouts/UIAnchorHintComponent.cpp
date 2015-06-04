#include "UIAnchorHintComponent.h"

namespace DAVA
{
    UIAnchorHintComponent::UIAnchorHintComponent()
    {
        
    }
    
    UIAnchorHintComponent::UIAnchorHintComponent(const UIAnchorHintComponent &src)
        : anchors(src.anchors)
        , leftAnchor(src.leftAnchor)
        , hCenterAnchor(src.hCenterAnchor)
        , rightAnchor(src.rightAnchor)
        , topAnchor(src.topAnchor)
        , vCenterAnchor(src.vCenterAnchor)
        , bottomAnchor(src.bottomAnchor)
    {
        
    }
        
    UIAnchorHintComponent::~UIAnchorHintComponent()
    {
        
    }
        
    UIAnchorHintComponent* UIAnchorHintComponent::Clone()
    {
        return new UIAnchorHintComponent(*this);
    }
        
    bool UIAnchorHintComponent::IsLeftAnchorEnabled() const
    {
        return (anchors & ANCHOR_LEFT) != 0;
    }

    void UIAnchorHintComponent::SetLeftAnchorEnabled(bool enabled)
    {
        if (enabled)
            anchors |= ANCHOR_LEFT;
        else
            anchors &= ~ANCHOR_LEFT;
    }
        
    float32 UIAnchorHintComponent::GetLeftAnchor() const
    {
        return leftAnchor;
    }

    void UIAnchorHintComponent::SetLeftAnchor(float32 anchor)
    {
        leftAnchor = anchor;
    }

    bool UIAnchorHintComponent::IsHCenterAnchorEnabled() const
    {
        return (anchors & ANCHOR_HCENTER) != 0;
    }

    void UIAnchorHintComponent::SetHCenterAnchorEnabled(bool enabled)
    {
        if (enabled)
            anchors |= ANCHOR_HCENTER;
        else
            anchors &= ~ANCHOR_HCENTER;
    }
        
    float32 UIAnchorHintComponent::GetHCenterAnchor() const
    {
        return hCenterAnchor;
    }

    void UIAnchorHintComponent::SetHCenterAnchor(float32 anchor)
    {
        hCenterAnchor = anchor;
    }

    bool UIAnchorHintComponent::IsRightAnchorEnabled() const
    {
        return (anchors & ANCHOR_RIGHT) != 0;
    }

    void UIAnchorHintComponent::SetRightAnchorEnabled(bool enabled)
    {
        if (enabled)
            anchors |= ANCHOR_RIGHT;
        else
            anchors &= ~ANCHOR_RIGHT;
    }
        
    float32 UIAnchorHintComponent::GetRightAnchor() const
    {
        return rightAnchor;
    }

    void UIAnchorHintComponent::SetRightAnchor(float32 anchor)
    {
        rightAnchor = anchor;
    }

    bool UIAnchorHintComponent::IsTopAnchorEnabled() const
    {
        return (anchors & ANCHOR_TOP) != 0;
    }

    void UIAnchorHintComponent::SetTopAnchorEnabled(bool enabled)
    {
        if (enabled)
            anchors |= ANCHOR_TOP;
        else
            anchors &= ~ANCHOR_TOP;
    }
        
    float32 UIAnchorHintComponent::GetTopAnchor() const
    {
        return topAnchor;
    }

    void UIAnchorHintComponent::SetTopAnchor(float32 anchor)
    {
        topAnchor = anchor;
    }

    bool UIAnchorHintComponent::IsVCenterAnchorEnabled() const
    {
        return (anchors & ANCHOR_VCENTER) != 0;
    }

    void UIAnchorHintComponent::SetVCenterAnchorEnabled(bool enabled)
    {
        if (enabled)
            anchors |= ANCHOR_VCENTER;
        else
            anchors &= ~ANCHOR_VCENTER;
    }
        
    float32 UIAnchorHintComponent::GetVCenterAnchor() const
    {
        return vCenterAnchor;
    }

    void UIAnchorHintComponent::SetVCenterAnchor(float32 anchor)
    {
        vCenterAnchor = anchor;
    }

    bool UIAnchorHintComponent::IsBottomAnchorEnabled() const
    {
        return (anchors & ANCHOR_BOTTOM) != 0;
    }

    void UIAnchorHintComponent::SetBottomAnchorEnabled(bool enabled)
    {
        if (enabled)
            anchors |= ANCHOR_BOTTOM;
        else
            anchors &= ~ANCHOR_BOTTOM;
    }
        
    float32 UIAnchorHintComponent::GetBottomAnchor() const
    {
        return bottomAnchor;
    }

    void UIAnchorHintComponent::SetBottomAnchor(float32 anchor)
    {
        bottomAnchor = anchor;
    }
        
}
