#include "FontValueProperty.h"

using namespace DAVA;

FontValueProperty::FontValueProperty(DAVA::BaseObject* object, const DAVA::String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, -1, name, ref, sourceProperty, copyType)
{
    ApplyValue(ref.GetValue());
}

FontValueProperty::~FontValueProperty()
{
}

void FontValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    IntrospectionProperty::Refresh(refreshFlags);

    if (refreshFlags & REFRESH_FONT)
    {
        reflection.SetValue(presetName);
    }
}

Any FontValueProperty::GetValue() const
{
    return reflection.GetValue();
}

void FontValueProperty::ApplyValue(const DAVA::Any& value)
{
    presetName = value.Cast<String>();
    reflection.SetValue(presetName);
}
