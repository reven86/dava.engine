#ifndef __DAVAENGINE_UI_COMPONENT_H__
#define __DAVAENGINE_UI_COMPONENT_H__

#include "Entity/Component.h"

namespace DAVA
{
class UIControl;

class UIComponent : public Component
{
protected:
    virtual ~UIComponent() {}

public:
    UIComponent() : control(NULL) {}
    inline void SetControl(UIControl* control);
    inline UIControl* GetControl() const;

    virtual Component* Clone(Entity* toEntity);
    virtual Component* Clone(UIControl * toControl) = 0;

private:
    UIControl* control;

};

inline void UIComponent::SetControl(UIControl* control)
{
    control = control;
}

inline UIControl* UIComponent::GetControl() const
{
    return control;
}

inline Component* UIComponent::Clone(Entity* toEntity)
{
    // Empty stub
    return NULL;
}

}


#endif //__DAVAENGINE_UI_COMPONENT_H__