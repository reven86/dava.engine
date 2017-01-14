#include "FontValueProperty.h"

using namespace DAVA;

FontValueProperty::FontValueProperty(DAVA::BaseObject* object, const DAVA::ReflectedStructure::Field* field, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, field, sourceProperty, copyType)
{
    ApplyValue(field->valueWrapper->GetValue(object));
}

FontValueProperty::~FontValueProperty()
{
}

void FontValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    IntrospectionProperty::Refresh(refreshFlags);

    if (refreshFlags & REFRESH_FONT)
        field->valueWrapper->SetValue(GetBaseObject(), VariantType(presetName));
}

Any FontValueProperty::GetValue() const
{
    return field->valueWrapper->GetValue(object);
}

void FontValueProperty::ApplyValue(const DAVA::Any& value)
{
    presetName = value.Get<String>();
    field->valueWrapper->SetValue(GetBaseObject(), VariantType(presetName));
}
