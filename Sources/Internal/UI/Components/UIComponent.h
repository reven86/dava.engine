#pragma once

#include "Math/Math2D.h"
#include "Base/BaseObject.h"
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
    virtual ~UIComponent();

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

template <class T>
class UIBaseComponent : public UIComponent
{
public:
    int32 GetRuntimeType() const override
    {
        return runtimeType;
    }

    static int32 GetStaticRuntimeType()
    {
        return runtimeType;
    }

    const Type* GetType() const override
    {
        return reflectionType;
    }

private:
    static int32 runtimeType;
    static const Type* reflectionType;
    friend class ComponentManager;
};

template <class T>
int32 UIBaseComponent<T>::runtimeType = -1;

template <class T>
const Type* UIBaseComponent<T>::reflectionType = Type::Instance<T>();
}
