#include "LocalizedTextValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject* anObject, const DAVA::InspMember* aMmember, const IntrospectionProperty* sourceProperty, eCloneType cloneType)
    : IntrospectionProperty(anObject, aMmember, sourceProperty, cloneType)
{
    ApplyValue(member->Value(object));
}

LocalizedTextValueProperty::~LocalizedTextValueProperty()
{
}

void LocalizedTextValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    IntrospectionProperty::Refresh(refreshFlags);

    if (refreshFlags & REFRESH_LOCALIZATION)
        member->SetValue(GetBaseObject(), VariantType(LocalizedUtf8String(text)));
}

VariantType LocalizedTextValueProperty::GetValue() const
{
    return VariantType(text);
}

void LocalizedTextValueProperty::ApplyValue(const DAVA::VariantType& value)
{
    text = value.AsString();
    member->SetValue(GetBaseObject(), VariantType(LocalizedUtf8String(text)));
}
