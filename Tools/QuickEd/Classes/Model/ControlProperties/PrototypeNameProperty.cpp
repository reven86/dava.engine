#include "PrototypeNameProperty.h"

PrototypeNameProperty::PrototypeNameProperty(const DAVA::String &propName, DAVA::UIControl *anObject, const PrototypeNameProperty *sourceProperty, eCloneType cloneType)
    : ValueProperty(propName)
{
    object = SafeRetain(anObject);
}

PrototypeNameProperty::~PrototypeNameProperty()
{
    SafeRelease(object);
}

void PrototypeNameProperty::Serialize(PackageSerializer *serializer) const
{

}

AbstractProperty::ePropertyType PrototypeNameProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::VariantType PrototypeNameProperty::GetValue() const
{
    return DAVA::VariantType();
}

void PrototypeNameProperty::ApplyValue(const DAVA::VariantType &value)
{
}

