#include "LocalizedTextValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject* anObject, const DAVA::ReflectedStructure::Field* field_, const IntrospectionProperty* sourceProperty, eCloneType cloneType)
    : IntrospectionProperty(anObject, field_, sourceProperty, cloneType)
{
    ApplyValue(field_->valueWrapper->GetValue(object));
}

LocalizedTextValueProperty::~LocalizedTextValueProperty()
{
}

void LocalizedTextValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    IntrospectionProperty::Refresh(refreshFlags);

    if (refreshFlags & REFRESH_LOCALIZATION)
        field->valueWrapper->SetValue(GetBaseObject(), VariantType(LocalizedUtf8String(text)));
}

Any LocalizedTextValueProperty::GetValue() const
{
    return Any(text);
}

void LocalizedTextValueProperty::ApplyValue(const DAVA::Any& value)
{
    text = value.Get<String>();
    field->valueWrapper->SetValue(GetBaseObject(), VariantType(LocalizedUtf8String(text)));
}
