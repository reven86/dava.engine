#include "AbstractProperty.h"

using namespace DAVA;

AbstractProperty::AbstractProperty() : parent(NULL)
{
    
}

AbstractProperty::~AbstractProperty()
{
}

AbstractProperty *AbstractProperty::GetParent() const
{
    return parent;
}

void AbstractProperty::SetParent(AbstractProperty *parent)
{
    this->parent = parent;
}

int AbstractProperty::GetIndex(AbstractProperty *property) const
{
    for (int32 i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i) == property)
            return i;
    }
    return -1;
}

void AbstractProperty::Refresh()
{
}

AbstractProperty *AbstractProperty::FindPropertyByPrototype(AbstractProperty *prototype)
{
    for (int32 i = 0; i < GetCount(); i++)
    {
        AbstractProperty *result = GetProperty(i)->FindPropertyByPrototype(prototype);
        if (result)
            return result;
    }
    return nullptr;
}

bool AbstractProperty::HasChanges() const
{
    for (int32 i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i)->HasChanges())
            return true;
    }
    return false;
}

bool AbstractProperty::IsReadOnly() const
{
    return parent ? parent->IsReadOnly() : true;
}

DAVA::VariantType AbstractProperty::GetValue() const
{
    return DAVA::VariantType();
}

void AbstractProperty::SetValue(const DAVA::VariantType &/*newValue*/)
{
    // Do nothing by default
}

VariantType AbstractProperty::GetDefaultValue() const
{
    return VariantType();
}

void AbstractProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    // Do nothing by default
}

const EnumMap *AbstractProperty::GetEnumMap() const
{
    return NULL;
}

void AbstractProperty::ResetValue()
{
    // Do nothing by default
}

bool AbstractProperty::IsReplaced() const
{
    return false; // false by default
}

AbstractProperty *AbstractProperty::GetRootProperty()
{
    AbstractProperty *property = this;
    while (property->parent)
        property = property->parent;
    return property;
}

const AbstractProperty *AbstractProperty::GetRootProperty() const
{
    const AbstractProperty *property = this;
    while (property->parent)
        property = property->parent;
    return property;
}
