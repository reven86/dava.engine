#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIControl;

/**
 \ingroup layouts
 Component shows to layout system that we want to detach controls subtree from 
 common traversal algorithm. It's usefull in cases when some ui-controls 
 hierarchy works in editor context with their own ui-controls hierarchy.
 */
class UILayoutIsolationComponent : public UIBaseComponent<UIComponent::LAYOUT_ISOLATION_COMPONENT>
{
public:
    UILayoutIsolationComponent();
    UILayoutIsolationComponent(const UILayoutIsolationComponent& src);
    UILayoutIsolationComponent* Clone() const override;

protected:
    virtual ~UILayoutIsolationComponent();
    UILayoutIsolationComponent& operator=(const UILayoutIsolationComponent&) = delete;
};
}
