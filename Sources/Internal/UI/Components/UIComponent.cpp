#include "UIComponent.h"
#include "UI/UIControl.h"

#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Focus/UIFocusComponent.h"

namespace DAVA
{
UIComponent::UIComponent()
    : control(nullptr)
{
}

UIComponent::UIComponent(const UIComponent& src)
    : control(nullptr)
{
}

UIComponent::~UIComponent()
{
}

UIComponent& UIComponent::operator=(const UIComponent& src)
{
    return *this;
}

UIComponent* UIComponent::CreateByType(uint32 componentType)
{
    switch (componentType)
    {
    case LINEAR_LAYOUT_COMPONENT:
        return new UILinearLayoutComponent();

    case FLOW_LAYOUT_COMPONENT:
        return new UIFlowLayoutComponent();

    case FLOW_LAYOUT_HINT_COMPONENT:
        return new UIFlowLayoutHintComponent();

    case IGNORE_LAYOUT_COMPONENT:
        return new UIIgnoreLayoutComponent();

    case SIZE_POLICY_COMPONENT:
        return new UISizePolicyComponent();

    case ANCHOR_COMPONENT:
        return new UIAnchorComponent();

    case FOCUS_COMPONENT:
        return new UIFocusComponent();

    default:
        DVASSERT(false);
        return nullptr;
    }
}

bool UIComponent::IsMultiple(uint32 componentType)
{
    return false;
}
}
