#include "LocalizedTextValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

LocalizedTextValueProperty::LocalizedTextValueProperty(DAVA::BaseObject* anObject, const DAVA::String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType cloneType)
    : IntrospectionProperty(anObject, nullptr, name, ref, sourceProperty, cloneType)
{
    ApplyValue(ref.GetValue());
}

LocalizedTextValueProperty::~LocalizedTextValueProperty()
{
}

void LocalizedTextValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    IntrospectionProperty::Refresh(refreshFlags);

    if (refreshFlags & REFRESH_LOCALIZATION)
    {
        reflection.SetValue(LocalizedUtf8String(text));
    }
}

Any LocalizedTextValueProperty::GetValue() const
{
    return Any(text);
}

void LocalizedTextValueProperty::ApplyValue(const DAVA::Any& value)
{
    text = value.Get<String>();
    reflection.SetValue(LocalizedUtf8String(text));
}
