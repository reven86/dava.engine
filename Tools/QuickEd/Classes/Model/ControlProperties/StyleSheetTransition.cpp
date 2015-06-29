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

#include "StyleSheetTransition.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

StyleSheetTransition::StyleSheetTransition(StyleSheetNode *aStyleSheet, DAVA::uint32 aPropertyIndex)
    : ValueProperty("prop")
    , styleSheet(aStyleSheet) // weak
    , propertyIndex(aPropertyIndex)
{
    const UIStyleSheetPropertyDescriptor& descr = UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex);
    name = String(descr.name.c_str());
}

StyleSheetTransition::~StyleSheetTransition()
{
    styleSheet = nullptr; // weak
}

int StyleSheetTransition::GetCount() const
{
    return 0;
}

AbstractProperty *StyleSheetTransition::GetProperty(int index) const
{
    return nullptr;
}

void StyleSheetTransition::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetTransition(this);
}

bool StyleSheetTransition::IsReadOnly() const
{
    return styleSheet->IsReadOnly();
}

AbstractProperty::ePropertyType StyleSheetTransition::GetType() const
{
    return TYPE_VARIANT;
}

VariantType StyleSheetTransition::GetValue() const
{
    const UIStyleSheetProperty* prop = GetStyleSheetProperty();
    
    DVASSERT(prop);

    return VariantType(Format("%0.2f %s", prop->transitionTime, GlobalEnumMap<Interpolation::FuncType>::Instance()->ToString(prop->transitionFunction)));
}

float32 StyleSheetTransition::GetTransitionTime() const
{
    const UIStyleSheetProperty* prop = GetStyleSheetProperty();
    DVASSERT(prop);
    return prop->transitionTime;
}

Interpolation::FuncType StyleSheetTransition::GetTransitionFunction() const
{
    const UIStyleSheetProperty* prop = GetStyleSheetProperty();
    DVASSERT(prop);
    return prop->transitionFunction;
}

const EnumMap *StyleSheetTransition::GetEnumMap() const
{
    return nullptr;
}

void StyleSheetTransition::ApplyValue(const DAVA::VariantType &value)
{
}

const UIStyleSheetProperty* StyleSheetTransition::GetStyleSheetProperty() const
{
    const UIStyleSheet *ss = styleSheet->GetStyleSheet();
    const auto &properties = ss->GetPropertyTable()->GetProperties();
    for (const auto &prop : properties)
    {
        if (prop.propertyIndex == propertyIndex)
        {
            return &prop;
        }
    }

    DVASSERT(false);
    return nullptr;
}