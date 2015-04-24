#include "StringProperty.h"

StringProperty::StringProperty(const DAVA::String &propName, DAVA::UIControl *anObject, const Getter &aGetter, const Setter &aSetter, const StringProperty *sourceProperty, eCloneType cloneType)
    : ValueProperty(propName)
    , editFlags(0)
{
    object = SafeRetain(anObject);
    getter = aGetter;
    setter = aSetter;
    if (aSetter == NULL)
        SetReadOnly();

    if (sourceProperty && setter != NULL && sourceProperty->GetValue() != DAVA::VariantType(getter(object)))
        setter(object, sourceProperty->GetValue().AsString());
}

StringProperty::~StringProperty()
{
    SafeRelease(object);
}

void StringProperty::Serialize(PackageSerializer *serializer) const
{

}

AbstractProperty::ePropertyType StringProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::VariantType StringProperty::GetValue() const
{
    if (getter != NULL)
    {
        return DAVA::VariantType(getter(object));
    }
    return DAVA::VariantType();
}

void StringProperty::SetEditFlag(DAVA::uint32 newEditFlags)
{
    editFlags = newEditFlags;
}

void StringProperty::ApplyValue(const DAVA::VariantType &value)
{
    if (setter != NULL && value.GetType() == DAVA::VariantType::TYPE_STRING)
    {
        setter(object, value.AsString());
    }
}

