#ifndef __DAVAENGINE_UI_COMPONENT_H__
#define __DAVAENGINE_UI_COMPONENT_H__

#include "Base/BaseObject.h"

namespace DAVA
{
class UIControl;

class UIComponent : public BaseObject
{
public:
    enum eType
    {
        LINEAR_LAYOUT_COMPONENT,
        FLOW_LAYOUT_COMPONENT,
        FLOW_LAYOUT_HINT_COMPONENT,
        IGNORE_LAYOUT_COMPONENT,
        SIZE_POLICY_COMPONENT,
        ANCHOR_COMPONENT,
        FOCUS_COMPONENT,

        COMPONENT_COUNT
    };

public:
    UIComponent();
    UIComponent(const UIComponent& src);
    virtual ~UIComponent();

    UIComponent& operator=(const UIComponent& src);

    static UIComponent* CreateByType(uint32 componentType);
    static bool IsMultiple(uint32 componentType);

    virtual uint32 GetType() const = 0;

    void SetControl(UIControl* _control);
    UIControl* GetControl() const;

    virtual UIComponent* Clone() const = 0;

private:
    UIControl* control;
};

template <uint32 TYPE>
class UIBaseComponent : public UIComponent
{
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
