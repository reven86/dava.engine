#include "StyleSheetProperty.h"

#include "PropertyVisitor.h"
#include "IntrospectionProperty.h"
#include "VariantTypeProperty.h"

#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

namespace SStyleSheetProperty
{
static VariantType::eVariantType GetValueType(uint32 propertyIndex)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
    return VariantType::TypeFromMetaInfo(descr.memberInfo->Type());
}
static const InspDesc* GetInspDesc(uint32 propertyIndex)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
    return &descr.memberInfo->Desc();
}
}

StyleSheetProperty::StyleSheetProperty(const DAVA::UIStyleSheetProperty& aProperty)
    : ValueProperty("prop", SStyleSheetProperty::GetValueType(aProperty.propertyIndex), false, SStyleSheetProperty::GetInspDesc(aProperty.propertyIndex))
    , property(aProperty)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(property.propertyIndex);
    SetName(String(descr.GetFullName().c_str()));
    SetOverridden(true);

    RefPtr<VariantTypeProperty> valueProp(new VariantTypeProperty("Value", &descr.memberInfo->Desc(), property.value));
    valueProp->SetValue(property.value);
    valueProp->SetParent(this);
    AddSubValueProperty(valueProp.Get());

    for (int32 i = 0; i < TypeInfo()->MembersCount(); i++)
    {
        const InspMember* member = TypeInfo()->Member(i);
        RefPtr<IntrospectionProperty> inspProp(new IntrospectionProperty(this, member, nullptr, CT_COPY));
        inspProp->SetValue(member->Value(this));
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

VariantType StyleSheetProperty::GetValue() const
{
    return property.value;
}

void StyleSheetProperty::ApplyValue(const DAVA::VariantType& value)
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

DAVA::int32 StyleSheetProperty::GetTransitionFunctionAsInt() const
{
    return GetTransitionFunction();
}

void StyleSheetProperty::SetTransitionFunctionFromInt(DAVA::int32 type)
{
    SetTransitionFunction(static_cast<Interpolation::FuncType>(type));
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
