#ifndef __UI_EDITOR_VALUE_PROPERTY__
#define __UI_EDITOR_VALUE_PROPERTY__

#include "AbstractProperty.h"

class SubValueProperty;

class ValueProperty : public AbstractProperty
{
public:
    ValueProperty(const DAVA::String &propName);

protected:
    virtual ~ValueProperty();
    
public:
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;

    virtual bool HasChanges() const override;
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual const DAVA::String &GetName() const override;
    virtual ePropertyType GetType() const override;

    virtual DAVA::VariantType GetValue() const override;
    virtual void SetValue(const DAVA::VariantType &newValue) override;
    virtual DAVA::VariantType GetDefaultValue() const override;
    virtual void SetDefaultValue(const DAVA::VariantType &newValue) override;
    virtual void ResetValue() override;
    virtual bool IsReplaced() const override;
    
    virtual DAVA::VariantType GetSubValue(int index) const;
    virtual void SetSubValue(int index, const DAVA::VariantType &newValue);
    virtual DAVA::VariantType GetDefaultSubValue(int index) const;
    virtual void SetDefaultSubValue(int index, const DAVA::VariantType &newValue);

    virtual const EnumMap *GetEnumMap() const override;
    DAVA_DEPRECATED(virtual bool IsSameMember(const DAVA::InspMember *member) const)
    {
        return false;
    }

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);
    
private:
    DAVA::VariantType ChangeValueComponent(const DAVA::VariantType &value, const DAVA::VariantType &component, DAVA::int32 index) const;
    DAVA::VariantType GetValueComponent(const DAVA::VariantType &value, DAVA::int32 index) const;
    
protected:
    DAVA::String name;
    bool replaced;
    DAVA::VariantType defaultValue;
    DAVA::Vector<SubValueProperty*> children;
};

#endif //__UI_EDITOR_VALUE_PROPERTY__
