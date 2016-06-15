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
    IntrospectionProperty(DAVA::BaseObject* object, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType copyType);

protected:
    virtual ~IntrospectionProperty();

public:
    static IntrospectionProperty* Create(DAVA::UIControl* control, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType cloneType);

    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    DAVA::uint32 GetFlags() const override;

    DAVA::VariantType GetValue() const override;

    DAVA::BaseObject* GetBaseObject() const
    {
        return object;
    }

    bool IsSameMember(const DAVA::InspMember* aMember) const override
    {
        return (aMember == member);
    }

    const DAVA::InspMember* GetMember() const;

    void DisableResetFeature();

protected:
    void ApplyValue(const DAVA::VariantType& value) override;

protected:
    DAVA::BaseObject* object;
    const DAVA::InspMember* member;
    DAVA::int32 flags;

private:
    DAVA::VariantType sourceValue;
};

#endif //__UI_EDITOR_INTROSPECTION_PROPERTY__
