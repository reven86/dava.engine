#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIClipContentComponent : public UIBaseComponent<UIComponent::CLIP_CONTENT_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIDebugRenderComponent, UIBaseComponent<UIComponent::CLIP_CONTENT_COMPONENT>);

public:
    UIClipContentComponent();
    UIClipContentComponent(const UIClipContentComponent& src);

    UIClipContentComponent* Clone() const override;

private:
    ~UIClipContentComponent() override = default;
    UIClipContentComponent& operator=(const UIClipContentComponent&) = delete;
};
}
