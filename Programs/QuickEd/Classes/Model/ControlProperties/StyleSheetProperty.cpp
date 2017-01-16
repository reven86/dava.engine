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
//static const Type *GetValueType(uint32 propertyIndex)
//{
//    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
//    return descr.field_s->reflectedType->GetType();
//}
//    
//static const ReflectedStructure::Field* GetField(uint32 propertyIndex)
//{
//    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
//    return descr.field_s;
//}
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

StyleSheetProperty::StyleSheetProperty(const DAVA::UIStyleSheetProperty& property_)
    : ValueProperty("prop", property_.value.GetType(), false)
    , property(property_)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(property.propertyIndex);
    SetName(String(descr.GetFullName().c_str()));
    SetOverridden(true);

    RefPtr<VariantTypeProperty> valueProp(new VariantTypeProperty("Value", property.value));
    valueProp->SetValue(property.value);
    valueProp->SetParent(this);
    AddSubValueProperty(valueProp.Get());

    StyleSheetProperty *pp = this;
    Reflection ref = Reflection::Create(&pp);
    Vector<Reflection::Field> fields = ref.GetFields();
    for (const Reflection::Field &field : fields)
    {
        RefPtr<IntrospectionProperty> inspProp(new IntrospectionProperty(this, field.key.Get<String>(), field.ref, nullptr, CT_COPY));
        inspProp->SetValue(field.ref.GetValue());
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
