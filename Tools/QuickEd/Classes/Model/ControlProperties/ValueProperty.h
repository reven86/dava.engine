#ifndef __UI_EDITOR_VALUE_PROPERTY__
#define __UI_EDITOR_VALUE_PROPERTY__

#include "AbstractProperty.h"

class ValueProperty : public AbstractProperty
{
public:
    ValueProperty(const DAVA::String& propName, DAVA::VariantType::eVariantType valueType, bool builtinSubProps = false, const DAVA::InspDesc* inspDesc = nullptr);

protected:
    virtual ~ValueProperty();

public:
    DAVA::uint32 GetCount() const override;
    AbstractProperty* GetProperty(DAVA::int32 index) const override;

    void Refresh(DAVA::int32 refreshFlags) override;

    void AttachPrototypeProperty(const ValueProperty* prototypeProperty);
    void DetachPrototypeProperty(const ValueProperty* prototypeProperty);
    const ValueProperty* GetPrototypeProperty() const;
    AbstractProperty* FindPropertyByPrototype(AbstractProperty* prototype) override;

    bool HasChanges() const override;
    const DAVA::String& GetName() const override;
    ePropertyType GetType() const override;
    DAVA::int32 GetStylePropertyIndex() const override;

    DAVA::VariantType::eVariantType GetValueType() const override;
    DAVA::VariantType GetValue() const override;
    void SetValue(const DAVA::VariantType& newValue) override;
    DAVA::VariantType GetDefaultValue() const override;
    void SetDefaultValue(const DAVA::VariantType& newValue) override;
    void ResetValue() override;
    bool IsOverridden() const override;
    bool IsOverriddenLocally() const override;

    virtual DAVA::VariantType::eVariantType GetSubValueType(DAVA::int32 index) const;
    virtual DAVA::VariantType GetSubValue(DAVA::int32 index) const;
    virtual void SetSubValue(DAVA::int32 index, const DAVA::VariantType& newValue);
    virtual DAVA::VariantType GetDefaultSubValue(DAVA::int32 index) const;
    virtual void SetDefaultSubValue(DAVA::int32 index, const DAVA::VariantType& newValue);

    virtual const EnumMap* GetEnumMap() const override;
    DAVA_DEPRECATED(virtual bool IsSameMember(const DAVA::InspMember* member) const)
    {
        return false;
    }

protected:
    virtual void ApplyValue(const DAVA::VariantType& value);
    void SetName(const DAVA::String& newName);
    void SetOverridden(bool overridden);
    void SetStylePropertyIndex(DAVA::int32 index);
    void AddSubValueProperty(AbstractProperty* prop);

private:
    DAVA::VariantType ChangeValueComponent(const DAVA::VariantType& value, const DAVA::VariantType& component, DAVA::int32 index) const;
    DAVA::VariantType::eVariantType GetValueTypeComponent(DAVA::int32 index) const;
    DAVA::VariantType GetValueComponent(const DAVA::VariantType& value, DAVA::int32 index) const;
    void GenerateBuiltInSubProperties();

private:
    DAVA::String name;
    DAVA::VariantType::eVariantType valueType;
    DAVA::VariantType defaultValue;
    DAVA::Vector<DAVA::RefPtr<AbstractProperty>> children;
    DAVA::int32 stylePropertyIndex = -1;
    bool overridden = false;
    const DAVA::InspDesc* inspDesc = nullptr;
    const ValueProperty* prototypeProperty = nullptr; // weak

public:
    INTROSPECTION_EXTEND(ValueProperty, AbstractProperty,
                         nullptr
                         );
};

#endif //__UI_EDITOR_VALUE_PROPERTY__
