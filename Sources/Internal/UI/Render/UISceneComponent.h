#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISceneComponent final : public UIBaseComponent<UISceneComponent>
{
    DAVA_VIRTUAL_REFLECTION(UISceneComponent, UIBaseComponent<UISceneComponent>);

public:
    UISceneComponent();
    UISceneComponent(const UISceneComponent& src);

    UISceneComponent* Clone() const override;

    UISceneComponent& operator=(const UISceneComponent&) = delete;

protected:
    ~UISceneComponent() override;
};
}
