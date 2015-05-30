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
    void Refresh();
    AbstractProperty *FindPropertyByPrototype(AbstractProperty *prototype) override;
    void Accept(PropertyVisitor *visitor) override;
    

    ePropertyType GetType() const override;
    DAVA::uint32 GetFlags() const  override;

    DAVA::VariantType GetValue() const override;

    DAVA::BaseObject *GetBaseObject() const
    {
        return object;
    }
    
    const EnumMap *GetEnumMap() const override;

    bool IsSameMember(const DAVA::InspMember *aMember) const override
    {
        return (aMember == member);
    }
    
    const DAVA::InspMember *GetMember() const;

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);

protected:
    DAVA::BaseObject *object;
    const IntrospectionProperty *prototypeProperty;
    const DAVA::InspMember *member;
};

#endif //__UI_EDITOR_INTROSPECTION_PROPERTY__
