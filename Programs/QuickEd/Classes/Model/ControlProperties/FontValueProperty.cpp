#include "FontValueProperty.h"

using namespace DAVA;

FontValueProperty::FontValueProperty(DAVA::BaseObject* object, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, member, sourceProperty, copyType)
{
    ApplyValue(member->Value(object));
}

FontValueProperty::~FontValueProperty()
{
}

void FontValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    IntrospectionProperty::Refresh(refreshFlags);

    if (refreshFlags & REFRESH_FONT)
        member->SetValue(GetBaseObject(), VariantType(presetName));
}

VariantType FontValueProperty::GetValue() const
{
    return member->Value(object);
}

void FontValueProperty::ApplyValue(const DAVA::VariantType& value)
{
    presetName = value.AsString();
    member->SetValue(GetBaseObject(), VariantType(presetName));
}
