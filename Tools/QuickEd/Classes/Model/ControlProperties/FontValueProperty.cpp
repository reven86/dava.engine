#include "FontValueProperty.h"

using namespace DAVA;

FontValueProperty::FontValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, FontValueProperty *sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, member, sourceProperty, copyType)
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

AbstractProperty *FontValueProperty::GetProperty(int index) const
{
    return nullptr;
}

void FontValueProperty::Refresh()
{
    IntrospectionProperty::Refresh();
    
    member->SetValue(GetBaseObject(), VariantType(presetName));
}

VariantType FontValueProperty::GetValue() const
{
    return VariantType(presetName);
}

void FontValueProperty::ApplyValue(const DAVA::VariantType &value)
{
    presetName = value.AsString();
    member->SetValue(GetBaseObject(), VariantType(presetName));
}
