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


#include "IntrospectionProperty.h"

#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"

#include "PropertyVisitor.h"
#include "SubValueProperty.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include <Base/BaseMath.h>
#include <UI/UIControl.h>

using namespace DAVA;

namespace
{
    const FastName PROPERTY_NAME_SIZE("size");
    const FastName PROPERTY_NAME_POSITION("position");
    const FastName PROPERTY_NAME_TEXT("text");
    const FastName PROPERTY_NAME_FONT("font");
    const FastName PROPERTY_NAME_CLASSES("classes");
}

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject *anObject, const DAVA::InspMember *aMember, const IntrospectionProperty *sourceProperty, eCloneType copyType)
    : ValueProperty(aMember->Desc().text)
    , object(SafeRetain(anObject))
    , member(aMember)
    , flags(EF_CAN_RESET)
{
    if (UIStyleSheetPropertyDataBase::Instance()->IsValidStyleSheetProperty(member->Name()))
        SetStylePropertyIndex(UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyIndex(member->Name()));
    
    if (sourceProperty)
    {
        if (copyType == CT_COPY)
        {
            SetOverridden(sourceProperty->IsOverriddenLocally());
            SetDefaultValue(sourceProperty->GetDefaultValue());
        }
        else
        {
            AttachPrototypeProperty(sourceProperty);
            SetDefaultValue(member->Value(object));
        }
        member->SetValue(object, sourceProperty->GetValue());
    }
    else
    {
        SetDefaultValue(member->Value(object));
    }

    static std::vector<String> vector2ComponentNames = { "X", "Y" };
    static std::vector<String> colorComponentNames = { "Red", "Green", "Blue", "Alpha" };
    static std::vector<String> marginsComponentNames = { "Left", "Top", "Right", "Bottom" };

    std::vector<String> *componentNames = nullptr;
    std::vector<SubValueProperty*> children;
    VariantType defaultValue = GetDefaultValue();
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
        const EnumMap *map = member->Desc().enumMap;
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
    
    if (sourceProperty != nullptr)
        sourceValue = sourceProperty->sourceValue;
    else
        sourceValue = member->Value(object);
}

IntrospectionProperty::~IntrospectionProperty()
{
    SafeRelease(object);
}

IntrospectionProperty *IntrospectionProperty::Create(UIControl *control, const InspMember *member, const IntrospectionProperty *sourceProperty, eCloneType cloneType)
{
    if (member->Name() == PROPERTY_NAME_TEXT)
    {
        return new LocalizedTextValueProperty(control, member, sourceProperty, cloneType);
    }
    else if (member->Name() == PROPERTY_NAME_FONT)
    {
        return new FontValueProperty(control, member, sourceProperty, cloneType);
    }
    else
    {
        IntrospectionProperty *result = new IntrospectionProperty(control, member, sourceProperty, cloneType);;
        if (member->Name() == PROPERTY_NAME_SIZE || member->Name() == PROPERTY_NAME_POSITION)
        {
            result->flags |= EF_DEPENDS_ON_LAYOUTS;
        }
        if (member->Name() == PROPERTY_NAME_CLASSES)
        {
            result->flags |= EF_AFFECTS_STYLES;
        }
        return result;
    }

}

void IntrospectionProperty::Refresh(DAVA::int32 refreshFlags)
{
    ValueProperty::Refresh(refreshFlags);
    
    if ((refreshFlags & REFRESH_DEPENDED_ON_LAYOUT_PROPERTIES) != 0 && (GetFlags() & EF_DEPENDS_ON_LAYOUTS) != 0)
        ApplyValue(sourceValue);
}

void IntrospectionProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitIntrospectionProperty(this);
}

IntrospectionProperty::ePropertyType IntrospectionProperty::GetType() const
{
    auto type = member->Desc().type;
    if (type == InspDesc::T_ENUM)
        return TYPE_ENUM;
    else if (type == InspDesc::T_FLAGS)
        return TYPE_FLAGS;

    return TYPE_VARIANT;
}

uint32 IntrospectionProperty::GetFlags() const
{
    uint32 result = flags;
    if (GetPrototypeProperty() && !IsOverriddenLocally() && IsOverridden())
        result |= EF_INHERITED;
    return result;
}

VariantType IntrospectionProperty::GetValue() const
{
    return member->Value(object);
}

const EnumMap *IntrospectionProperty::GetEnumMap() const
{
    auto type = member->Desc().type;

    if (type == InspDesc::T_ENUM ||
        type == InspDesc::T_FLAGS)
        return member->Desc().enumMap;

    return nullptr;
}

const DAVA::InspMember *IntrospectionProperty::GetMember() const
{
    return member;
}

void IntrospectionProperty::DisableResetFeature()
{
    flags &= ~EF_CAN_RESET;
}

void IntrospectionProperty::ApplyValue(const DAVA::VariantType &value)
{
    sourceValue = value;
    member->SetValue(object, value);

}
