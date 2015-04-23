#ifndef __UI_EDITOR_VALUE_PROPERTY__
#define __UI_EDITOR_VALUE_PROPERTY__

#include "AbstractProperty.h"

class SubValueProperty;

class ValueProperty : public AbstractProperty
{
public:
    ValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType);
    
protected:
    virtual ~ValueProperty();
    
public:
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;

    virtual bool HasChanges() const override;
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual DAVA::String GetName() const override;
    virtual ePropertyType GetType() const override;
    virtual eEditFrags GetEditFlag() const  override{ return EF_CAN_RESET; };

    virtual DAVA::VariantType GetValue() const override;
    virtual void SetValue(const DAVA::VariantType &newValue) override;
    virtual DAVA::VariantType GetDefaultValue() const override;
    virtual void SetDefaultValue(const DAVA::VariantType &newValue) override;
    virtual void ResetValue() override;
    virtual bool IsReplaced() const override;
    
    virtual DAVA::String GetSubValueName(int index) const;
    virtual DAVA::VariantType GetSubValue(int index) const;
    virtual void SetSubValue(int index, const DAVA::VariantType &newValue);
    virtual DAVA::VariantType GetDefaultSubValue(int index) const;
    virtual void SetDefaultSubValue(int index, const DAVA::VariantType &newValue);
    
    virtual DAVA::BaseObject *GetBaseObject() const {
        return object;
    }
    
    virtual const DAVA::InspMember *GetMember() const {
        return member;
    }
    
    virtual const EnumMap *GetEnumMap() const override;

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);
    
private:
    DAVA::VariantType ChangeValueComponent(const DAVA::VariantType &value, const DAVA::VariantType &component, DAVA::int32 index) const;
    DAVA::VariantType GetValueComponent(const DAVA::VariantType &value, DAVA::int32 index) const;
    
private:
    DAVA::BaseObject *object;
    const DAVA::InspMember *member;
    bool replaced;
    DAVA::VariantType defaultValue;
    DAVA::Vector<SubValueProperty*> children;
};

#endif //__UI_EDITOR_VALUE_PROPERTY__
