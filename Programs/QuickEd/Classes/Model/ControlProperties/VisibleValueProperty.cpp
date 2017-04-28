#include "VisibleValueProperty.h"

using namespace DAVA;

VisibleValueProperty::VisibleValueProperty(DAVA::BaseObject* object, const String& name, const Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, nullptr, name, ref, sourceProperty, copyType)
{
    ApplyValue(ref.GetValue());
}

void VisibleValueProperty::SetVisibleInEditor(bool visible)
{
    visibleInEditor = visible;
    reflection.SetValue(visibleInGame && visibleInEditor);
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
    visibleInGame = value.Cast<bool>();
    reflection.SetValue(visibleInGame && visibleInEditor);
}
