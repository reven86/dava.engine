#include "ValueProperty.h"

#include "SubValueProperty.h"
#include "../PackageSerializer.h"
#include "Base/BaseMath.h"

using namespace DAVA;

ValueProperty::ValueProperty(BaseObject *object, const InspMember *member, ValueProperty *sourceProperty, eCopyType copyType)
    : object(NULL), member(member), replaced(false)
{
    this->object = SafeRetain(object);
    
    if (sourceProperty)
    {
        if (sourceProperty->GetValue() != member->Value(object))
            member->SetValue(object, sourceProperty->GetValue());
        
        if (copyType == BaseProperty::COPY_FULL)
        {
            defaultValue = sourceProperty->defaultValue;
            replaced = sourceProperty->replaced;
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
        children.push_back(new SubValueProperty(0));
        children.push_back(new SubValueProperty(1));
    }
    else if (defaultValue.GetType() == VariantType::TYPE_COLOR)
    {
        children.push_back(new SubValueProperty(0));
        children.push_back(new SubValueProperty(1));
        children.push_back(new SubValueProperty(2));
        children.push_back(new SubValueProperty(3));
    }
    else if (defaultValue.GetType() == VariantType::TYPE_INT32 && member->Desc().type == InspDesc::T_FLAGS)
    {
        const EnumMap *map = member->Desc().enumMap;
        for (int32 i = 0; i < (int32) map->GetCount(); i++)
            children.push_back(new SubValueProperty(i));
    }
    
    for (auto it = children.begin(); it != children.end(); ++it)
        (*it)->SetParent(this);
}

ValueProperty::~ValueProperty()
{
    for (auto it = children.begin(); it != children.end(); ++it)
        (*it)->Release();
    children.clear();
    
    SafeRelease(object);
}

int ValueProperty::GetCount() const
{
    return (int) children.size();
}

BaseProperty *ValueProperty::GetProperty(int index) const
{
    return children[index];
}

bool ValueProperty::HasChanges() const
{
    return replaced;
}

void ValueProperty::Serialize(PackageSerializer *serializer) const
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

String ValueProperty::GetName() const
{
    return member->Desc().text;
}

ValueProperty::ePropertyType ValueProperty::GetType() const
{
    if (member->Desc().type == InspDesc::T_ENUM)
        return TYPE_ENUM;
    else if (member->Desc().type == InspDesc::T_FLAGS)
        return TYPE_FLAGS;
    return TYPE_VARIANT;
}

VariantType ValueProperty::GetValue() const
{
    return member->Value(object);
}

void ValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    replaced = true;
    ApplyValue(newValue);
}

VariantType ValueProperty::GetDefaultValue() const
{
    return defaultValue;
}

void ValueProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    defaultValue = newValue;
    if (!replaced)
        ApplyValue(newValue);
}

const EnumMap *ValueProperty::GetEnumMap() const
{
    if (member->Desc().type == InspDesc::T_ENUM)
        return member->Desc().enumMap;
    else if (member->Desc().type == InspDesc::T_FLAGS)
        return member->Desc().enumMap;
    return NULL;
}

void ValueProperty::ResetValue()
{
    replaced = false;
    ApplyValue(defaultValue);
}

bool ValueProperty::IsReplaced() const
{
    return replaced;
}

String ValueProperty::GetSubValueName(int index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            return index == 0 ? "X" : "Y";

        case VariantType::TYPE_COLOR:
        {
            if (index == 0)
                return "Red";
            else if (index == 1)
                return "Green";
            else if (index == 2)
                return "Blue";
            else
                return "Alpha";
        }
            
        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int val = 0;
                map->GetValue(index, val);
                return map->ToString(val);
            }
            else
            {
                DVASSERT(false);
                return "???";
            }

            
        default:
        {
            DVASSERT(false);
            return "???";
        }
    }
}

VariantType ValueProperty::GetSubValue(int index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            {
                DVASSERT(index >= 0 && index < 2);
                return VariantType(GetValue().AsVector2().data[index]);
            }

        case VariantType::TYPE_COLOR:
        {
            DVASSERT(index >= 0 && index < 4);
            return VariantType(GetValue().AsColor().color[index]);
        }

        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int val = 0;
                map->GetValue(index, val);
                return VariantType((GetValue().AsInt32() & val) != 0);
            }
            else
            {
                DVASSERT(false);
                return VariantType();
            }

        default:
            DVASSERT(false);
            return VariantType();
    }
}

void ValueProperty::SetSubValue(int index, const DAVA::VariantType &newValue)
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
        {
            Vector2 val = GetValue().AsVector2();
            if (index == 0)
                val.x = newValue.AsFloat();
            else
                val.y = newValue.AsFloat();
            
            SetValue(VariantType(val));
            break;
        }
            
        case VariantType::TYPE_COLOR:
        {
            Color val = GetValue().AsColor();
            if (index == 0)
                val.r = newValue.AsFloat();
            else if (index == 1)
                val.g = newValue.AsFloat();
            else if (index == 2)
                val.b = newValue.AsFloat();
            else
                val.a = newValue.AsFloat();
            
            SetValue(VariantType(val));
            break;
        }
            
        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int32 value = GetValue().AsInt32();

                int val = 0;
                map->GetValue(index, val);
                if (newValue.AsBool())
                    SetValue(VariantType(value | val));
                else
                    SetValue(VariantType(value & (~val)));
            }
            else
            {
                DVASSERT(false);
            }
            break;
            
        default:
            DVASSERT(false);
            break;
    }
}

void ValueProperty::ApplyValue(const DAVA::VariantType &value)
{
    member->SetValue(object, value);
}
