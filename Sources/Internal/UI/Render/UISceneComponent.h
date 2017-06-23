#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISceneComponent final : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UISceneComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UISceneComponent);

public:
    UISceneComponent();
    UISceneComponent(const UISceneComponent& src);

    UISceneComponent* Clone() const override;

    UISceneComponent& operator=(const UISceneComponent&) = delete;

protected:
    ~UISceneComponent() override;
};
}
