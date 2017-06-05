#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIClipContentComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIDebugRenderComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIClipContentComponent);

public:
    UIClipContentComponent();
    UIClipContentComponent(const UIClipContentComponent& src);

    UIClipContentComponent* Clone() const override;

private:
    ~UIClipContentComponent() override = default;
    UIClipContentComponent& operator=(const UIClipContentComponent&) = delete;
};
}
