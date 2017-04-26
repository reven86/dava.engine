#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIControl;

/**
     \ingroup layouts
     Layout system uses this component to setup initial control position and size, because real control position and size
     can be changed by layout system in previous invocation. This is usefull for QuickEd which uses this component for saving
     position and size entered by user.
     */
class UILayoutSourceRectComponent : public UIBaseComponent<UIComponent::LAYOUT_SOURCE_RECT_COMPONENT>
{
public:
    UILayoutSourceRectComponent();
    UILayoutSourceRectComponent(const UILayoutSourceRectComponent& src);
    UILayoutSourceRectComponent* Clone() const override;

    const Vector2& GetPosition() const;
    void SetPosition(const Vector2& position);

    const Vector2& GetSize() const;
    void SetSize(const Vector2& size);

protected:
    virtual ~UILayoutSourceRectComponent();
    UILayoutSourceRectComponent& operator=(const UILayoutSourceRectComponent&) = delete;

private:
    void SetLayoutDirty();

    Vector2 postion;
    Vector2 size;
};
}
