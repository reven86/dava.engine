#ifndef __DAVAENGINE_UI_FOCUS_COMPONENT_H__
#define __DAVAENGINE_UI_FOCUS_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "UI/Focus/FocusHelpers.h"

namespace DAVA
{
class UIFocusComponent : public UIBaseComponent<UIComponent::FOCUS_COMPONENT>
{
public:
    UIFocusComponent();
    UIFocusComponent(const UIFocusComponent& src);

protected:
    virtual ~UIFocusComponent();

private:
    UIFocusComponent& operator=(const UIFocusComponent&) = delete;

public:
    UIFocusComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool value);

    bool IsRequestFocus() const;
    void SetRequestFocus(bool value);

private:
    bool enabled = true;
    bool requestFocus = false;

public:
    INTROSPECTION_EXTEND(UIFocusComponent, UIComponent,
                         PROPERTY("enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("requestFocus", "Request Focus", IsRequestFocus, SetRequestFocus, I_SAVE | I_VIEW | I_EDIT));
};
}


#endif //__DAVAENGINE_UI_FOCUS_COMPONENT_H__
