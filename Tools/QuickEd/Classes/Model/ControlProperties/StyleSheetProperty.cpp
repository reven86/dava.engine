/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

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
