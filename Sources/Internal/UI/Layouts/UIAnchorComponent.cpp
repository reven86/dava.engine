#include "UIAnchorComponent.h"

namespace DAVA
{
    UIAnchorComponent::UIAnchorComponent()
    : leftAnchorEnabled(false)
    , hCenterAnchorEnabled(false)
    , rightAnchorEnabled(false)
    , topAnchorEnabled(false)
    , vCenterAnchorEnabled(false)
    , bottomAnchorEnabled(false)
    , useRtl(false)
    {
        
    }
    
    UIAnchorComponent::UIAnchorComponent(const UIAnchorComponent &src)
        : leftAnchor(src.leftAnchor)
        , hCenterAnchor(src.hCenterAnchor)
        , rightAnchor(src.rightAnchor)
        , topAnchor(src.topAnchor)
        , vCenterAnchor(src.vCenterAnchor)
        , bottomAnchor(src.bottomAnchor)
        , leftAnchorEnabled(src.leftAnchorEnabled)
        , hCenterAnchorEnabled(src.hCenterAnchorEnabled)
        , rightAnchorEnabled(src.rightAnchorEnabled)
        , topAnchorEnabled(src.topAnchorEnabled)
        , vCenterAnchorEnabled(src.vCenterAnchorEnabled)
        , bottomAnchorEnabled(src.bottomAnchorEnabled)
        , useRtl(src.useRtl)
    {
        
    }
        
    UIAnchorComponent::~UIAnchorComponent()
    {
        
    }
        
    UIAnchorComponent* UIAnchorComponent::Clone()
    {
        return new UIAnchorComponent(*this);
    }
        
    bool UIAnchorComponent::IsLeftAnchorEnabled() const
    {
        return leftAnchorEnabled;
    }

    void UIAnchorComponent::SetLeftAnchorEnabled(bool enabled)
    {
        leftAnchorEnabled = enabled;
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
        return hCenterAnchorEnabled;
    }

    void UIAnchorComponent::SetHCenterAnchorEnabled(bool enabled)
    {
        hCenterAnchorEnabled = enabled;
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
        return rightAnchorEnabled;
    }

    void UIAnchorComponent::SetRightAnchorEnabled(bool enabled)
    {
        rightAnchorEnabled = enabled;
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
        return topAnchorEnabled;
    }

    void UIAnchorComponent::SetTopAnchorEnabled(bool enabled)
    {
        topAnchorEnabled = enabled;
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
        return vCenterAnchorEnabled;
    }

    void UIAnchorComponent::SetVCenterAnchorEnabled(bool enabled)
    {
        vCenterAnchorEnabled = enabled;
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
        return bottomAnchorEnabled;
    }

    void UIAnchorComponent::SetBottomAnchorEnabled(bool enabled)
    {
        bottomAnchorEnabled = enabled;
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
        return useRtl;
    }
    
    void UIAnchorComponent::SetUseRtl(bool use)
    {
        useRtl = use;
    }

}
