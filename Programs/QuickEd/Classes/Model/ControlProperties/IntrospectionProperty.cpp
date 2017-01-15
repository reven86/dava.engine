#include "IntrospectionProperty.h"

#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"
#include "VisibleValueProperty.h"

#include "PropertyVisitor.h"
#include "SubValueProperty.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include <Base/BaseMath.h>
#include <UI/UIControl.h>

using namespace DAVA;

namespace
{
const String INTROSPECTION_PROPERTY_NAME_SIZE("size");
const String INTROSPECTION_PROPERTY_NAME_POSITION("position");
const String INTROSPECTION_PROPERTY_NAME_TEXT("text");
const String INTROSPECTION_PROPERTY_NAME_FONT("font");
const String INTROSPECTION_PROPERTY_NAME_CLASSES("classes");
const String INTROSPECTION_PROPERTY_NAME_VISIBLE("visible");
}

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject* anObject, const DAVA::ReflectedStructure::Field* field_, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : ValueProperty(field_->name, field_->valueWrapper->GetType(), true, field_)
    , object(SafeRetain(anObject))
    , field(field_)
    , flags(EF_CAN_RESET)
{
    int32 propertyIndex = UIStyleSheetPropertyDataBase::Instance()->FindStyleSheetPropertyByField(field);
    SetStylePropertyIndex(propertyIndex);

    if (sourceProperty)
    {
        if (copyType == CT_COPY)
        {
            SetOverridden(sourceProperty->IsOverriddenLocally());
            SetDefaultValue(sourceProperty->GetDefaultValue());
        }
        else
        {
            AttachPrototypeProperty(sourceProperty);
            SetDefaultValue(field->valueWrapper->GetValue(object));
        }
        field->valueWrapper->SetValue(object, sourceProperty->GetValue());
    }
    else
    {
        SetDefaultValue(field->valueWrapper->GetValue(object));
    }

    if (sourceProperty != nullptr)
        sourceValue = sourceProperty->sourceValue;
    else
        sourceValue = field->valueWrapper->GetValue(object);
}

IntrospectionProperty::~IntrospectionProperty()
{
    SafeRelease(object);
}

IntrospectionProperty* IntrospectionProperty::Create(UIControl* control, const ReflectedStructure::Field* field, const IntrospectionProperty* sourceProperty, eCloneType cloneType)
{
    if (field->name == INTROSPECTION_PROPERTY_NAME_TEXT)
    {
        return new LocalizedTextValueProperty(control, field, sourceProperty, cloneType);
    }
    else if (field->name == INTROSPECTION_PROPERTY_NAME_FONT)
    {
        return new FontValueProperty(control, field, sourceProperty, cloneType);
    }
    else if (field->name == INTROSPECTION_PROPERTY_NAME_VISIBLE)
    {
        return new VisibleValueProperty(control, field, sourceProperty, cloneType);
    }
    else
    {
        IntrospectionProperty* result = new IntrospectionProperty(control, field, sourceProperty, cloneType);
        ;
        if (field->name == INTROSPECTION_PROPERTY_NAME_SIZE || field->name == INTROSPECTION_PROPERTY_NAME_POSITION)
        {
            result->flags |= EF_DEPENDS_ON_LAYOUTS;
        }
        if (field->name == INTROSPECTION_PROPERTY_NAME_CLASSES)
        {
            result->flags |= EF_AFFECTS_STYLES;
        }
        return result;
    }
}

void IntrospectionProperty::Refresh(DAVA::int32 refreshFlags)
{
    ValueProperty::Refresh(refreshFlags);

    if ((refreshFlags & REFRESH_DEPENDED_ON_LAYOUT_PROPERTIES) != 0 && (GetFlags() & EF_DEPENDS_ON_LAYOUTS) != 0)
        ApplyValue(sourceValue);
}

void IntrospectionProperty::Accept(PropertyVisitor* visitor)
{
    visitor->VisitIntrospectionProperty(this);
}

uint32 IntrospectionProperty::GetFlags() const
{
    uint32 result = flags;
    if (GetPrototypeProperty() && !IsOverriddenLocally() && IsOverridden())
        result |= EF_INHERITED;
    return result;
}

Any IntrospectionProperty::GetValue() const
{
    return field->valueWrapper->GetValue(object);
}

const DAVA::ReflectedStructure::Field* IntrospectionProperty::GetField() const
{
    return field;
}

void IntrospectionProperty::DisableResetFeature()
{
    flags &= ~EF_CAN_RESET;
}

void IntrospectionProperty::ApplyValue(const DAVA::Any& value)
{
    sourceValue = value;
    field->valueWrapper->SetValue(object, value);
}
