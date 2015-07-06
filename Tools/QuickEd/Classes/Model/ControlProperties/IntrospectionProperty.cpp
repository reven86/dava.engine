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

#include "PropertyVisitor.h"
#include "SubValueProperty.h"
#include <Base/BaseMath.h>

using namespace DAVA;

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject *anObject, const DAVA::InspMember *aMember, const IntrospectionProperty *sourceProperty, eCloneType copyType)
    : ValueProperty(aMember->Desc().text)
    , object(SafeRetain(anObject))
    , prototypeProperty(nullptr)
    , member(aMember)
{
    if (sourceProperty)
    {
        if (sourceProperty->GetValue() != member->Value(object))
            member->SetValue(object, sourceProperty->GetValue());

        if (copyType == CT_COPY)
        {
            defaultValue = sourceProperty->GetDefaultValue();
            replaced = sourceProperty->IsReplaced();
        }
        else
        {
            prototypeProperty = sourceProperty;
            defaultValue = member->Value(object);
        }
    }
    else
    {
        defaultValue = member->Value(object);
    }

    static std::vector<String> vector2ComponentNames = { "X", "Y" };
    static std::vector<String> colorComponentNames = { "Red", "Green", "Blue", "Alpha" };
    static std::vector<String> marginsComponentNames = { "Left", "Top", "Right", "Bottom" };

    std::vector<String> *componentNames = nullptr;
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
    }
}

IntrospectionProperty::~IntrospectionProperty()
{
    SafeRelease(object);
    prototypeProperty = nullptr;
}

void IntrospectionProperty::Refresh()
{
    if (prototypeProperty)
    {
        SetDefaultValue(prototypeProperty->GetValue());
    }
    ValueProperty::Refresh();
}

AbstractProperty *IntrospectionProperty::FindPropertyByPrototype(AbstractProperty *prototype)
{
    return prototype == prototypeProperty ? this : nullptr;
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
    uint32 flags = EF_CAN_RESET;
    if (prototypeProperty && !replaced)
        flags |= EF_INHERITED;
    return flags;
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

void IntrospectionProperty::ApplyValue(const DAVA::VariantType &value)
{
    member->SetValue(object, value);
}
