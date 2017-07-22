#pragma once

#include "Base/BaseObject.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIControl;

class UIComponent : public BaseObject
{
    DAVA_VIRTUAL_REFLECTION(UIComponent, BaseObject);

public:
    UIComponent();
    UIComponent(const UIComponent& src);

    UIComponent& operator=(const UIComponent& src);

    static UIComponent* CreateByType(const Type* componentType);
    static RefPtr<UIComponent> SafeCreateByType(const Type* componentType);
    static bool IsMultiple(const Type* componentType);

    void SetControl(UIControl* _control);
    UIControl* GetControl() const;

    virtual UIComponent* Clone() const = 0;

    RefPtr<UIComponent> SafeClone() const;

    virtual int32 GetRuntimeType() const = 0;

    virtual const Type* GetType() const = 0;

protected:
    virtual ~UIComponent();

private:
    UIControl* control;
};

inline void UIComponent::SetControl(UIControl* _control)
{
    control = _control;
}

inline UIControl* UIComponent::GetControl() const
{
    return control;
}

#define IMPLEMENT_UI_COMPONENT(TYPE) \
const Type* GetType() const override { return Type::Instance<TYPE>(); }; \
int32 GetRuntimeType() const override \
{ \
    static int32 runtimeType = GetEngineContext()->componentManager->GetRuntimeType(GetType()); \
    return runtimeType; \
}
}
