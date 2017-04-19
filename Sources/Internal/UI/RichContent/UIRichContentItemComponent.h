#pragma once

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
/** Marks control as one of RichContent child control. */
class UIRichContentItemComponent : public UIBaseComponent<UIComponent::RICH_CONTENT_ITEM_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIRichContentItemComponent, UIBaseComponent<UIComponent::RICH_CONTENT_ITEM_COMPONENT>);

public:
    /** Default constructor. */
    UIRichContentItemComponent() = default;
    /** Copy constructor. */
    UIRichContentItemComponent(const UIRichContentItemComponent& src);
    /** Removed operator overloading. */
    UIRichContentItemComponent& operator=(const UIRichContentItemComponent&) = delete;

    UIComponent* Clone() const override;

protected:
    ~UIRichContentItemComponent() override = default;
};
}