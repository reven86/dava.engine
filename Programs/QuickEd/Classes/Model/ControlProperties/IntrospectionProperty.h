#ifndef __UI_EDITOR_INTROSPECTION_PROPERTY__
#define __UI_EDITOR_INTROSPECTION_PROPERTY__

#include "ValueProperty.h"

namespace DAVA
{
class UIControl;
class UILayoutSourceRectComponent;
}

class IntrospectionProperty : public ValueProperty
{
public:
    IntrospectionProperty(DAVA::BaseObject* object, DAVA::int32 componentType, const DAVA::String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType);

protected:
    virtual ~IntrospectionProperty();

public:
    static IntrospectionProperty* Create(DAVA::UIControl* control, const DAVA::String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType cloneType);

    void Accept(PropertyVisitor* visitor) override;

    DAVA::uint32 GetFlags() const override;

    ePropertyType GetType() const override;
    const EnumMap* GetEnumMap() const override;

    DAVA::Any GetValue() const override;

    DAVA::BaseObject* GetBaseObject() const
    {
        return object;
    }

    void DisableResetFeature();

protected:
    void ApplyValue(const DAVA::Any& value) override;

protected:
    DAVA::BaseObject* object = nullptr;
    DAVA::Reflection reflection;
    DAVA::int32 flags;

private:
    void SetLayoutSourceRectValue(const DAVA::Any& value);
    DAVA::RefPtr<DAVA::UILayoutSourceRectComponent> sourceRectComponent;
};

#endif //__UI_EDITOR_INTROSPECTION_PROPERTY__
