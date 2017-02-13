#include "Model/ControlProperties/SubValueProperty.h"
#include "Preferences/PreferencesIntrospectionProperty.h"
#include "Preferences/PreferencesStorage.h"
#include "Utils/QtDavaConvertion.h"

PreferencesIntrospectionProperty::PreferencesIntrospectionProperty(const DAVA::InspMember* member_)
    : ValueProperty(member_->Desc().text, VariantTypeToType(DAVA::VariantType::TypeFromMetaInfo(member_->Type())))
    , member(member_)
{
    DAVA::VariantType defaultValue = PreferencesStorage::Instance()->GetDefaultValue(member);
    if (defaultValue.type != DAVA::VariantType::TYPE_NONE)
    {
        ValueProperty::SetDefaultValue(VariantTypeToAny(defaultValue));
    }
    else
    {
        DAVA::VariantType::eVariantType type = DAVA::VariantType::TypeFromMetaInfo(member_->Type());
        ValueProperty::SetDefaultValue(VariantTypeToAny(DAVA::VariantType::FromType(type)));
    }
    valueOnOpen = PreferencesStorage::Instance()->GetValue(member);
    value = valueOnOpen;
    DAVA::String name(member_->Desc().text);
    DAVA::size_type index = name.find_last_of('/');
    if (index == DAVA::String::npos)
    {
        SetName(name);
    }
    else
    {
        SetName(name.substr(index + 1));
    }

    GenerateBuiltInSubProperties();
}

void PreferencesIntrospectionProperty::ApplyPreference()
{
    if (value != valueOnOpen)
    {
        PreferencesStorage::Instance()->SetValue(member, AnyToVariantType(GetValue()));
    }
}

PreferencesIntrospectionProperty::ePropertyType PreferencesIntrospectionProperty::GetType() const
{
    return TYPE_VARIANT;
}

void PreferencesIntrospectionProperty::SetValue(const DAVA::Any& val)
{
    value = AnyToVariantType(val);
}

DAVA::Any PreferencesIntrospectionProperty::GetValue() const
{
    return VariantTypeToAny(value);
}

const EnumMap* PreferencesIntrospectionProperty::GetEnumMap() const
{
    DAVA::InspDesc::Type type = member->Desc().type;
    switch (type)
    {
    case DAVA::InspDesc::T_ENUM:
    case DAVA::InspDesc::T_FLAGS:
        return member->Desc().enumMap;
    default:
        return nullptr;
    }
}

const DAVA::InspMember* PreferencesIntrospectionProperty::GetMember() const
{
    return member;
}

bool PreferencesIntrospectionProperty::IsReadOnly() const
{
    return (member->Flags() & DAVA::I_EDIT) == 0;
}

bool PreferencesIntrospectionProperty::IsOverriddenLocally() const
{
    return GetDefaultValue() != GetValue();
}

DAVA::uint32 PreferencesIntrospectionProperty::GetFlags() const
{
    DAVA::VariantType defaultValue = PreferencesStorage::Instance()->GetDefaultValue(member);
    return (defaultValue.type != DAVA::VariantType::TYPE_NONE) ? EF_CAN_RESET : EF_NONE;
}

void PreferencesIntrospectionProperty::ResetValue()
{
    SetValue(VariantTypeToAny(PreferencesStorage::Instance()->GetDefaultValue(member)));
}
