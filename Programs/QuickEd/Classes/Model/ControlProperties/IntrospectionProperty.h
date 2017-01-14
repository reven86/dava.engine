#ifndef __UI_EDITOR_INTROSPECTION_PROPERTY__
#define __UI_EDITOR_INTROSPECTION_PROPERTY__

#include "ValueProperty.h"

namespace DAVA
{
class UIControl;
}

class IntrospectionProperty : public ValueProperty
{
public:
    IntrospectionProperty(DAVA::BaseObject* object, const DAVA::ReflectedStructure::Field* field, const IntrospectionProperty* sourceProperty, eCloneType copyType);

protected:
    virtual ~IntrospectionProperty();

public:
    static IntrospectionProperty* Create(DAVA::UIControl* control, const DAVA::ReflectedStructure::Field* field, const IntrospectionProperty* sourceProperty, eCloneType cloneType);

    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    DAVA::uint32 GetFlags() const override;

    DAVA::Any GetValue() const override;

    DAVA::BaseObject* GetBaseObject() const
    {
        return object;
    }

    bool IsSameField(const DAVA::ReflectedStructure::Field* field_) const override
    {
        return (field == field_);
    }

    const DAVA::ReflectedStructure::Field* GetField() const;

    void DisableResetFeature();

protected:
    void ApplyValue(const DAVA::Any& value) override;

protected:
    DAVA::BaseObject* object = nullptr;
    const DAVA::ReflectedStructure::Field* field = nullptr;
    DAVA::int32 flags;

private:
    DAVA::Any sourceValue;
};

#endif //__UI_EDITOR_INTROSPECTION_PROPERTY__
