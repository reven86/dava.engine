#include "IntrospectionProperty.h"

#include "SubValueProperty.h"
#include "Model/PackageSerializer.h"
#include <Base/BaseMath.h>

using namespace DAVA;

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject *anObject, const DAVA::InspMember *aMember, const IntrospectionProperty *sourceProperty, eCloneType copyType)
    : ValueProperty(aMember->Desc().text)
    , object(SafeRetain(anObject))
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
            defaultValue = member->Value(object);
        }
    }
    else
    {
        defaultValue = member->Value(object);
    }

    if (defaultValue.GetType() == VariantType::TYPE_VECTOR2)
    {
        static std::vector<String> componentNames = { "X", "Y" };
        for (size_t i = 0; i < componentNames.size(); ++i)
        {
            children.push_back(new SubValueProperty(i, componentNames[i]));
        }
    }
    else if (defaultValue.GetType() == VariantType::TYPE_COLOR)
    {
        static std::vector<String> componentNames = { "Red", "Green", "Blue", "Alpha" };
        for (size_t i = 0; i < componentNames.size(); ++i)
        {
            children.push_back(new SubValueProperty(i, componentNames[i]));
        }
    }
    else if (defaultValue.GetType() == VariantType::TYPE_VECTOR4)
    {
        static std::vector<String> componentNames = { "Left", "Top", "Right", "Bottom" };
        for (size_t i = 0; i < componentNames.size(); ++i)
        {
            children.push_back(new SubValueProperty(i, componentNames[i]));
        }
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

    for (auto child : children)
    {
        child->SetParent(this);
    }
}

IntrospectionProperty::~IntrospectionProperty()
{
    SafeRelease(object);
}

void IntrospectionProperty::Serialize(PackageSerializer *serializer) const
{
    if (replaced)
    {
        VariantType value = GetValue();
        String key = member->Name();

        if (value.GetType() == VariantType::TYPE_INT32 && GetType() == TYPE_FLAGS)
        {
            Vector<String> values;
            const EnumMap *enumMap = GetEnumMap();
            int val = value.AsInt32();
            int p = 1;
            while (val > 0)
            {
                if ((val & 0x01) != 0)
                    values.push_back(enumMap->ToString(p));
                val >>= 1;
                p <<= 1;
            }
            serializer->PutValue(key, values);
        }
        else if (value.GetType() == VariantType::TYPE_INT32 && GetType() == TYPE_ENUM)
        {
            const EnumMap *enumMap = GetEnumMap();
            serializer->PutValue(key, enumMap->ToString(value.AsInt32()));
        }
        else
        {
            serializer->PutValue(key, value);
        }
    }
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

void IntrospectionProperty::ApplyValue(const DAVA::VariantType &value)
{
    member->SetValue(object, value);
}

