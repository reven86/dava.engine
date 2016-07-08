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
const FastName INTROSPECTION_PROPERTY_NAME_SIZE("size");
const FastName INTROSPECTION_PROPERTY_NAME_POSITION("position");
const FastName INTROSPECTION_PROPERTY_NAME_TEXT("text");
const FastName INTROSPECTION_PROPERTY_NAME_FONT("font");
const FastName INTROSPECTION_PROPERTY_NAME_CLASSES("classes");
const FastName INTROSPECTION_PROPERTY_NAME_VISIBLE("visible");
}

IntrospectionProperty::IntrospectionProperty(DAVA::BaseObject* anObject, const DAVA::InspMember* aMember, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : ValueProperty(aMember->Desc().text, VariantType::TypeFromMetaInfo(aMember->Type()), true, &aMember->Desc())
    , object(SafeRetain(anObject))
    , member(aMember)
    , flags(EF_CAN_RESET)
{
    int32 propertyIndex = UIStyleSheetPropertyDataBase::Instance()->FindStyleSheetPropertyByMember(aMember);
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
            SetDefaultValue(member->Value(object));
        }
        member->SetValue(object, sourceProperty->GetValue());
    }
    else
    {
        SetDefaultValue(member->Value(object));
    }

    if (sourceProperty != nullptr)
        sourceValue = sourceProperty->sourceValue;
    else
        sourceValue = member->Value(object);
}

IntrospectionProperty::~IntrospectionProperty()
{
    SafeRelease(object);
}

IntrospectionProperty* IntrospectionProperty::Create(UIControl* control, const InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType cloneType)
{
    if (member->Name() == INTROSPECTION_PROPERTY_NAME_TEXT)
    {
        return new LocalizedTextValueProperty(control, member, sourceProperty, cloneType);
    }
    else if (member->Name() == INTROSPECTION_PROPERTY_NAME_FONT)
    {
        return new FontValueProperty(control, member, sourceProperty, cloneType);
    }
    else if (member->Name() == INTROSPECTION_PROPERTY_NAME_VISIBLE)
    {
        return new VisibleValueProperty(control, member, sourceProperty, cloneType);
    }
    else
    {
        IntrospectionProperty* result = new IntrospectionProperty(control, member, sourceProperty, cloneType);
        ;
        if (member->Name() == INTROSPECTION_PROPERTY_NAME_SIZE || member->Name() == INTROSPECTION_PROPERTY_NAME_POSITION)
        {
            result->flags |= EF_DEPENDS_ON_LAYOUTS;
        }
        if (member->Name() == INTROSPECTION_PROPERTY_NAME_CLASSES)
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

VariantType IntrospectionProperty::GetValue() const
{
    return member->Value(object);
}

const DAVA::InspMember* IntrospectionProperty::GetMember() const
{
    return member;
}

void IntrospectionProperty::DisableResetFeature()
{
    flags &= ~EF_CAN_RESET;
}

void IntrospectionProperty::ApplyValue(const DAVA::VariantType& value)
{
    sourceValue = value;
    member->SetValue(object, value);
}
