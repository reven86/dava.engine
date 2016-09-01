#include "ValueProperty.h"

#include "SubValueProperty.h"
#include <Base/BaseMath.h>

using namespace DAVA;

namespace SValueProperty
{
static const Vector<String> VECTOR2_COMPONENT_NAMES = { "X", "Y" };
static const Vector<String> COLOR_COMPONENT_NAMES = { "Red", "Green", "Blue", "Alpha" };
static const Vector<String> MARGINS_COMPONENT_NAMESs = { "Left", "Top", "Right", "Bottom" };
}

ValueProperty::ValueProperty(const String& propName, VariantType::eVariantType type, bool builtinSubProps, const InspDesc* desc)
    : name(propName)
    , valueType(type)
    , defaultValue(VariantType::FromType(type))
    , inspDesc(desc)
{
    if (builtinSubProps)
    {
        GenerateBuiltInSubProperties();
    }
}

ValueProperty::~ValueProperty()
{
    children.clear();

    prototypeProperty = nullptr; // weak
}

uint32 ValueProperty::GetCount() const
{
    return static_cast<int32>(children.size());
}

AbstractProperty* ValueProperty::GetProperty(int32 index) const
{
    if (0 <= index && index < static_cast<int32>(children.size()))
    {
        return children[index].Get();
    }
    else
    {
        DVASSERT(false);
        return nullptr;
    }
}

void ValueProperty::Refresh(int32 refreshFlags)
{
    if ((refreshFlags & REFRESH_DEFAULT_VALUE) != 0 && prototypeProperty)
        SetDefaultValue(prototypeProperty->GetValue());

    for (RefPtr<AbstractProperty>& prop : children)
        prop->Refresh(refreshFlags);
}

void ValueProperty::AttachPrototypeProperty(const ValueProperty* property)
{
    if (prototypeProperty == nullptr)
    {
        prototypeProperty = property;
    }
    else
    {
        DVASSERT(false);
    }
}

void ValueProperty::DetachPrototypeProperty(const ValueProperty* property)
{
    if (prototypeProperty == property)
    {
        prototypeProperty = nullptr;
    }
    else
    {
        DVASSERT(false);
    }
}

const ValueProperty* ValueProperty::GetPrototypeProperty() const
{
    return prototypeProperty;
}

AbstractProperty* ValueProperty::FindPropertyByPrototype(AbstractProperty* prototype)
{
    return prototype == prototypeProperty ? this : nullptr;
}

bool ValueProperty::HasChanges() const
{
    return IsOverriddenLocally();
}

const String& ValueProperty::GetName() const
{
    return name;
}

ValueProperty::ePropertyType ValueProperty::GetType() const
{
    auto type = inspDesc ? inspDesc->type : InspDesc::T_UNDEFINED;
    if (type == InspDesc::T_ENUM)
        return TYPE_ENUM;
    else if (type == InspDesc::T_FLAGS)
        return TYPE_FLAGS;

    return TYPE_VARIANT;
}

DAVA::VariantType::eVariantType ValueProperty::GetValueType() const
{
    return valueType;
}

VariantType ValueProperty::GetValue() const
{
    return VariantType::FromType(GetValueType());
}

void ValueProperty::SetValue(const VariantType& newValue)
{
    overridden = true;
    ApplyValue(newValue);
}

VariantType ValueProperty::GetDefaultValue() const
{
    return defaultValue;
}

void ValueProperty::SetDefaultValue(const VariantType& newValue)
{
    VariantType::eVariantType valueType = GetValueType();
    DVASSERT(newValue.GetType() == valueType);

    defaultValue = newValue;
    if (!overridden)
        ApplyValue(newValue);
}

const EnumMap* ValueProperty::GetEnumMap() const
{
    auto type = inspDesc ? inspDesc->type : InspDesc::T_UNDEFINED;

    if (type == InspDesc::T_ENUM ||
        type == InspDesc::T_FLAGS)
        return inspDesc->enumMap;

    return nullptr;
}

void ValueProperty::ResetValue()
{
    overridden = false;
    ApplyValue(defaultValue);
}

bool ValueProperty::IsOverridden() const
{
    bool overriddenLocally = IsOverriddenLocally();
    if (overriddenLocally || prototypeProperty == nullptr)
        return overriddenLocally;

    return prototypeProperty->IsOverridden();
}

bool ValueProperty::IsOverriddenLocally() const
{
    return overridden;
}

VariantType::eVariantType ValueProperty::GetSubValueType(int32 index) const
{
    return GetValueTypeComponent(index);
}

