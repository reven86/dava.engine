#include "StyleSheetProperty.h"

#include "PropertyVisitor.h"
#include "IntrospectionProperty.h"
#include "VariantTypeProperty.h"

#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

namespace SStyleSheetProperty
{
static const Type *GetValueType(uint32 propertyIndex)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
    return descr.field_s->reflectedType->GetType();
}
    
static const ReflectedStructure::Field* GetField(uint32 propertyIndex)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
    return descr.field_s;
}
}

DAVA_REFLECTION_IMPL(StyleSheetProperty)
{
    ReflectionRegistrator<StyleSheetProperty>::Begin()
    .Field("transition", &StyleSheetProperty::HasTransition, &StyleSheetProperty::SetTransition)
    .Field("transitionTime", &StyleSheetProperty::GetTransitionTime, &StyleSheetProperty::SetTransitionTime)
    .Field("transitionFunction", &StyleSheetProperty::GetTransitionFunction, &StyleSheetProperty::SetTransitionFunction)
    [
     EnumMeta::Create<Interpolation::FuncType>(EnumMeta::EM_NOCAST)
     ]

    .End();
}

StyleSheetProperty::StyleSheetProperty(const DAVA::UIStyleSheetProperty& aProperty)
    : ValueProperty("prop", SStyleSheetProperty::GetValueType(aProperty.propertyIndex), false, SStyleSheetProperty::GetField(aProperty.propertyIndex))
    , property(aProperty)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(property.propertyIndex);
    SetName(String(descr.GetFullName().c_str()));
    SetOverridden(true);

    RefPtr<VariantTypeProperty> valueProp(new VariantTypeProperty("Value", descr.field_s, property.value));
    valueProp->SetValue(property.value);
    valueProp->SetParent(this);
    AddSubValueProperty(valueProp.Get());

    const ReflectedType *reflectedType = GetReflectedType();
    for (const std::unique_ptr<ReflectedStructure::Field> &field : reflectedType->GetStrucutre()->fields)
    {
        RefPtr<IntrospectionProperty> inspProp(new IntrospectionProperty(this, field.get(), nullptr, CT_COPY));
        inspProp->SetValue(field->valueWrapper->GetValue(this));
        inspProp->SetParent(this);
        inspProp->DisableResetFeature();
        AddSubValueProperty(inspProp.Get());
    }
}

StyleSheetProperty::~StyleSheetProperty()
{
}

void StyleSheetProperty::Accept(PropertyVisitor* visitor)
{
    visitor->VisitStyleSheetProperty(this);
}

DAVA::uint32 StyleSheetProperty::GetFlags() const
{
    return EF_CAN_REMOVE;
}

Any StyleSheetProperty::GetValue() const
{
    return property.value;
}

void StyleSheetProperty::ApplyValue(const DAVA::Any& value)
{
    property.value = value;
}

Interpolation::FuncType StyleSheetProperty::GetTransitionFunction() const
{
    return property.transitionFunction;
}

void StyleSheetProperty::SetTransitionFunction(Interpolation::FuncType type)
{
    property.transitionFunction = type;
}

float32 StyleSheetProperty::GetTransitionTime() const
{
    return property.transitionTime;
}

void StyleSheetProperty::SetTransitionTime(DAVA::float32 transitionTime)
{
    property.transitionTime = transitionTime;
}

bool StyleSheetProperty::HasTransition() const
{
    return property.transition;
}

void StyleSheetProperty::SetTransition(bool transition)
{
    property.transition = transition;
}

uint32 StyleSheetProperty::GetPropertyIndex() const
{
    return property.propertyIndex;
}

const UIStyleSheetProperty& StyleSheetProperty::GetProperty() const
{
    return property;
}
