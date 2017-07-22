#include "VisibleValueProperty.h"

#include <UI/UIControl.h>

using namespace DAVA;

VisibleValueProperty::VisibleValueProperty(DAVA::BaseObject* object, const String& name, const Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, nullptr, name, ref, sourceProperty, copyType)
{
}

void VisibleValueProperty::SetVisibleInEditor(bool visible)
{
    DynamicTypeCheck<UIControl*>(GetBaseObject())->SetHiddenForDebug(!visible);
}

bool VisibleValueProperty::GetVisibleInEditor() const
{
    return !DynamicTypeCheck<UIControl*>(GetBaseObject())->IsHiddenForDebug();
}
