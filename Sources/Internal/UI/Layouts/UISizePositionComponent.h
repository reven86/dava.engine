#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIControl;
    
    class UISizePositionComponent : public UIBaseComponent<UIComponent::SIZE_POSITION_COMPONENT>
    {
    public:
        UISizePositionComponent();
        UISizePositionComponent(const UISizePositionComponent& src);
        UISizePositionComponent* Clone() const override;

        const Vector2 &GetPosition() const;
        void SetPosition(const Vector2 &position);
        
        const Vector2 &GetSize() const;
        void SetSize(const Vector2 &size);
        
    protected:
        virtual ~UISizePositionComponent();
        UISizePositionComponent& operator=(const UISizePositionComponent&) = delete;
        
    private:
        void SetLayoutDirty();

        Vector2 postion;
        Vector2 size;
    };
}
