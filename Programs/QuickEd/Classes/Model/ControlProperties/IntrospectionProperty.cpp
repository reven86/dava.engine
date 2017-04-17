#include "IntrospectionProperty.h"

#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"
#include "VisibleValueProperty.h"

#include "PropertyVisitor.h"
#include "SubValueProperty.h"
#include <Base/BaseMath.h>
#include <UI/Layouts/UILayoutSourceRectComponent.h>
#include <UI/Styles/UIStyleSheetPropertyDataBase.h>
#include <UI/UIControl.h>
#include <UI/UIScrollViewContainer.h>
#include <UI/UISlider.h>
#include <UI/UISwitch.h>

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

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject* anObject, DAVA::int32 componentType, const String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType)
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

    GenerateBuiltInSubProperties();

    if (name == INTROSPECTION_PROPERTY_NAME_SIZE || name == INTROSPECTION_PROPERTY_NAME_POSITION)
    {
        UIControl* control = DynamicTypeCheck<UIControl*>(anObject);
        bool shouldAddSourceRectComponent = true;

        if (dynamic_cast<UIScrollViewContainer*>(control) != nullptr)
        {
            shouldAddSourceRectComponent = false;
        }
        else
        {
            if (control->GetName() == UISlider::THUMB_SPRITE_CONTROL_NAME ||
                control->GetName() == UISlider::MIN_SPRITE_CONTROL_NAME ||
                control->GetName() == UISlider::MAX_SPRITE_CONTROL_NAME ||
                control->GetName() == UISwitch::BUTTON_LEFT_NAME ||
                control->GetName() == UISwitch::BUTTON_RIGHT_NAME ||
                control->GetName() == UISwitch::BUTTON_TOGGLE_NAME)
            {
                shouldAddSourceRectComponent = false;
            }
        }

        if (shouldAddSourceRectComponent)
        {
            sourceRectComponent = control->GetOrCreateComponent<UILayoutSourceRectComponent>();

            if (sourceProperty != nullptr && sourceProperty->sourceRectComponent)
            {
                if (name == INTROSPECTION_PROPERTY_NAME_SIZE)
                {
                    sourceRectComponent->SetSize(sourceProperty->sourceRectComponent->GetSize());
                }
                else
                {
                    sourceRectComponent->SetPosition(sourceProperty->sourceRectComponent->GetPosition());
                }
            }
            else
            {
                SetLayoutSourceRectValue(reflection.GetValue());
            }
        }
    }
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
        return new IntrospectionProperty(control, -1, name, ref, sourceProperty, cloneType);
    }
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
    reflection.SetValueWithCast(value);

    if (sourceRectComponent.Valid())
    {
        SetLayoutSourceRectValue(value);
    }
}

void IntrospectionProperty::SetLayoutSourceRectValue(const DAVA::Any& value)
{
    DVASSERT(sourceRectComponent.Valid());
    if (GetName() == INTROSPECTION_PROPERTY_NAME_SIZE)
    {
        sourceRectComponent->SetSize(value.Get<Vector2>());
    }
    else if (GetName() == INTROSPECTION_PROPERTY_NAME_POSITION)
    {
        UIControl* control = DynamicTypeCheck<UIControl*>(object);
        Vector2 p = value.Get<Vector2>();
        sourceRectComponent->SetPosition(p);
    }
    else
    {
        DVASSERT(false);
    }
}