VariantType ValueProperty::GetSubValue(int32 index) const
{
    return GetValueComponent(GetValue(), index);
}

void ValueProperty::SetSubValue(int32 index, const VariantType& newValue)
{
    SetValue(ChangeValueComponent(GetValue(), newValue, index));
}

VariantType ValueProperty::GetDefaultSubValue(int32 index) const
{
    return GetValueComponent(defaultValue, index);
}

void ValueProperty::SetDefaultSubValue(int32 index, const VariantType& newValue)
{
    SetDefaultValue(ChangeValueComponent(defaultValue, newValue, index));
}

int32 ValueProperty::GetStylePropertyIndex() const
{
    return stylePropertyIndex;
}

void ValueProperty::ApplyValue(const VariantType& value)
{
}

void ValueProperty::SetName(const String& newName)
{
    name = newName;
}

void ValueProperty::SetOverridden(bool anOverridden)
{
    overridden = anOverridden;
}

void ValueProperty::SetStylePropertyIndex(int32 index)
{
    stylePropertyIndex = index;
}

void ValueProperty::AddSubValueProperty(AbstractProperty* prop)
{
    children.push_back(RefPtr<AbstractProperty>(SafeRetain(prop)));
}

VariantType ValueProperty::ChangeValueComponent(const VariantType& value, const VariantType& component, int32 index) const
{
    VariantType::eVariantType valueType = GetValueType();
    DVASSERT(defaultValue.GetType() == valueType);

    switch (valueType)
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
            val.color[index] = component.AsFloat();
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
        if (GetType() == TYPE_FLAGS)
        {
            const EnumMap* map = GetEnumMap();
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

VariantType::eVariantType ValueProperty::GetValueTypeComponent(int32 index) const
{
    VariantType::eVariantType valueType = GetValueType();
    DVASSERT(defaultValue.GetType() == valueType);

    switch (valueType)
    {
    case VariantType::TYPE_VECTOR2:
    {
        DVASSERT(index >= 0 && index < 2);
        return VariantType::TYPE_FLOAT;
    }

    case VariantType::TYPE_COLOR:
    {
        DVASSERT(index >= 0 && index < 4);
        return VariantType::TYPE_FLOAT;
    }

    case VariantType::TYPE_VECTOR4:
    {
        DVASSERT(index >= 0 && index < 4);
        return VariantType::TYPE_FLOAT;
    }

    case VariantType::TYPE_INT32:
        if (GetType() == TYPE_FLAGS)
        {
            return VariantType::TYPE_BOOLEAN;
        }
        else
        {
            DVASSERT(false);
            return VariantType::TYPE_NONE;
        }

    default:
        DVASSERT(false);
        return VariantType::TYPE_NONE;
    }
}

VariantType ValueProperty::GetValueComponent(const VariantType& value, int32 index) const
{
    VariantType::eVariantType valueType = GetValueType();
    DVASSERT(defaultValue.GetType() == valueType);

    switch (valueType)
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
        if (GetType() == TYPE_FLAGS)
        {
            const EnumMap* map = GetEnumMap();
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

void ValueProperty::GenerateBuiltInSubProperties()
{
    const Vector<String>* componentNames = nullptr;
    Vector<SubValueProperty*> subProperties;
    VariantType::eVariantType valueType = GetValueType();
    if (valueType == VariantType::TYPE_VECTOR2)
    {
        componentNames = &SValueProperty::VECTOR2_COMPONENT_NAMES;
    }
    else if (valueType == VariantType::TYPE_COLOR)
    {
        componentNames = &SValueProperty::COLOR_COMPONENT_NAMES;
    }
    else if (valueType == VariantType::TYPE_VECTOR4)
    {
        componentNames = &SValueProperty::MARGINS_COMPONENT_NAMESs;
    }
    else if (valueType == VariantType::TYPE_INT32 && inspDesc && inspDesc->type == InspDesc::T_FLAGS)
    {
        const EnumMap* map = inspDesc->enumMap;
        for (size_type i = 0; i < map->GetCount(); ++i)
        {
            int val = 0;
            DVASSERT(map->GetValue(i, val));
            subProperties.push_back(new SubValueProperty(i, map->ToString(val)));
        }
    }

    if (componentNames != nullptr)
    {
        for (size_type i = 0; i < componentNames->size(); ++i)
            subProperties.push_back(new SubValueProperty(i, componentNames->at(i)));
    }

    for (SubValueProperty* prop : subProperties)
    {
        prop->SetParent(this);
        AddSubValueProperty(prop);
        SafeRelease(prop);
    }
    subProperties.clear();
}
