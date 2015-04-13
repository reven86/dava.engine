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
    else if (defaultValue.GetType() == VariantType::TYPE_VECTOR4)
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
    static std::vector<String> colorComponents = {{"Red", "Green", "Blue", "Alpha"}};
    static std::vector<String> marginComponents = {{"Left", "Top", "Right", "Bottom"}};
    static std::vector<String> vector2Components = {{"X", "Y"}};
    
    std::vector<String> *components = nullptr;
    
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            components = &vector2Components;
            break;

        case VariantType::TYPE_COLOR:
            components = &colorComponents;
            break;

        case VariantType::TYPE_VECTOR4:
            components = &marginComponents;
            break;

        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int val = 0;
                map->GetValue(index, val);
                return map->ToString(val);
            }
            break;
            
        default:
            break;
    }
    
    if (components != nullptr)
    {
        if (0 <= index && index < components->size())
            return components->at(index);
        else
        {
            DVASSERT(false);
            return "???";
        }
    }
    else
    {
        DVASSERT(false);
        return "???";
    }
}

VariantType ValueProperty::GetSubValue(int index) const
{
    return GetValueComponent(GetValue(), index);
}

void ValueProperty::SetSubValue(int index, const DAVA::VariantType &newValue)
{
    SetValue(ChangeValueComponent(GetValue(), newValue, index));
}

VariantType ValueProperty::GetDefaultSubValue(int index) const
{
    return GetValueComponent(defaultValue, index);
}

void ValueProperty::SetDefaultSubValue(int index, const DAVA::VariantType &newValue)
{
    SetDefaultValue(ChangeValueComponent(defaultValue, newValue, index));
}

void ValueProperty::ApplyValue(const DAVA::VariantType &value)
{
    member->SetValue(object, value);
}

VariantType ValueProperty::ChangeValueComponent(const VariantType &value, const VariantType &component, int32 index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
        {
            Vector2 val = value.AsVector2();
            if (index == 0)
                val.x = component.AsFloat();
            else
                val.y = component.AsFloat();
            
            return VariantType(val);
        }
            
        case VariantType::TYPE_COLOR:
        {
            Color val = value.AsColor();
            if (0 <= index && index < 4)
            {
                val.color[index ] = component.AsFloat();
            }
            else
            {
                DVASSERT(false);
            }
            
            return VariantType(val);
        }
            
        case VariantType::TYPE_VECTOR4:
        {
            Vector4 val = value.AsVector4();
            if (0 <= index && index < 4)
            {
                val.data[index] = component.AsFloat();
            }
            else
            {
                DVASSERT(false);
            }
            return VariantType(val);
        }
            
        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int32 intValue = value.AsInt32();
                
                int val = 0;
                map->GetValue(index, val);
                if (component.AsBool())
                    return VariantType(intValue | val);
                else
                    return VariantType(intValue & (~val));
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
    return VariantType();
}

DAVA::VariantType ValueProperty::GetValueComponent(const DAVA::VariantType &value, DAVA::int32 index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
        {
            DVASSERT(index >= 0 && index < 2);
            return VariantType(value.AsVector2().data[index]);
        }
            
        case VariantType::TYPE_COLOR:
        {
            DVASSERT(index >= 0 && index < 4);
            return VariantType(value.AsColor().color[index]);
        }
            
        case VariantType::TYPE_VECTOR4:
        {
            DVASSERT(index >= 0 && index < 4);
            return VariantType(value.AsVector4().data[index]);
        }
            
        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int val = 0;
                map->GetValue(index, val);
                return VariantType((value.AsInt32() & val) != 0);
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
