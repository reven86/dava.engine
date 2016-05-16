/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Model/ControlProperties/SubValueProperty.h"
#include "Preferences/PreferencesIntrospectionProperty.h"
#include "Preferences/PreferencesStorage.h"

PreferencesIntrospectionProperty::PreferencesIntrospectionProperty(const DAVA::InspMember* aMember)
    : ValueProperty(aMember->Desc().text, DAVA::VariantType::TypeFromMetaInfo(aMember->Type()), true, &aMember->Desc())
    , member(aMember)
{
    DAVA::VariantType value = PreferencesStorage::Instance()->GetDefaultValue(member);
    if (value.type != DAVA::VariantType::TYPE_NONE)
    {
        ValueProperty::SetDefaultValue(value);
    }
    else
    {
        DAVA::VariantType::eVariantType type = ValueProperty::GetValueType();
        ValueProperty::SetDefaultValue(DAVA::VariantType::FromType(type));
    }
    PreferencesIntrospectionProperty::SetValue(PreferencesStorage::Instance()->GetValue(member));
    DAVA::String name(aMember->Desc().text);
    DAVA::size_type index = name.find_last_of('/');
    if (index == DAVA::String::npos)
    {
        SetName(name);
    }
    else
    {
        SetName(name.substr(index + 1));
    }
}

void PreferencesIntrospectionProperty::ApplyPreference()
{
    if (IsOverriddenLocally())
    {
        PreferencesStorage::Instance()->SetValue(member, GetValue());
    }
}

void PreferencesIntrospectionProperty::SetValue(const DAVA::VariantType& val)
{
    value = val;
}

DAVA::VariantType PreferencesIntrospectionProperty::GetValue() const
{
    return value;
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
    return PreferencesStorage::Instance()->GetValue(member) != GetValue();
}

DAVA::uint32 PreferencesIntrospectionProperty::GetFlags() const
{
    DAVA::VariantType defaultValue = PreferencesStorage::Instance()->GetDefaultValue(member);
    return (defaultValue.type != DAVA::VariantType::TYPE_NONE) ? EF_CAN_RESET : EF_NONE;
}

void PreferencesIntrospectionProperty::ResetValue()
{
    SetValue(PreferencesStorage::Instance()->GetDefaultValue(member));
}
