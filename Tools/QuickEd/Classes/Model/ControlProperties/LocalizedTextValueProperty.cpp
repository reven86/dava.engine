#include "LocalizedTextValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType)
    : ValueProperty(object, member, sourceProperty, copyType)
{
    ApplyValue(member->Value(object));
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

void LocalizedTextValueProperty::ApplyValue(const DAVA::VariantType &value)
{
    text = value.AsWideString();
    GetMember()->SetValue(GetBaseObject(), VariantType(LocalizedString(text)));
}
