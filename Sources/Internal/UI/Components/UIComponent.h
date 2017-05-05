#ifndef __DAVAENGINE_UI_COMPONENT_H__
#define __DAVAENGINE_UI_COMPONENT_H__

#include "Base/BaseObject.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
class UIControl;

class UIComponent : public BaseObject
{
    DAVA_VIRTUAL_REFLECTION(UIComponent, BaseObject);

public:
    enum eType
    {
        LINEAR_LAYOUT_COMPONENT,
        FLOW_LAYOUT_COMPONENT,
        FLOW_LAYOUT_HINT_COMPONENT,
        IGNORE_LAYOUT_COMPONENT,
        SIZE_POLICY_COMPONENT,
        ANCHOR_COMPONENT,
        LAYOUT_SOURCE_RECT_COMPONENT,
        LAYOUT_ISOLATION_COMPONENT,
        BACKGROUND_COMPONENT,
        MODAL_INPUT_COMPONENT,
        FOCUS_COMPONENT,
        FOCUS_GROUP_COMPONENT,
        NAVIGATION_COMPONENT,
        TAB_ORDER_COMPONENT,
        ACTION_COMPONENT,
        ACTION_BINDING_COMPONENT,
        SCROLL_BAR_DELEGATE_COMPONENT,
        SCROLL_COMPONENT,
        SOUND_COMPONENT,
        SOUND_VALUE_FILTER_COMPONENT,
        UPDATE_COMPONENT,
        CUSTOM_UPDATE_DELTA_COMPONENT,
        SCENE_COMPONENT,

        COMPONENT_COUNT
    };

public:
    UIComponent();
    UIComponent(const UIComponent& src);

    UIComponent& operator=(const UIComponent& src);

    static UIComponent* CreateByType(uint32 componentType);
    static RefPtr<UIComponent> SafeCreateByType(uint32 componentType);
    static bool IsMultiple(uint32 componentType);

    virtual uint32 GetType() const = 0;

    void SetControl(UIControl* _control);
    UIControl* GetControl() const;

    virtual UIComponent* Clone() const = 0;

    RefPtr<UIComponent> SafeClone() const;

protected:
    virtual ~UIComponent();

private:
    UIControl* control;
};

template <uint32 TYPE>
class UIBaseComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(UIBaseComponent<TYPE>, UIComponent)
    {
        ReflectionRegistrator<UIBaseComponent<TYPE>>::Begin()
        .End();
    }

public:
    static const uint32 C_TYPE = TYPE;

    uint32 GetType() const override
    {
        return TYPE;
    }
};

inline void UIComponent::SetControl(UIControl* _control)
{
    control = _control;
}

inline UIControl* UIComponent::GetControl() const
{
    return control;
}
}


#endif //__DAVAENGINE_UI_COMPONENT_H__
