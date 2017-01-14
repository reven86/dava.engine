#include "VisibleValueProperty.h"

using namespace DAVA;

VisibleValueProperty::VisibleValueProperty(DAVA::BaseObject* object, const DAVA::ReflectedStructure::Field* field_, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, field_, sourceProperty, copyType)
{
    ApplyValue(field_->valueWrapper->GetValue(object));
}

void VisibleValueProperty::SetVisibleInEditor(bool visible)
{
    visibleInEditor = visible;
    field->valueWrapper->SetValue(GetBaseObject(), VariantType(visibleInGame && visibleInEditor));
}

bool VisibleValueProperty::GetVisibleInEditor() const
{
    return visibleInEditor;
}

Any VisibleValueProperty::GetValue() const
{
    return Any(visibleInGame);
}

void VisibleValueProperty::ApplyValue(const DAVA::Any& value)
{
    visibleInGame = value.Get<bool>();
    field->valueWrapper->SetValue(GetBaseObject(), VariantType(visibleInGame && visibleInEditor));
}
