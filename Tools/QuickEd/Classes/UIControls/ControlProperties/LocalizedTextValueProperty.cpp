#include "LocalizedTextValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType)
    : ValueProperty(object, member, sourceProperty, copyType)
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
