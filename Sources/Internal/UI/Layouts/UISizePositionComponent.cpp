#include "UISizePositionComponent.h"

#include "UI/UIControl.h"
#include "Math/Vector.h"

namespace DAVA
{
    UISizePositionComponent::UISizePositionComponent()
    {
    }
    
    UISizePositionComponent::UISizePositionComponent(const UISizePositionComponent& src)
        : postion(src.postion)
        , size(src.size)
    {
    }
    
    UISizePositionComponent::~UISizePositionComponent()
    {
    }
    
    UISizePositionComponent* UISizePositionComponent::Clone() const
    {
        return new UISizePositionComponent(*this);
    }
    
    const Vector2 &UISizePositionComponent::GetPosition() const
    {
        return postion;
    }
    
    void UISizePositionComponent::SetPosition(const Vector2 &position_)
    {
        postion = position_;
        SetLayoutDirty();
    }
    
    const Vector2 &UISizePositionComponent::GetSize() const
    {
        return size;
    }
    
    void UISizePositionComponent::SetSize(const Vector2 &size_)
    {
        size = size_;
        SetLayoutDirty();
    }
    
    void UISizePositionComponent::SetLayoutDirty()
    {
        if (GetControl() != nullptr)
        {
            GetControl()->SetLayoutDirty();
        }
    }
}
