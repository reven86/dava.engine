#include "AbstractProperty.h"

using namespace DAVA;

AbstractProperty::AbstractProperty() : parent(NULL), readOnly(false)
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
    for (int i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i) == property)
            return i;
    }
    return -1;
}

bool AbstractProperty::HasChanges() const
{
    for (int i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i)->HasChanges())
            return true;
    }
    return false;
}

void AbstractProperty::Serialize(PackageSerializer *serializer) const
{
    for (int i = 0; i < GetCount(); i++)
        GetProperty(i)->Serialize(serializer);
}

bool AbstractProperty::IsReadOnly() const
{
    return readOnly;
}

void AbstractProperty::SetReadOnly()
{
    readOnly = true;
    for (int i = 0; i < GetCount(); i++)
        GetProperty(i)->SetReadOnly();
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

Vector<String> AbstractProperty::GetPath() const
{
    const AbstractProperty *p = this;
    
    int32 count = 0;
    while (p && p->parent)
    {
        count++;
        p = p->parent;
    }

    Vector<String> path;
    path.resize(count);
    
    p = this;
    while (p && p->parent)
    {
        path[count - 1] = (p->GetName());
        p = p->parent;
        count--;
    }
    
    DVASSERT(count == 0);
    
    return path;
}

AbstractProperty *AbstractProperty::GetPropertyByPath(const Vector<String> &path)
{
    AbstractProperty *prop = this;
    
    for (const String &name : path)
    {
        AbstractProperty *child = nullptr;
        for (int32 index = 0; index < prop->GetCount(); index++)
        {
            AbstractProperty *candidate = prop->GetProperty(index);
            if (candidate->GetName() == name)
            {
                child = candidate;
                break;
            }
        }
        prop = child;
        if (!prop)
            break;
    }
    
    return prop;
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
