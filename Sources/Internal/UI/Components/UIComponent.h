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
    UIComponent();
    UIComponent(const UIComponent& src);
    virtual ~UIComponent();

    UIComponent& operator=(const UIComponent& src);

    static UIComponent* CreateByType(const Type* componentType);
    static RefPtr<UIComponent> SafeCreateByType(const Type* componentType);
    static bool IsMultiple(const Type* componentType);

    void SetControl(UIControl* _control);
    UIControl* GetControl() const;

    virtual UIComponent* Clone() const = 0;

    RefPtr<UIComponent> SafeClone() const;

    const Type* GetType() const;

private:
    UIControl* control;
    const Type* type;
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
