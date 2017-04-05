#pragma once

#include "Model/ControlProperties/ValueProperty.h"

class PreferencesIntrospectionProperty : public ValueProperty
{
public:
    PreferencesIntrospectionProperty(const DAVA::InspMember* member);

    //Apply is already used :(
    void ApplyPreference();

    ePropertyType GetType() const override;
    void SetValue(const DAVA::Any& value) override;
    DAVA::Any GetValue() const override;

    const EnumMap* GetEnumMap() const override;

    const DAVA::InspMember* GetMember() const;

    bool IsReadOnly() const override;

    bool IsOverriddenLocally() const override;

    DAVA::uint32 GetFlags() const override;

    void ResetValue() override;

    void Accept(PropertyVisitor* /*visitor*/) override
    {
    }

protected:
    DAVA::VariantType value;
    DAVA::VariantType valueOnOpen;
    const DAVA::InspMember* member;
};
