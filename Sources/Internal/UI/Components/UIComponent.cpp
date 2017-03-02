#include "UIComponent.h"
#include "UI/UIControl.h"

#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Input/UIModalInputComponent.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Focus/UIFocusGroupComponent.h"
#include "UI/Focus/UINavigationComponent.h"
#include "UI/Focus/UITabOrderComponent.h"
#include "UI/Input/UIActionComponent.h"
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Scroll/UIScrollBarDelegateComponent.h"
#include "UI/Sound/UISoundComponent.h"
#include "UI/Sound/UISoundValueFilterComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIComponent)
{
    ReflectionRegistrator<UIComponent>::Begin()
    .Field("type", &UIComponent::GetType, nullptr)
    .End();
}

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
    case BACKGROUND_COMPONENT:
        return new UIControlBackground();

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

    case MODAL_INPUT_COMPONENT:
        return new UIModalInputComponent();

    case FOCUS_COMPONENT:
        return new UIFocusComponent();

    case FOCUS_GROUP_COMPONENT:
        return new UIFocusGroupComponent();

    case NAVIGATION_COMPONENT:
        return new UINavigationComponent();

    case TAB_ORDER_COMPONENT:
        return new UITabOrderComponent();

    case ACTION_COMPONENT:
        return new UIActionComponent();

    case ACTION_BINDING_COMPONENT:
        return new UIActionBindingComponent();

    case SCROLL_BAR_DELEGATE_COMPONENT:
        return new UIScrollBarDelegateComponent();

    case SOUND_COMPONENT:
        return new UISoundComponent();

    case SOUND_VALUE_FILTER_COMPONENT:
        return new UISoundValueFilterComponent();

    default:
        DVASSERT(false);
        return nullptr;
    }
}

RefPtr<UIComponent> UIComponent::SafeCreateByType(uint32 componentType)
{
    return RefPtr<UIComponent>(CreateByType(componentType));
}

bool UIComponent::IsMultiple(uint32 componentType)
{
    return false;
}

RefPtr<UIComponent> UIComponent::SafeClone() const
{
    return RefPtr<UIComponent>(Clone());
}
}
