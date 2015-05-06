#include "FontValueProperty.h"

#include "FileSystem/LocalizationSystem.h"

using namespace DAVA;

FontValueProperty::FontValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType)
    : ValueProperty(object, member, sourceProperty, copyType)
{
    ApplyValue(member->Value(object));
}

FontValueProperty::~FontValueProperty()
{
    
}

int FontValueProperty::GetCount() const
{
    return 0;
}

BaseProperty *FontValueProperty::GetProperty(int index) const
{
    return nullptr;
}

VariantType FontValueProperty::GetValue() const
{
    return VariantType(presetName);
}

void FontValueProperty::RefreshFontValue()
{
    GetMember()->SetValue(GetBaseObject(), VariantType(presetName));
}

void FontValueProperty::ApplyValue(const DAVA::VariantType &value)
{
    presetName = value.AsString();
    GetMember()->SetValue(GetBaseObject(), VariantType(presetName));
}
