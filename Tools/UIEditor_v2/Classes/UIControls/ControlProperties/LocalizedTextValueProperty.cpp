//
//  LocalizedTextValueProperty.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 7.10.14.
//
//

#include "LocalizedTextValueProperty.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member) : ValueProperty(object, member)
{
    text = member->Value(object).AsWideString();
    GetMember()->SetValue(GetBaseObject(), VariantType(LocalizedString(text)));
}

LocalizedTextValueProperty::~LocalizedTextValueProperty()
{
    
}

int LocalizedTextValueProperty::GetCount() const
{
    return 0;
}

BaseProperty *LocalizedTextValueProperty::GetProperty(int index) const
{
    return NULL;
}

VariantType LocalizedTextValueProperty::GetValue() const
{
    return VariantType(text);
}

void LocalizedTextValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    ValueProperty::SetValue(newValue);
    text = newValue.AsWideString();
    GetMember()->SetValue(GetBaseObject(), VariantType(LocalizedString(text)));
}

void LocalizedTextValueProperty::ResetValue()
{
    ValueProperty::ResetValue();
    text = GetMember()->Value(GetBaseObject()).AsWideString();
    GetMember()->SetValue(GetBaseObject(), VariantType(LocalizedString(text)));
}
