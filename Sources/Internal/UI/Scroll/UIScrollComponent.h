#pragma once

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIScrollComponent : public UIBaseComponent<UIComponent::SCROLL_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIScrollComponent, UIBaseComponent<UIComponent::SCROLL_COMPONENT>);

public:
    UIScrollComponent();
    UIScrollComponent(const UIScrollComponent& src);

protected:
    virtual ~UIScrollComponent();

    UIScrollComponent& operator=(const UIScrollComponent&) = delete;

public:
    UIScrollComponent* Clone() const override;
};
}
