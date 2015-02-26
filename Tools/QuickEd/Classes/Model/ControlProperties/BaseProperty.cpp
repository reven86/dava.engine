#include "BaseProperty.h"

using namespace DAVA;

BaseProperty::BaseProperty() : parent(NULL), readOnly(false)
{
    
}

BaseProperty::~BaseProperty()
{
}

BaseProperty *BaseProperty::GetParent() const
{
    return parent;
}

void BaseProperty::SetParent(BaseProperty *parent)
{
    this->parent = parent;
}

int BaseProperty::GetIndex(BaseProperty *property) const
{
    for (int i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i) == property)
            return i;
    }
    return -1;
}

bool BaseProperty::HasChanges() const
{
    for (int i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i)->HasChanges())
            return true;
    }
    return false;
}

void BaseProperty::Serialize(PackageSerializer *serializer) const
{
    for (int i = 0; i < GetCount(); i++)
        GetProperty(i)->Serialize(serializer);
}

bool BaseProperty::IsReadOnly() const
{
    return readOnly;
}

void BaseProperty::SetReadOnly()
{
    readOnly = true;
    for (int i = 0; i < GetCount(); i++)
        GetProperty(i)->SetReadOnly();
}

DAVA::VariantType BaseProperty::GetValue() const
{
    return DAVA::VariantType();
}

void BaseProperty::SetValue(const DAVA::VariantType &/*newValue*/)
{
    // Do nothing by default
}

VariantType BaseProperty::GetDefaultValue() const
{
    return VariantType();
}

void BaseProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    // Do nothing by default
}

const EnumMap *BaseProperty::GetEnumMap() const
{
    return NULL;
}

void BaseProperty::ResetValue()
{
    // Do nothing by default
}

bool BaseProperty::IsReplaced() const
{
    return false; // false by default
}

Vector<String> BaseProperty::GetPath() const
{
    const BaseProperty *p = this;
    
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

BaseProperty *BaseProperty::GetPropertyByPath(const Vector<String> &path)
{
    BaseProperty *prop = this;
    
    for (const String &name : path)
    {
        BaseProperty *child = nullptr;
        for (int32 index = 0; index < prop->GetCount(); index++)
        {
            BaseProperty *candidate = prop->GetProperty(index);
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

BaseProperty *BaseProperty::GetRootProperty()
{
    BaseProperty *property = this;
    while (property->parent)
        property = property->parent;
    return property;
}

const BaseProperty *BaseProperty::GetRootProperty() const
{
    const BaseProperty *property = this;
    while (property->parent)
        property = property->parent;
    return property;
}
