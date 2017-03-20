#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIControl;

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
