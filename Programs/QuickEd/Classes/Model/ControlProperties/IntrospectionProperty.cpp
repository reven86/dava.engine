#include "IntrospectionProperty.h"

#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"
#include "VisibleValueProperty.h"

#include "PropertyVisitor.h"
#include "SubValueProperty.h"

#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include <Base/BaseMath.h>
#include <UI/UIControl.h>
#include <Reflection/ReflectedMeta.h>

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

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject* anObject, const DAVA::Type* componentType, const String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : ValueProperty(name, ref.GetValueType())
    , object(SafeRetain(anObject))
    , reflection(ref)
    , flags(EF_CAN_RESET)
{
    int32 propertyIndex = UIStyleSheetPropertyDataBase::Instance()->FindStyleSheetProperty(componentType, FastName(name));
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
            SetDefaultValue(reflection.GetValue());
        }
        reflection.SetValue(sourceProperty->GetValue());
    }
    else
    {
        SetDefaultValue(reflection.GetValue());
    }

    if (sourceProperty != nullptr)
        sourceValue = sourceProperty->sourceValue;
    else
        sourceValue = reflection.GetValue();

    GenerateBuiltInSubProperties();
}

IntrospectionProperty::~IntrospectionProperty()
{
    SafeRelease(object);
}

IntrospectionProperty* IntrospectionProperty::Create(UIControl* control, const String& name, const Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType cloneType)
{
    if (name == INTROSPECTION_PROPERTY_NAME_TEXT)
    {
        return new LocalizedTextValueProperty(control, name, ref, sourceProperty, cloneType);
    }
    else if (name == INTROSPECTION_PROPERTY_NAME_FONT)
    {
        return new FontValueProperty(control, name, ref, sourceProperty, cloneType);
    }
    else if (name == INTROSPECTION_PROPERTY_NAME_VISIBLE)
    {
        return new VisibleValueProperty(control, name, ref, sourceProperty, cloneType);
    }
    else
    {
        IntrospectionProperty* result = new IntrospectionProperty(control, Type::Instance<void>(), name, ref, sourceProperty, cloneType);
        if (name == INTROSPECTION_PROPERTY_NAME_SIZE || name == INTROSPECTION_PROPERTY_NAME_POSITION)
        {
            result->flags |= EF_DEPENDS_ON_LAYOUTS;
        }
        if (name == INTROSPECTION_PROPERTY_NAME_CLASSES)
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

IntrospectionProperty::ePropertyType IntrospectionProperty::GetType() const
{
    const M::Enum* enumMeta = reflection.GetMeta<M::Enum>();
    if (enumMeta)
    {
        return TYPE_ENUM;
    }

    const M::Flags* flagsMeta = reflection.GetMeta<M::Flags>();
    if (flagsMeta)
    {
        return TYPE_FLAGS;
    }

    return TYPE_VARIANT;
}

const EnumMap* IntrospectionProperty::GetEnumMap() const
{
    const M::Enum* enumMeta = reflection.GetMeta<M::Enum>();
    if (enumMeta != nullptr)
    {
        return enumMeta->GetEnumMap();
    }

    const M::Flags* flagsMeta = reflection.GetMeta<M::Flags>();
    if (flagsMeta != nullptr)
    {
        return flagsMeta->GetFlagsMap();
    }

    return nullptr;
}

Any IntrospectionProperty::GetValue() const
{
    return reflection.GetValue();
}

void IntrospectionProperty::DisableResetFeature()
{
    flags &= ~EF_CAN_RESET;
}

void IntrospectionProperty::ApplyValue(const DAVA::Any& value)
{
    sourceValue = value;
    reflection.SetValueWithCast(value);
}
