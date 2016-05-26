#ifndef __UI_EDITOR_SUB_VALUE_PROPERTY__
#define __UI_EDITOR_SUB_VALUE_PROPERTY__

#include "AbstractProperty.h"

class ValueProperty;

class SubValueProperty : public AbstractProperty
{
public:
    SubValueProperty(DAVA::int32 index, const DAVA::String& propName);

protected:
    virtual ~SubValueProperty();

public:
    DAVA::uint32 GetCount() const override;
    AbstractProperty* GetProperty(int index) const override;
    void Accept(PropertyVisitor* visitor) override;

    const DAVA::String& GetName() const override;
    ePropertyType GetType() const override;
    DAVA::VariantType::eVariantType GetValueType() const override;
    DAVA::VariantType GetValue() const override;
    void SetValue(const DAVA::VariantType& newValue) override;
    DAVA::VariantType GetDefaultValue() const override;
    void SetDefaultValue(const DAVA::VariantType& newValue) override;
    void ResetValue() override;
    bool IsOverriddenLocally() const override;

private:
    ValueProperty* GetValueProperty() const;
    DAVA::int32 index = 0;
    DAVA::String name;
};

#endif // __UI_EDITOR_SUB_VALUE_PROPERTY__
