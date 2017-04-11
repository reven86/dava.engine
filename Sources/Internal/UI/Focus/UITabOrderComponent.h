#ifndef __DAVAENGINE_UI_TAB_ORDER_COMPONENT_H__
#define __DAVAENGINE_UI_TAB_ORDER_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UITabOrderComponent : public UIBaseComponent<UIComponent::TAB_ORDER_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UITabOrderComponent, UIBaseComponent<UIComponent::TAB_ORDER_COMPONENT>);

public:
    enum Direction
    {
        FORWARD = 0,
        BACKWARD,
    };

    UITabOrderComponent();
    UITabOrderComponent(const UITabOrderComponent& src);

protected:
    virtual ~UITabOrderComponent();

private:
    UITabOrderComponent& operator=(const UITabOrderComponent&) = delete;

public:
    UITabOrderComponent* Clone() const override;

    int32 GetTabOrder() const;
    void SetTabOrder(int32 val);

private:
    int32 tabOrder = 0;
};
}


#endif //__DAVAENGINE_UI_TAB_ORDER_COMPONENT_H__
