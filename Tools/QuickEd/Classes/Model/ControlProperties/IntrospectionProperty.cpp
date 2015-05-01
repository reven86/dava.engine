#include "IntrospectionProperty.h"

#include "SubValueProperty.h"
#include "Model/PackageSerializer.h"
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

uint32 IntrospectionProperty::GetEditFlag() const
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

void IntrospectionProperty::ApplyValue(const DAVA::VariantType &value)
{
    member->SetValue(object, value);
}
