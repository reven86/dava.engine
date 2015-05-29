#include "LocalizedTextValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject *anObject, const DAVA::InspMember *aMmember, const LocalizedTextValueProperty *sourceProperty, eCloneType cloneType)
    : IntrospectionProperty(anObject, aMmember, sourceProperty, cloneType)
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

AbstractProperty *LocalizedTextValueProperty::GetProperty(int index) const
{
    return NULL;
}

void LocalizedTextValueProperty::Refresh()
{
    IntrospectionProperty::Refresh();

    member->SetValue(GetBaseObject(), VariantType(LocalizedString(text)));
}

VariantType LocalizedTextValueProperty::GetValue() const
{
    return VariantType(text);
}

void LocalizedTextValueProperty::ApplyValue(const DAVA::VariantType &value)
{
    text = value.AsWideString();
    member->SetValue(GetBaseObject(), VariantType(LocalizedString(text)));
}
