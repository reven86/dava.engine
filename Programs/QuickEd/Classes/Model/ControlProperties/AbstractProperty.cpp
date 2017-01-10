#include "AbstractProperty.h"

using namespace DAVA;

AbstractProperty::AbstractProperty()
    : parent(NULL)
{
}

AbstractProperty::~AbstractProperty()
{
}

AbstractProperty* AbstractProperty::GetParent() const
{
    return parent;
}

void AbstractProperty::SetParent(AbstractProperty* parent)
{
    this->parent = parent;
}

int32 AbstractProperty::GetIndex(const AbstractProperty* property) const
{
    for (uint32 i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i) == property)
            return static_cast<int32>(i);
    }
    return -1;
}

void AbstractProperty::Refresh(int32 refreshFlags)
{
}

AbstractProperty* AbstractProperty::FindPropertyByPrototype(AbstractProperty* prototype)
{
    for (uint32 i = 0; i < GetCount(); i++)
    {
        AbstractProperty* result = GetProperty(i)->FindPropertyByPrototype(prototype);
        if (result)
            return result;
    }
    return nullptr;
}

bool AbstractProperty::HasChanges() const
{
    for (uint32 i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i)->HasChanges())
            return true;
    }
    return false;
}

uint32 AbstractProperty::GetFlags() const
{
    return EF_NONE;
}

int32 AbstractProperty::GetStylePropertyIndex() const
{
    return -1;
}

bool AbstractProperty::IsReadOnly() const
{
    return parent ? parent->IsReadOnly() : true;
}

VariantType::eVariantType AbstractProperty::GetValueType() const
{
    return VariantType::TYPE_NONE;
}

VariantType AbstractProperty::GetValue() const
{
    return VariantType();
}

void AbstractProperty::SetValue(const VariantType& /*newValue*/)
{
    // Do nothing by default
}

VariantType AbstractProperty::GetDefaultValue() const
{
    return VariantType();
}

void AbstractProperty::SetDefaultValue(const VariantType& newValue)
{
    // Do nothing by default
}

const EnumMap* AbstractProperty::GetEnumMap() const
{
    return NULL;
}

void AbstractProperty::ResetValue()
{
    // Do nothing by default
}

bool AbstractProperty::IsOverridden() const
{
    return false; // false by default
}

bool AbstractProperty::IsOverriddenLocally() const
{
    return false; // false by default
}

AbstractProperty* AbstractProperty::GetRootProperty()
{
    AbstractProperty* property = this;
    while (property->parent)
        property = property->parent;
    return property;
}

const AbstractProperty* AbstractProperty::GetRootProperty() const
{
    const AbstractProperty* property = this;
    while (property->parent)
        property = property->parent;
    return property;
}

AbstractProperty* AbstractProperty::FindPropertyByName(const String& name)
{
    if (GetName() == name)
    {
        return this;
    }
    for (DAVA::uint32 index = 0, count = GetCount(); index < count; ++index)
    {
        AbstractProperty* property = GetProperty(index)->FindPropertyByName(name);
        if (property != nullptr)
        {
            return property;
        }
    }
    return nullptr;
}
