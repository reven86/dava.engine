#ifndef __DAVAENGINE_UI_MODAL_INPUT_COMPONENT_H__
#define __DAVAENGINE_UI_MODAL_INPUT_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIControl;

class UIModalInputComponent : public UIBaseComponent<UIComponent::MODAL_INPUT_COMPONENT>
{
public:
    UIModalInputComponent();
    UIModalInputComponent(const UIModalInputComponent& src);

protected:
    virtual ~UIModalInputComponent();

private:
    UIModalInputComponent& operator=(const UIModalInputComponent&) = delete;

public:
    UIModalInputComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enabled_);

private:
    bool enabled = true;

public:
    INTROSPECTION_EXTEND(UIModalInputComponent, UIComponent,
                         PROPERTY("enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT));
};
}



#endif // __DAVAENGINE_UI_MODAL_INPUT_COMPONENT_H__
