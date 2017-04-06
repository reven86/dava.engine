#ifndef __DAVAENGINE_UI_ACTION_COMPONENT_H__
#define __DAVAENGINE_UI_ACTION_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIControl;

class UIActionComponent : public UIBaseComponent<UIComponent::ACTION_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIActionComponent, UIBaseComponent<UIComponent::ACTION_COMPONENT>);

public:
    UIActionComponent();
    UIActionComponent(const UIActionComponent& src);

protected:
    virtual ~UIActionComponent();

private:
    UIActionComponent& operator=(const UIActionComponent&) = delete;

public:
    UIActionComponent* Clone() const override;

    const FastName& GetAction() const;
    void SetAction(const FastName& value);

private:
    String GetActionAsString() const;
    void SetActionFromString(const String& value);

    FastName action;
};
}


#endif //__DAVAENGINE_UI_ACTION_COMPONENT_H__
