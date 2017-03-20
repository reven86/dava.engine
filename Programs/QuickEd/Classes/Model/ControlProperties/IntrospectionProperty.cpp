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

    bool isSizeProperty = member->Name() == INTROSPECTION_PROPERTY_NAME_SIZE;
    if (isSizeProperty || member->Name() == INTROSPECTION_PROPERTY_NAME_POSITION)
    {
        UIControl* control = DynamicTypeCheck<UIControl*>(anObject);
        if (dynamic_cast<UIScrollViewContainer*>(control) == nullptr && control->GetName() != FastName("buttonToggle") &&
            control->GetName() != FastName("thumbSpriteControl"))
        {
            sourceRectComponent = control->GetOrCreateComponent<UILayoutSourceRectComponent>();
            SetLayoutSourceRectValue(member->Value(anObject));
        }
    }
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
        return new IntrospectionProperty(control, member, sourceProperty, cloneType);
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

DAVA::UILayoutSourceRectComponent *IntrospectionProperty::GetLayoutSourceRectComponent() const
{
    return sourceRectComponent.Get();
}

void IntrospectionProperty::ApplyValue(const DAVA::VariantType& value)
{
    member->SetValue(object, value);

    if (sourceRectComponent.Valid())
    {
        SetLayoutSourceRectValue(value);
    }
}

void IntrospectionProperty::SetLayoutSourceRectValue(const DAVA::VariantType& value)
{
    DVASSERT(sourceRectComponent.Valid());
    if (member->Name() == INTROSPECTION_PROPERTY_NAME_SIZE)
    {
        sourceRectComponent->SetSize(value.AsVector2());
    }
    else if (member->Name() == INTROSPECTION_PROPERTY_NAME_POSITION)
    {
        sourceRectComponent->SetPosition(value.AsVector2());
    }
    else
    {
        DVASSERT(false);
    }
}
