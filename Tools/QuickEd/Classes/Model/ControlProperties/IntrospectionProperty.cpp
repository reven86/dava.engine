#include "IntrospectionProperty.h"

#include "SubValueProperty.h"
#include "Model/PackageSerializer.h"
#include <Base/BaseMath.h>

using namespace DAVA;

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject *anObject, const DAVA::InspMember *aMember, const IntrospectionProperty *sourceProperty, eCloneType copyType)
    : object(SafeRetain(anObject))
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
        static std::vector<String> vector2Components = { "X", "Y" };
        children.push_back(new SubValueProperty(0, vector2Components[0]));
        children.push_back(new SubValueProperty(1, vector2Components[1]));
    }
    else if (defaultValue.GetType() == VariantType::TYPE_COLOR)
    {
        static std::vector<String> colorComponents = { "Red", "Green", "Blue", "Alpha" };
        children.push_back(new SubValueProperty(0, colorComponents[0]));
        children.push_back(new SubValueProperty(1, colorComponents[1]));
        children.push_back(new SubValueProperty(2, colorComponents[2]));
        children.push_back(new SubValueProperty(3, colorComponents[3]));
    }
    else if (defaultValue.GetType() == VariantType::TYPE_VECTOR4)
    {
        static std::vector<String> marginComponents = { "Left", "Top", "Right", "Bottom" };
        children.push_back(new SubValueProperty(0, marginComponents[0]));
        children.push_back(new SubValueProperty(1, marginComponents[1]));
        children.push_back(new SubValueProperty(2, marginComponents[2]));
        children.push_back(new SubValueProperty(3, marginComponents[3]));
    }
    else if (defaultValue.GetType() == VariantType::TYPE_INT32 && member->Desc().type == InspDesc::T_FLAGS)
    {
        const EnumMap *map = member->Desc().enumMap;
        for (int32 i = 0; i < (int32)map->GetCount(); i++)
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

        if (value.GetType() == VariantType::TYPE_INT32 && member->Desc().type == InspDesc::T_FLAGS)
        {
            Vector<String> values;
            int val = value.AsInt32();
            int p = 1;
            while (val > 0)
            {
                if ((val & 0x01) != 0)
                    values.push_back(member->Desc().enumMap->ToString(p));
                val >>= 1;
                p <<= 1;
            }
            serializer->PutValue(member->Name(), values);
        }
        else if (value.GetType() == VariantType::TYPE_INT32 && member->Desc().type == InspDesc::T_ENUM)
        {
            serializer->PutValue(member->Name(), member->Desc().enumMap->ToString(value.AsInt32()));
        }
        else
        {
            serializer->PutValue(member->Name(), value);
        }
    }
}

String IntrospectionProperty::GetName() const
{
    return member->Desc().text;
}

IntrospectionProperty::ePropertyType IntrospectionProperty::GetType() const
{
    if (member->Desc().type == InspDesc::T_ENUM)
        return TYPE_ENUM;
    else if (member->Desc().type == InspDesc::T_FLAGS)
        return TYPE_FLAGS;
    return TYPE_VARIANT;
}

VariantType IntrospectionProperty::GetValue() const
{
    return member->Value(object);
}

const EnumMap *IntrospectionProperty::GetEnumMap() const
{
    if (member->Desc().type == InspDesc::T_ENUM)
        return member->Desc().enumMap;
    else if (member->Desc().type == InspDesc::T_FLAGS)
        return member->Desc().enumMap;
    return NULL;
}

void IntrospectionProperty::ApplyValue(const DAVA::VariantType &value)
{
    member->SetValue(object, value);
}

