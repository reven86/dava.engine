//
//  SubValueProperty.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "SubValueProperty.h"

#include "ValueProperty.h"

using namespace DAVA;

SubValueProperty::SubValueProperty(int index) : index(index)
{
    
}

SubValueProperty::~SubValueProperty()
{
    
}

int SubValueProperty::GetCount() const
{
    return 0;
}

BaseProperty *SubValueProperty::GetProperty(int index) const
{
    return NULL;
}

String SubValueProperty::GetName() const
{
    return GetValueProperty()->GetSubValueName(index);
}

SubValueProperty::ePropertyType SubValueProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType SubValueProperty::GetValue() const
{
    return GetValueProperty()->GetSubValue(index);
}

void SubValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    GetValueProperty()->SetSubValue(index, newValue);
}

void SubValueProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    GetValueProperty()->SetDefaultSubValue(index, newValue);
}

void SubValueProperty::ResetValue()
{
    GetValueProperty()->ResetValue();
}

bool SubValueProperty::IsReplaced() const
{
    return GetValueProperty()->IsReplaced();
}

ValueProperty *SubValueProperty::GetValueProperty() const
{
    return DynamicTypeCheck<ValueProperty*>(GetParent());
}
