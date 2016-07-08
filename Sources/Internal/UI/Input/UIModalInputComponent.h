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

public:
    INTROSPECTION_EXTEND(UIModalInputComponent, UIComponent,
                         nullptr);
};
}



#endif // __DAVAENGINE_UI_MODAL_INPUT_COMPONENT_H__
