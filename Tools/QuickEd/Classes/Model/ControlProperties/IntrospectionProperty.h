#ifndef __UI_EDITOR_INTROSPECTION_PROPERTY__
#define __UI_EDITOR_INTROSPECTION_PROPERTY__

#include "ValueProperty.h"

class SubValueProperty;

class IntrospectionProperty : public ValueProperty
{
public:
    IntrospectionProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, const IntrospectionProperty *sourceProperty, eCloneType copyType);

protected:
    virtual ~IntrospectionProperty();
    
public:
    virtual void Refresh();
    virtual AbstractProperty *FindPropertyByPrototype(AbstractProperty *prototype) override;
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual ePropertyType GetType() const override;
    virtual DAVA::uint32 GetEditFlag() const  override;

    virtual DAVA::VariantType GetValue() const override;

    virtual DAVA::BaseObject *GetBaseObject() const
    {
        return object;
    }
    
    virtual const EnumMap *GetEnumMap() const override;

    virtual bool IsSameMember(const DAVA::InspMember *aMember) const override
    {
        return (aMember == member);
    }

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);

protected:
    DAVA::BaseObject *object;
    const IntrospectionProperty *prototypeProperty;
    const DAVA::InspMember *member;
};

#endif //__UI_EDITOR_INTROSPECTION_PROPERTY__
