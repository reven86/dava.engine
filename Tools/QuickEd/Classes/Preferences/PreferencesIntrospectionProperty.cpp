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
#include "QtTools/EditorPreferences/PreferencesStorage.h"

using namespace DAVA;

PreferencesIntrospectionProperty::PreferencesIntrospectionProperty(const DAVA::InspMember* aMember)
    : ValueProperty(aMember->Desc().text)
    , member(aMember)
    , flags(EF_CAN_RESET)
{
    DAVA::VariantType value = PreferencesStorage::GetPreferencesValue(member);
    SetDefaultValue(value);

    static std::vector<DAVA::String> vector2ComponentNames = { "X", "Y" };
    static std::vector<DAVA::String> colorComponentNames = { "Red", "Green", "Blue", "Alpha" };
    static std::vector<DAVA::String> marginsComponentNames = { "Left", "Top", "Right", "Bottom" };

    std::vector<DAVA::String>* componentNames = nullptr;
    std::vector<SubValueProperty*> children;
    DAVA::VariantType defaultValue = GetDefaultValue();
    if (defaultValue.GetType() == VariantType::TYPE_VECTOR2)
    {
        componentNames = &vector2ComponentNames;
    }
    else if (defaultValue.GetType() == VariantType::TYPE_COLOR)
    {
        componentNames = &colorComponentNames;
    }
    else if (defaultValue.GetType() == VariantType::TYPE_VECTOR4)
    {
        componentNames = &marginsComponentNames;
    }
    else if (defaultValue.GetType() == VariantType::TYPE_INT32 && member->Desc().type == InspDesc::T_FLAGS)
    {
        const EnumMap* map = member->Desc().enumMap;
        for (size_t i = 0; i < map->GetCount(); ++i)
        {
            int val = 0;
            map->GetValue(i, val);
            children.push_back(new SubValueProperty(i, map->ToString(val)));
        }
    }

    if (componentNames != nullptr)
    {
        for (size_t i = 0; i < componentNames->size(); ++i)
            children.push_back(new SubValueProperty(i, componentNames->at(i)));
    }

    for (auto child : children)
    {
        child->SetParent(this);
        AddSubValueProperty(child);
        SafeRelease(child);
    }
    children.clear();

    sourceValue = defaultValue;
}

PreferencesIntrospectionProperty::~PreferencesIntrospectionProperty() = default;

void PreferencesIntrospectionProperty::Refresh(DAVA::int32 refreshFlags)
{
    ValueProperty::Refresh(refreshFlags);
}

void PreferencesIntrospectionProperty::Accept(PropertyVisitor*)
{
}

PreferencesIntrospectionProperty::ePropertyType PreferencesIntrospectionProperty::GetType() const
{
    auto type = member->Desc().type;
    if (type == InspDesc::T_ENUM)
        return TYPE_ENUM;
    else if (type == InspDesc::T_FLAGS)
        return TYPE_FLAGS;

    return TYPE_VARIANT;
}

uint32 PreferencesIntrospectionProperty::GetFlags() const
{
    uint32 result = flags;
    if (GetPrototypeProperty() && !IsOverriddenLocally() && IsOverridden())
        result |= EF_INHERITED;
    return result;
}

VariantType PreferencesIntrospectionProperty::GetValue() const
{
    return PreferencesStorage::GetPreferencesValue(member);
}

const EnumMap* PreferencesIntrospectionProperty::GetEnumMap() const
{
    auto type = member->Desc().type;

    if (type == InspDesc::T_ENUM ||
        type == InspDesc::T_FLAGS)
        return member->Desc().enumMap;

    return nullptr;
}

const InspMember* PreferencesIntrospectionProperty::GetMember() const
{
    return member;
}

bool PreferencesIntrospectionProperty::IsReadOnly() const
{
    return (member->Flags() & I_EDIT) == 0;
}

void PreferencesIntrospectionProperty::ApplyValue(const DAVA::VariantType& value)
{
    sourceValue = value;
    PreferencesStorage::SetNewValueToAllRegisteredObjects(member->GetParentInsp(), member, sourceValue);
}
