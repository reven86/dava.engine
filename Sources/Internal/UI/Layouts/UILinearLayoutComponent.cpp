#include "UILinearLayoutComponent.h"

namespace DAVA
{
    UILinearLayoutComponent::UILinearLayoutComponent()
    {
        
    }
    
    UILinearLayoutComponent::UILinearLayoutComponent(const UILinearLayoutComponent &src)
    : orientation(src.orientation)
    {
        
    }
    
    UILinearLayoutComponent::~UILinearLayoutComponent()
    {
        
    }
    
    UILinearLayoutComponent* UILinearLayoutComponent::Clone()
    {
        return new UILinearLayoutComponent(*this);
    }
    
    UILinearLayoutComponent::eOrientation UILinearLayoutComponent::GetOrientation() const
    {
        return orientation;
    }
    
    void UILinearLayoutComponent::SetOrientation(eOrientation newOrientation)
    {
        orientation = newOrientation;
    }
    
    int32 UILinearLayoutComponent::GetOrientationAsInt() const
    {
        return static_cast<int32>(GetOrientation());
    }
    
    void UILinearLayoutComponent::SetOrientationFromInt(int32 orientation)
    {
        SetOrientation(static_cast<eOrientation>(orientation));
    }
    
    
    float32 UILinearLayoutComponent::GetPadding() const
    {
        return padding;
    }
    
    void UILinearLayoutComponent::SetPadding(float32 newPadding)
    {
        padding = newPadding;
    }
    
    float32 UILinearLayoutComponent::GetSpacing() const
    {
        return spacing;
    }
    
    void UILinearLayoutComponent::SetSpacing(float32 newSpacing)
    {
        spacing = newSpacing;
    }

    bool UILinearLayoutComponent::IsUseRtl() const
    {
        return useRtl;
    }
    
    void UILinearLayoutComponent::SetUseRtl(bool use)
    {
        useRtl = use;
    }

}
