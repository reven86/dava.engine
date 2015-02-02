//
//  BaseProperty.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#include "BaseProperty.h"

using namespace DAVA;

BaseProperty::BaseProperty() : parent(NULL)
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

DAVA::VariantType BaseProperty::GetValue() const
{
    return DAVA::VariantType();
}

void BaseProperty::SetValue(const DAVA::VariantType &/*newValue*/)
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
