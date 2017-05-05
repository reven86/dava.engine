#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISceneComponent : public UIBaseComponent<UIComponent::SCENE_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UISceneComponent, UIBaseComponent<UIComponent::SCENE_COMPONENT>);

public:
    UISceneComponent();
    UISceneComponent(const UISceneComponent& src);

    UISceneComponent* Clone() const override;

private:
    ~UISceneComponent() override;

    UISceneComponent& operator=(const UISceneComponent&) = delete;
};
}
