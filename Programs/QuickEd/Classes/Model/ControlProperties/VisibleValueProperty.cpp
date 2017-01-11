#include "VisibleValueProperty.h"

using namespace DAVA;

VisibleValueProperty::VisibleValueProperty(DAVA::BaseObject* object, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, member, sourceProperty, copyType)
{
    ApplyValue(member->Value(object));
}

void VisibleValueProperty::SetVisibleInEditor(bool visible)
{
    visibleInEditor = visible;
    member->SetValue(GetBaseObject(), VariantType(visibleInGame && visibleInEditor));
}

bool VisibleValueProperty::GetVisibleInEditor() const
{
    return visibleInEditor;
}

VariantType VisibleValueProperty::GetValue() const
{
    return VariantType(visibleInGame);
}

void VisibleValueProperty::ApplyValue(const DAVA::VariantType& value)
{
    visibleInGame = value.AsBool();
    member->SetValue(GetBaseObject(), VariantType(visibleInGame && visibleInEditor));
}
